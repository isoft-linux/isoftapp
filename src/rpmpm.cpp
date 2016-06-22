#include "rpmpm.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <iomanip>

#include <rpm/rpmlog.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmcli.h>
#include <rpm/rpmts.h>
#include <glib/gi18n.h>

using namespace std;

enum modes {
    MODE_QUERY      = (1 <<  0),
    MODE_VERIFY     = (1 <<  3),
#define MODES_QV (MODE_QUERY | MODE_VERIFY)

    MODE_INSTALL    = (1 <<  1),
    MODE_ERASE      = (1 <<  2),
#define MODES_IE (MODE_INSTALL | MODE_ERASE)

    MODE_UNKNOWN    = 0
};

#define MODES_FOR_NODEPS    (MODES_IE | MODE_VERIFY)
#define MODES_FOR_TEST      (MODES_IE)

static int quiet;
static void *m_arg_data = NULL;

static struct poptOption optionsTable[] = {
    {NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmQVSourcePoptTable, 0,
     N_("Query/Verify package selection options:"),
     NULL},
    {NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmQueryPoptTable, 0,
     N_("Query options (with -q or --query):"),
     NULL},
    {NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmVerifyPoptTable, 0,
     N_("Verify options (with -V or --verify):"),
     NULL},
    {NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmInstallPoptTable, 0,
     N_("Install/Upgrade/Erase options:"),
     NULL},
    {"quiet", '\0', POPT_ARGFLAG_DOC_HIDDEN, &quiet, 0, NULL, NULL},
    {NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmcliAllPoptTable, 0,
     N_("Common options for all rpm modes and executables:"),
     NULL},
    POPT_AUTOALIAS
    POPT_AUTOHELP
    POPT_TABLEEND
};


static void PrintHashes(double progress)
{
    int Percent = 100 * progress;
    int hashesTotal = 30;
    int hashesNeeded = int(hashesTotal * Percent / 100);

    cout << setw(hashesNeeded) << setfill('#') << "";
    cout << setw(hashesTotal-hashesNeeded) << setfill(' ') << "";
    cout << right << " [" << setw(3) << int(Percent) << "%]" << left;
    if (hashesTotal == hashesNeeded) {
       cout << endl;
    }

}

static void (*g_new_progress_handle)(double *progress, void *arg_data,const char *filename,Status status);

static void g_progress_handle (double *progress, void *arg_data,const char *filename)
{
    if (filename == NULL) {
        return ;
    }

    char pkgName[256]="";
    const char *p = strrchr(filename,'/');
    if (p) {
        snprintf(pkgName,sizeof(pkgName),"%s",p+1);
    } else
        snprintf(pkgName,sizeof(pkgName),"%s",filename) ;

    cout.setf(ios_base::left);

    string s(pkgName);

    const int namemax = 40;
    cout << "\r";
    cout << setw(namemax) << s.substr(0, namemax) << " ";
    PrintHashes(*progress);
    cout << flush;

}


static int rpmcliProgressState = 0;
static int rpmcliPackagesTotal = 0;
static int rpmcliHashesCurrent = 0;
static int rpmcliHashesTotal = 0;
static int rpmcliProgressCurrent = 0;
static int rpmcliProgressTotal = 0;
void * rpmShowProgress(const void * arg,
                        const rpmCallbackType what,
                        const rpm_loff_t amount,
                        const rpm_loff_t total,
                        fnpyKey key,
                        void * data)
{
    Header h = (Header) arg;
    int flags = (int) ((long)data);
    void * rc = NULL;
    const char * filename = (const char *)key;
    static FD_t fd = NULL;
    char pkgName[256]="";
    if(h) {
        snprintf(pkgName,sizeof(pkgName),"%s-%s-%s.%s",
                 headerGetString(h, RPMTAG_NAME),
                 headerGetString(h, RPMTAG_VERSION),
                 headerGetString(h, RPMTAG_RELEASE),
                 headerGetString(h, RPMTAG_ARCH));
    }

    switch (what) {
    case RPMCALLBACK_INST_OPEN_FILE:
        if (filename == NULL || filename[0] == '\0')
            return NULL;
        fd = Fopen(filename, "r.ufdio");
        if(fd == NULL) {
            printf("%s,%d: open [%s]error\n",__FUNCTION__,__LINE__,filename);
        }
        /* FIX: still necessary? */
        if (fd == NULL || Ferror(fd)) {
            if (fd != NULL) {
                Fclose(fd);

                fd = NULL;
            }
        } else
            fd = fdLink(fd);
        return (void *)fd;
        break;

    case RPMCALLBACK_INST_CLOSE_FILE:
        /* FIX: still necessary? */
        fd = fdFree(fd);
        if (fd != NULL) {
            Fclose(fd);
            fd = NULL;
        }
        break;

    case RPMCALLBACK_INST_START:
    case RPMCALLBACK_UNINST_START:
        if (rpmcliProgressState != what) {
            rpmcliProgressState = what;
            if (flags & INSTALL_HASH) {
                if (what == RPMCALLBACK_INST_START) {
                    fprintf(stdout, _("Updating / installing...\n"));
                } else {
                    //fprintf(stdout, _("Cleaning up / removing...\n"));
                }
                fflush(stdout);
            }
        }

        rpmcliHashesCurrent = 0;
        if (h == NULL || !(flags & INSTALL_LABEL))
            break;
        if (flags & INSTALL_HASH) {
            char *s = headerGetAsString(h, RPMTAG_NEVR);
            if (isatty (STDOUT_FILENO))
                fprintf(stdout, "%4d:%-33.33s", rpmcliProgressCurrent + 1, s);
            else
                fprintf(stdout, "%-38.38s", s);
            (void) fflush(stdout);
            free(s);
        } else {
            char *s = headerGetAsString(h, RPMTAG_NEVRA);
            fprintf(stdout, "%s\n", s);
            (void) fflush(stdout);
            free(s);
        }
        break;

    case RPMCALLBACK_INST_STOP:
        break;

    case RPMCALLBACK_TRANS_PROGRESS:
    case RPMCALLBACK_INST_PROGRESS:
    case RPMCALLBACK_UNINST_PROGRESS:
        if (flags & INSTALL_PERCENT) {
            double progress = (double) (total ? (((float) amount) / total): 100.0);
            g_progress_handle(&progress, m_arg_data,pkgName);

            Status status = STATUS_UNKNOWN;
            if (what == RPMCALLBACK_INST_PROGRESS)
                status = STATUS_INSTALL;
            else if (what == RPMCALLBACK_UNINST_PROGRESS)
                status = STATUS_REMOVE;
            if(g_new_progress_handle)
                g_new_progress_handle(&progress, m_arg_data,pkgName,status);

            // to test qjade queue...
            //usleep(10000);

        }
        else if (flags & INSTALL_HASH)
            //printHash(amount, total);
        (void) fflush(stdout);

        break;

    case RPMCALLBACK_TRANS_START:
        rpmcliHashesCurrent = 0;
        rpmcliProgressTotal = 1;
        rpmcliProgressCurrent = 0;
        rpmcliPackagesTotal = total;
        rpmcliProgressState = what;
        if (!(flags & INSTALL_LABEL))
            break;
        if (flags & INSTALL_HASH)
            fprintf(stdout, "%-38s", _("Preparing..."));
        else
            fprintf(stdout, "%s\n", _("Preparing packages..."));
        (void) fflush(stdout);
        break;

    case RPMCALLBACK_TRANS_STOP:
        //if (flags & INSTALL_HASH)
            //printHash(1, 1);    /* Fixes "preparing..." progress bar */
        rpmcliProgressTotal = rpmcliPackagesTotal;
        rpmcliProgressCurrent = 0;
        break;

    case RPMCALLBACK_UNINST_STOP:
        break;
    case RPMCALLBACK_UNPACK_ERROR:
        break;

    case RPMCALLBACK_CPIO_ERROR:
        break;
    case RPMCALLBACK_SCRIPT_ERROR:
        break;
    case RPMCALLBACK_SCRIPT_START:
        break;
    case RPMCALLBACK_SCRIPT_STOP:
        break;
    case RPMCALLBACK_UNKNOWN:
    default:
        break;
    }

    return rc;
}
void rpmInstall(int argc,char **argv,
                void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
                void *arg_data)
{
    char path[1024]="";
    snprintf(path,sizeof(path),"%s",argv[1]);

    m_arg_data = arg_data;
    g_new_progress_handle = report_progress;

    rpmts ts = NULL;
    struct rpmInstallArguments_s *ia = &rpmIArgs;
    poptContext optCon;
    int ret = 0;

    optCon = rpmcliInit(argc, argv, optionsTable);

    ia->installInterfaceFlags = INSTALL_UPGRADE | INSTALL_INSTALL | INSTALL_PERCENT;

    ts = rpmtsCreate();
    rpmtsSetRootDir(ts, rpmcliRootDir);

    if (!ia->incldocs) {
        if (rpmExpandNumeric("%{_excludedocs}"))
            ia->transFlags |= RPMTRANS_FLAG_NODOCS;
    }

    if (ia->noDeps)
        ia->installInterfaceFlags |= INSTALL_NODEPS;

    if (ia->prefix) {
        ia->relocations = malloc(2 * sizeof(*ia->relocations));
        ia->relocations[0].oldPath = NULL;   /* special case magic */
        ia->relocations[0].newPath = ia->prefix;
        ia->relocations[1].oldPath = NULL;
        ia->relocations[1].newPath = NULL;
    } else if (ia->relocations) {
        ia->relocations = realloc(ia->relocations,
            sizeof(*ia->relocations) * (ia->numRelocations + 1));
        ia->relocations[ia->numRelocations].oldPath = NULL;
        ia->relocations[ia->numRelocations].newPath = NULL;
    }
    chunk_t prob;
    prob.memory = malloc(1);
    prob.size = 0;

    ret = rpmInstallISoftApp(ts, ia, (ARGV_t) poptGetArgs(optCon), &prob);
    if (ret != 0) {
        double progress = 110.0;
        if(g_new_progress_handle)
            g_new_progress_handle(&progress, m_arg_data,NULL,STATUS_INSTALL);
    }

    rpmtsFree(ts);
    ts = NULL;

    rpmcliFini(optCon);
    optCon = NULL;
}

void rpmremove(int argc, char **argv,
               void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
               void *arg_data,
               bool nodeps)
{
    char path[1024]="";
    snprintf(path,sizeof(path),"%s",argv[1]);

    m_arg_data = arg_data;
    g_new_progress_handle = report_progress;

    rpmts ts = NULL;
    struct rpmInstallArguments_s *ia = &rpmIArgs;
    poptContext optCon;
    int ret = 0;

    optCon = rpmcliInit(argc, argv, optionsTable);

    ia->installInterfaceFlags = INSTALL_HASH | INSTALL_PERCENT;
    if (nodeps)
        ia->installInterfaceFlags |= UNINSTALL_NODEPS;

    ts = rpmtsCreate();
    rpmtsSetRootDir(ts, rpmcliRootDir);

    if (!ia->incldocs) {
        if (rpmExpandNumeric("%{_excludedocs}"))
            ia->transFlags |= RPMTRANS_FLAG_NODOCS;
    }

    if (ia->noDeps && nodeps)
        ia->installInterfaceFlags |= UNINSTALL_NODEPS;

    if (ia->prefix) {
        ia->relocations = malloc(2 * sizeof(*ia->relocations));
        ia->relocations[0].oldPath = NULL;   /* special case magic */
        ia->relocations[0].newPath = ia->prefix;
        ia->relocations[1].oldPath = NULL;
        ia->relocations[1].newPath = NULL;
    } else if (ia->relocations) {
        ia->relocations = realloc(ia->relocations,
            sizeof(*ia->relocations) * (ia->numRelocations + 1));
        ia->relocations[ia->numRelocations].oldPath = NULL;
        ia->relocations[ia->numRelocations].newPath = NULL;
    }


    ret = rpmEraseISoftApp(ts, ia, (ARGV_t) poptGetArgs(optCon));
    if (ret != 0) {
        double progress = 110.0;
        //g_progress_handle(&progress, m_arg_data,NULL);
    }

    rpmtsFree(ts);
    ts = NULL;

    rpmcliFini(optCon);
}

/*
* only update use useRpmApi(=true)
*/
bool getAllInstalledPkgs(list<string> &installedList,sqlite3 *db,bool useRpmApi)
{
    bool simple = true;

    if (!useRpmApi) {
        if (!db)
            return false;

        char pkgName[512]="";
        int ret = 0;
        char *sql = sqlite3_mprintf(
                    "SELECT Name,Version,Release,Arch FROM Package WHERE Status = 1 ");
        sqlite3_stmt *stmt;
        const char *tail;
        ret = sqlite3_prepare(db, sql, -1, &stmt, &tail);

        if (ret != SQLITE_OK) {
            //printf("\nERROR:%s,%d,sqlite db prepare error.ret[%d],err[%s]\n",
            //       __FUNCTION__,__LINE__,ret,tail);
            return false;
        }

        ret = sqlite3_step(stmt);

        sqlite3_free(sql);

        if ( ret == SQLITE_ROW) {
            do {

                if (simple) {
                    snprintf(pkgName,sizeof(pkgName),"%s",(char *)sqlite3_column_text(stmt, 0));
                } else {
                    snprintf(pkgName,sizeof(pkgName),"%s-%s-%s.%s",
                             (char *)sqlite3_column_text(stmt, 0),
                             (char *)sqlite3_column_text(stmt, 1),
                             (char *)sqlite3_column_text(stmt, 2),
                             (char *)sqlite3_column_text(stmt, 3));
                }

                string tmp(pkgName);
                installedList.push_back(tmp);

            } while(sqlite3_step(stmt) == SQLITE_ROW);
        }

        sqlite3_finalize( stmt );

        return true;
    }


    char format[16]="";
    rpmts ts = NULL;
    ts = rpmtsCreate();
    rpmtsSetRootDir(ts, "/");
    addMacro(NULL, "_dbpath", NULL, "/var/lib/isoft-app", RMIL_GLOBAL);

    //rpmdbMatchIterator mi = initFilterIteratorISoftApp(ts, NULL);
    rpmdbMatchIterator mi = rpmtsInitIterator(ts, RPMDBI_PACKAGES, NULL, 0);
    Header h;

    if (mi == NULL) {
        rpmtsFree(ts);
        return false;
    }

    if (simple) {
        strcpy(format,"%{n}");
    } else {
        strcpy(format,"%{nvra}");
    }

    while ((h = rpmdbNextIterator(mi)) != NULL) {
        const char *errstr;
        char *str = headerFormat(h, format, &errstr);
        if (str) {
            string tmp(str);
            installedList.push_back(tmp);
            free(str);
            str = NULL;
        }
    }

    rpmdbFreeIterator(mi);
    rpmtsFree(ts);

    return true;
}

bool getInstalledPkgVer(const char *pkgName,char nvrStr[256])
{
    bool ret = false;
    char format[16]="";
    rpmts ts = NULL;
    ts = rpmtsCreate();
    rpmtsSetRootDir(ts, "/");
    addMacro(NULL, "_dbpath", NULL, "/var/lib/isoft-app", RMIL_GLOBAL);

    //rpmdbMatchIterator mi = initFilterIteratorISoftApp(ts, NULL);
    rpmdbMatchIterator mi = rpmtsInitIterator(ts, RPMDBI_PACKAGES, NULL, 0);
    Header h;

    if (mi == NULL || pkgName == NULL || pkgName[0] == 0)
        goto cleanup;

    // name-version-release (no arch)
    strcpy(format,"%{nvr}");

    while ((h = rpmdbNextIterator(mi)) != NULL) {
        const char *errstr;
        char *str = headerFormat(h, format, &errstr);
        if (str) {
            const char *name = headerGetString(h, RPMTAG_NAME);
            if (name == NULL) {
                ret = false;
                break;
            }
            if (strcmp(pkgName,name) == 0) {
                snprintf(nvrStr,255,"%s",str);
                ret = true;
                free(str);
                str = NULL;
                break;
            }
        }
    }

    rpmdbFreeIterator(mi);

cleanup:
    rpmtsFree(ts);

    return ret;
}

#define IAM_RPMQV


int rpmCheck(int argc,char **argv)
{
    int ec = 0;
    rpmts ts = NULL;
    enum modes bigMode = MODE_UNKNOWN;
    poptContext optCon;
    optCon = rpmcliInit(argc, argv, optionsTable);
    ts = rpmtsCreate();
    (void) rpmtsSetRootDir(ts, rpmcliRootDir);

    rpmVerifyFlags verifyFlags = VERIFY_ALL;

#if defined(IAM_RPMQV)
    QVA_t qva = &rpmQVKArgs;
    qva->qva_mode = 'V';
    bigMode = MODE_VERIFY;


    verifyFlags &= ~qva->qva_flags;
    qva->qva_flags = (rpmQueryFlags) verifyFlags;

    if (!poptPeekArg(optCon) && !(qva->qva_source == RPMQV_ALL))
        printf("no arguments given for verify");
    ec = rpmcliVerifyISoftApp(ts, qva, (ARGV_const_t) poptGetArgs(optCon));

#endif
    #ifdef  IAM_RPMQV
        free(qva->qva_queryFormat);
    #endif

    #ifdef  IAM_RPMEIU
        if (ia->relocations != NULL) {
            for (i = 0; i < ia->numRelocations; i++)
                free(ia->relocations[i].oldPath);
            free(ia->relocations);
        }
    #endif

    rpmtsFree(ts);
    ts = NULL;

    rpmcliFini(optCon);
    optCon = NULL;

    return ec;
}

