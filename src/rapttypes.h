#ifndef PKGLIB_RAPTTYPES_H
#define PKGLIB_RAPTTYPES_H

/*
 * Layer of insulation against differing types used in rpm versions.
 * C happily converts enum to int etc automatically, C++ doesn't...
 */

#include <rpm/rpmtypes.h>
#include <rpm/rpmds.h>
#ifdef RPM_HAVE_RPMTAGVAL
typedef rpm_tag_t raptTag;
typedef rpm_tagtype_t raptTagType;
#else
typedef rpmTag raptTag;
typedef rpmTagType raptTagType;
#endif
typedef rpmsenseFlags raptDepFlags;
typedef uint32_t raptInt;
typedef uint32_t raptDbOffset;

#define CRPMTAG_TIMESTAMP   1012345
#define CRPMTAG_FILENAME          (rpmTag)1000000
#define CRPMTAG_FILESIZE          (rpmTag)1000001
#define CRPMTAG_MD5               (rpmTag)1000005
#define CRPMTAG_DIRECTORY         (rpmTag)1000010
#define CRPMTAG_BINARY            (rpmTag)1000011

#define raptInitIterator(a,b,c,d) rpmtsInitIterator(a,(rpmTag)b,c,d)
#endif /* PKGLIB_RAPTTYPES_H */
