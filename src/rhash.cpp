// Include Files
#include "rhash.h"
#include "strutl.h"

#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include "system.h"

raptHash::raptHash(const string & HashName) : HashCtx(NULL), HashType(HashName)
{
}

raptHash::raptHash(const raptHash & Hash)
{
   HashCtx = rpmDigestDup(Hash.HashCtx);
   Value = Hash.Value;
   HashType = Hash.HashType;
}

raptHash::~raptHash()
{
   if (HashCtx)
      rpmDigestFinal(HashCtx, NULL, NULL, 0);
}

raptHash & raptHash::operator= (const raptHash & Hash)
{
   if (this == &Hash)
      return *this;

   if (HashCtx)
      rpmDigestFinal(HashCtx, NULL, NULL, 0);
   
   HashCtx = rpmDigestDup(Hash.HashCtx);
   Value = Hash.Value;
   HashType = Hash.HashType;
   return *this;
}

// raptHash::Result - Return checksum value                        /*{{{*/
// ---------------------------------------------------------------------
/* Add() may not be called after this */
string raptHash::Result()
{
   void *data = NULL;
   size_t dlen = 0;
   
   rpmDigestFinal(HashCtx, &data, &dlen, 1);
   if (data) {
      Value = string((const char*)data);
      free(data);
   }
   HashCtx = NULL;

   return Value;
}
									/*}}}*/
// raptHash::Add - Adds content of buffer into the checksum        /*{{{*/
// ---------------------------------------------------------------------
/* May not be called after Result() is called */
bool raptHash::Add(const unsigned char *data,unsigned long len)
{
   int rc;
   if (HashCtx == NULL) {
      pgpHashAlgo algo;
      if (HashType == "SHA512-Hash")
         algo = PGPHASHALGO_SHA512;
      else if (HashType == "SHA384-Hash")
         algo = PGPHASHALGO_SHA384;
      else if (HashType == "SHA256-Hash")
         algo = PGPHASHALGO_SHA256;
      else if (HashType == "SHA1-Hash")
         algo = PGPHASHALGO_SHA1;
      else if (HashType == "MD5-Hash")
         algo = PGPHASHALGO_MD5;
      else
         return false;

      HashCtx = rpmDigestInit(algo, RPMDIGEST_NONE);
   }

   rc = rpmDigestUpdate(HashCtx, data, len);
   
   return (rc == 0);
}
									/*}}}*/
// raptHash::AddFD - Add content of file into the checksum         /*{{{*/
// ---------------------------------------------------------------------
/* */
bool raptHash::AddFD(int Fd,unsigned long Size)
{
   unsigned char Buf[64 * 64];
   int Res = 0;
   while (Size != 0)
   {
      Res = read(Fd,Buf,std::min(Size,(unsigned long)sizeof(Buf)));
      if (Res < 0 || (unsigned) Res != std::min(Size,(unsigned long)sizeof(Buf)))
	 return false;
      Size -= Res;
      Add(Buf,Res);
   }
   return true;
}
									/*}}}*/
