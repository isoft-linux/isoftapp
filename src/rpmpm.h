#ifndef rpmPM_H
#define rpmPM_H
#include <string>
#include <list>
#include <sqlite3.h>
#include "daemon.h"
using namespace std;
void rpmInstall(int argc,char **argv,
                void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
                void *arg_data);
void rpmremove(int argc, char **argv,
               void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
               void *arg_data,
               bool nodeps = true);
bool getAllInstalledPkgs(list<string> &installedList,sqlite3 *db,bool useRpmApi);
bool getInstalledPkgVer(const char *pkgName,char nvrStr[256]);
int rpmCheck(int argc,char **argv);
#endif
