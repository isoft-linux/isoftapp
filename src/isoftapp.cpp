/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 * Copyright (C) 2015 fujiang <fujiang.zhu@i-soft.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <iostream>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <fcntl.h>
#include <rpm/rpmio.h>
#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <sqlite3.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "splitstring.h"
#include "conf-helper.h"
#include "urihelper.h"
#include "download.h"
#include "rapttypes.h"
#include "rpmpm.h"
#include "daemon.h"
#include "md5.h"
using namespace std;

#undef DEBUG
#undef NOSIZY


#define  ISOFTAPP_CACHE_TIMESTAMP_FILE  ISOFTAPP_CACHE_DIR"/timestamp.cfg"

/*
* to store all pkgs which will be installed.
*
*/
#define  CACHE_ARCHIVES_PATH "/var/cache/isoftapp/archives/"
#define  URI_PATH_MAX  256
typedef struct {
    char pkgName[256]; //name
    char uri[URI_PATH_MAX]; // path to download
    char md5Str[256];
    char size[32];
    char datetime[32];
    char version[64]; // version-resease
} t_PKGS;

list<t_PKGS> toInstPkgList;
list<t_PKGS> toUpPkgList;

typedef struct {
    char pkgName[256];
    char removeName[URI_PATH_MAX];
} t_REMOVEPKGS;

list<t_REMOVEPKGS> toRemovePkgList;

/*
* chkedProPkgList: cache provides pkgs which have been checked.
*/
list<string> chkedProPkgList;

/*
* chkedInsPkgList: cache installed pkgs which have been checked.
*/
list<string> chkedInsPkgList;

list<string> chkedDepList;

/*
* handler for /var/cache/isoftapp/pkgcache.db
*/

// os db && app db
static rpmdb g_osdb = NULL,g_appdb = NULL;

// pkg db
static sqlite3 *g_db = NULL;
#define DB_FILENAME    ISOFTAPP_CACHE_DIR "/pkgcache.db"

t_RPMPATHMODE g_rpmPathMode;

static void usage();

static rpmdb open_osdb(rpmdb db)
{
    if (db)
        return db;
    openDatabase("/", "/var/lib/rpm", &db, O_RDONLY, 0644, 0);
    return db;
}

static rpmdb open_appdb(rpmdb db)
{
    if (db)
        return db;
    openDatabase("/", "/var/lib/isoft-app", &db, O_RDONLY, 0644, 0);
    return db;
}

static void close_rpmdb(rpmdb db) 
{
    if (db) {
        rpmdbClose(db);
        db = NULL;
    }
}

static bool check_rpm_dep(rpmdb db, const char *req) 
{
    if (db == NULL) {
        return false;
    }
    if (rpmdbCountPackages(db, req)         == 0    && 
        rpmdbCountProvides(db, req)         == 0    &&
        !g_file_test(req, G_FILE_TEST_EXISTS)     &&
        strstr(req, "rpmlib")             == NULL) 
    {
        return false;
    }
    return true;
}

static void downloaded_source(const gchar *file_path, const gchar *url) 
{
    // TODO: clear cmd info
    std::cout << "Get: " << url << std::endl;

    // 从url中找出parent uri
    urihelper u(url, "/");
    std::string uri = "";
    // 从file_path中找出file_name
    gchar *file_name = NULL;
    splitstring *s = NULL;
    std::vector<std::string> flds;
    // 分组
    std::string group = "";
    // pkgcache.db采用sqlite3
    std::string pkgcache_tpl = "";
    std::string pkgcache_path = "";
    sqlite3 *db = NULL;

    // 解析rpm头
    FD_t fd = NULL;
    Header hdr = NULL;
    char *name = NULL;
    char *version = NULL;
    char *release = NULL;
    char *md5 = NULL;
    char *arch = NULL;
    char *size = NULL;
    struct rpmtd_s pnames; 
    const char *pname = NULL;
    struct rpmtd_s rnames;
    const char *rname = NULL;
    std::string provides = "";
    std::string requires = "";
    int   ret = 0;

    int Status = 0;
    list<string>::iterator it;
    list<string> instedList;

    fd = Fopen(file_path, "r");
    if (fd == NULL) {
        printf("ERROR: failed to open header file %s\n", file_path);
        goto exit;
    }
    
    file_name = g_path_get_basename(file_path);
    if (file_name == NULL)
        goto exit;
    s = new splitstring(file_name);
    flds = s->split('.');
    if (flds.size() < 2)
        goto exit;
    if (flds[flds.size() - 2] == "srclist") {
        std::cout << "Not parsing srclist at present" << std::endl;
        goto exit;
    }
    group = flds[flds.size() - 1];
    if (g_str_has_prefix(url, "http://")) {
        std::string path = u.path();
        std::size_t pos = path.find("/base");
        if (pos != std::string::npos)
            path = path.substr(0, pos);
        uri = u.scheme() + "://" + u.host() + path;
    } else {
        uri = url;
    }
#if DEBUG
    std::cout << "DEBUG: " << __FILE__ << " " << __PRETTY_FUNCTION__ << " " << uri << std::endl;
#endif
    printf("trace:%s,%d.recreate[%s] by [%s]\n",__FUNCTION__,__LINE__,ISOFTAPP_CACHE_DIR,ISOFTAPP_DATA_DIR);
    bool printOnce = true;
    pkgcache_path = ISOFTAPP_CACHE_DIR + std::string("/pkgcache.db");

    // to check table 'Package' is new or not
    bool isNew = false;
    do {
        if (sqlite3_open(pkgcache_path.c_str(), &db) != 0) {
            db = NULL;
            printf("trace:%s,%d.open[%s] error.\n",__FUNCTION__,__LINE__,pkgcache_path.c_str());
            break;
        }
        char *sql = sqlite3_mprintf("select sql from sqlite_master where name ='Package' ");
        sqlite3_stmt *stmt;
        const char *tail;
        ret = sqlite3_prepare(db, sql, -1, &stmt, &tail);
        if (ret != SQLITE_OK) {
            printf("\nERROR:%s,%d,sqlite db prepare error.ret[%d],err[%s]!!!\n",
                   __FUNCTION__,__LINE__,ret,tail);
            sqlite3_free(sql);
            break;
        }
        ret = sqlite3_step(stmt);
        sqlite3_free(sql);
        sql = NULL;
        char buffer[1024]="";
        if ( ret == SQLITE_ROW) {
            do {
                snprintf(buffer,sizeof(buffer),"%s",(char *)sqlite3_column_text(stmt, 0));
                if(strstr(buffer,"Datetime") != NULL) {
                    isNew = true;
                    break;
                }
            } while(sqlite3_step(stmt) == SQLITE_ROW);
        }
        sqlite3_finalize( stmt );
    } while(0);
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
    printf("trace:%s,%d.[%s] is %s.\n",__FUNCTION__,__LINE__,pkgcache_path.c_str(),isNew?"new":"old");

    if (!isNew) {
        unlink(pkgcache_path.c_str());
        pkgcache_tpl = ISOFTAPP_DATA_DIR + std::string("/pkgcache.db");
        link(pkgcache_tpl.c_str(), pkgcache_path.c_str());
    }
    if (sqlite3_open(pkgcache_path.c_str(), &db) != 0) 
        goto exit;
    sqlite3_exec(db, "PRAGMA synchronous = OFF; ", NULL, 0, NULL);

    getAllInstalledPkgs(instedList,db,true);

    while (hdr = headerRead(fd, HEADER_MAGIC_YES)) {
        name = headerGetAsString(hdr, RPMTAG_NAME);
        version = headerGetAsString(hdr, RPMTAG_VERSION);
        release = headerGetAsString(hdr, RPMTAG_RELEASE);
        md5 = headerGetAsString(hdr, CRPMTAG_MD5);
        arch = headerGetAsString(hdr, RPMTAG_ARCH);
        size = headerGetAsString(hdr, RPMTAG_SIZE);

        if (name == NULL)
            continue;

        // TODO: too nosizy ;P
#ifdef NOSIZY
        printf("%s-%s-%s-%s-size[%s]\n", name, version, release, arch,size);
        printf("%s\n", md5);
#endif
        headerGet(hdr, RPMTAG_PROVIDENAME, &pnames, HEADERGET_ARGV);
#ifdef NOSIZY
        printf("* provide: %d\n", rpmtdCount(&pnames));
#endif
        provides = "";
        while (pname = rpmtdNextString(&pnames)) {
#ifdef NOSIZY
            printf("  %s\n", pname);
#endif
            provides += std::string(pname) + ",";
        }

        headerGet(hdr, RPMTAG_REQUIRENAME, &rnames, HEADERGET_ARGV);
#ifdef NOSIZY
        printf("* require: %d\n", rpmtdCount(&rnames));
#endif
        requires = "";
        while (rname = rpmtdNextString(&rnames)) {
#ifdef NOSIZY
            printf("  %s\n", rname);
#endif
            requires += std::string(rname) + ",";
        }
#ifdef NOSIZY
        printf("\n\n");
#endif

        char* errmsg = NULL;
#if 0
        char *sql = sqlite3_mprintf("DELETE FROM Package WHERE Name = '%s'", name);
        sqlite3_exec(db, sql, NULL, 0, NULL);
        sqlite3_free(sql);

        Status = 0;
        for(it = instedList.begin(); it != instedList.end();it++) {
            if (strcmp((*it).c_str(),name) ==0 ) {
                Status = 1;
                break;
            }
        }

        sql = sqlite3_mprintf("INSERT INTO Package (Name, Version, "
                "Release, Arch, IGroup, Uri, Requires, Provides, Md5,Status,Size,Datetime) VALUES("
                "'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',%d,'%s',datetime('now','localtime'));", name,
                version, release, arch, group.c_str(), uri.c_str(), 
                requires.c_str(), provides.c_str(), md5,Status,size);

#else
        char *sql = sqlite3_mprintf("SELECT Name FROM Package WHERE Name = '%s'", name);
        sqlite3_stmt *stmt;
        const char *tail;
        ret = sqlite3_prepare(db, sql, -1, &stmt, &tail);
        if (ret != SQLITE_OK) {
            printf("\nERROR:%s,%d,sqlite db prepare error.ret[%d],err[%s]!!!\n",
                   __FUNCTION__,__LINE__,ret,tail);
            sqlite3_free(sql);
            goto exit;
        }
        ret = sqlite3_step(stmt);
        sqlite3_free(sql);
        sql = NULL;
        char strName[256]="";
        if ( ret == SQLITE_ROW) {
            do {
                snprintf(strName,sizeof(strName),"%s",(char *)sqlite3_column_text(stmt, 0));

            } while(sqlite3_step(stmt) == SQLITE_ROW);
        }
        sqlite3_finalize( stmt );
        if (strName[0] != 0) {
            Status = 0;
            for(it = instedList.begin(); it != instedList.end();it++) {
                if (strcmp((*it).c_str(),name) ==0 ) {
                    Status = 1;
                    break;
                }
            }
            // do not update datetime
            sql = sqlite3_mprintf("UPDATE Package SET Version = '%s',Release='%s',Arch = '%s',IGroup='%s',"
                                  " Uri='%s',Requires='%s', Provides='%s', Md5='%s',Size='%s', Status =%d "
                                  " WHERE Name = '%s'",
                                  version, release, arch, group.c_str(),
                                  uri.c_str(),requires.c_str(), provides.c_str(), md5,size,Status,
                                  name);
            //printf("\n%s,%d,sqlite sql[%s]!!!\n",
            //       __FUNCTION__,__LINE__,sql);
        } else {
            Status = 0;
            for(it = instedList.begin(); it != instedList.end();it++) {
                if (strcmp((*it).c_str(),name) ==0 ) {
                    Status = 1;
                    break;
                }
            }
            // 已安装的，设置安装日期为现在;未安装的，不设置安装日期;
            if (Status == 1) {
                sql = sqlite3_mprintf("INSERT INTO Package (Name, Version, "
                    "Release, Arch, IGroup, Uri, Requires, Provides, Md5,Status,Size,Datetime) VALUES("
                    "'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',%d,'%s',datetime('now','localtime'));", name,
                    version, release, arch, group.c_str(), uri.c_str(),
                    requires.c_str(), provides.c_str(), md5,Status,size);

            } else {
                sql = sqlite3_mprintf("INSERT INTO Package (Name, Version, "
                    "Release, Arch, IGroup, Uri, Requires, Provides, Md5,Status,Size) VALUES("
                    "'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',%d,'%s');", name,
                    version, release, arch, group.c_str(), uri.c_str(),
                    requires.c_str(), provides.c_str(), md5,Status,size);
            }
        }

#endif
        if (printOnce) {
            printOnce = false;
            printf("\nto test sql[%s]\n",sql);
        }

        ret = sqlite3_exec(db, sql, NULL, 0, &errmsg);
        if (ret != SQLITE_OK) {
            printf("\nERROR: sqlite exec error.ret[%d]errmsg[%s]sql[%s]\n",ret,errmsg,sql);
            if(errmsg != NULL) {
                sqlite3_free(errmsg);
                errmsg = NULL;
            }
            sqlite3_free(sql);
            goto exit;
        }

        sqlite3_free(sql);
        sql = NULL;

        headerFree(hdr);
        hdr = NULL;
    }

exit:
    instedList.clear();
    if (fd) {
        Fclose(fd);
        fd = NULL;
    }

    if (file_name) {
        g_free(file_name);
        file_name = NULL;
    }

    if (s) {
        delete s;
        s = NULL;
    }

    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}

static void downloaded_source_error(const gchar *file_path, 
                                    const gchar *url, 
                                    CURLcode error) 
{
    std::cout << "ERROR: failed to download from " << url << std::endl;
}

void update();

/*
* ISOFTAPP_CACHE_DIR /var/cache/isoftapp
* dbname:pkgcache.db
*/
bool needUpdate()
{
    char dbFile[512]="";
    struct stat st;
    bool needUpd = false;
    time_t now = time(NULL);
    GError *error = NULL;
    GDBusNodeInfo *node;
    gchar *contents;

    snprintf(dbFile,sizeof(dbFile),"%s/pkgcache.db",ISOFTAPP_CACHE_DIR);
    if (g_file_test(dbFile,G_FILE_TEST_EXISTS) ) {
        memset(&st,0,sizeof(struct stat));
        if (stat(dbFile,&st) != 0) {
            needUpd = true;
        } else {
            if (st.st_size <= 7168) {
                needUpd = true;
            }
        }

    } else {
        needUpd = true;
    }

    if (!needUpd) {
        if (g_file_test(ISOFTAPP_CACHE_TIMESTAMP_FILE,G_FILE_TEST_EXISTS) ) {

            if (!g_file_get_contents(ISOFTAPP_CACHE_TIMESTAMP_FILE, &contents, NULL, &error)) {
                printf("\n%s,%d,Failed to get contents from file:%s,error info[%s]\n",__FUNCTION__,__LINE__,
                          ISOFTAPP_CACHE_TIMESTAMP_FILE, error->message);
                g_error_free(error);
                needUpd = true;
            } else {
                struct tm *tm_now;
                tm_now = localtime(&now);
                if (tm_now == NULL) {
                    needUpd = true;
                    goto exitit;
                }
                char today[32]="";
                char yestoday[128]="";
                snprintf(yestoday,sizeof(yestoday),"%s",(char *)contents);
                g_free(contents);
                contents = NULL;
                time_t t2 = (time_t )atol(yestoday);
                snprintf(today,sizeof(today),"%d%02d%02d",
                         tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday);
                tm_now = NULL;
                tm_now = localtime(&t2);
                if (tm_now == NULL) {
                    needUpd = true;
                    goto exitit;
                }
                snprintf(yestoday,sizeof(yestoday),"%d%02d%02d",
                         tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday);
                if(strcmp(today,yestoday) > 0) {
                    needUpd = true;
                }
            }
        } else {
            needUpd = true;
        }

    }

exitit:

    return needUpd;

}

void update()
{
    isoftapp::ConfHelper *conf = new isoftapp::ConfHelper;
    isoftapp::sources_t sources = conf->sources();
    DownloadData *data = NULL;
    std::string base_dir = ISOFTAPP_CACHE_DIR + std::string("/base");
    std::string list_type = "";
    std::string file_path = "";
    std::string url = "";

    if (g_mkdir_with_parents(base_dir.c_str(), 0755) == -1) {
        printf("ERROR: failed to create base directory\n");
        return;
    }
    for (unsigned int i = 0; i < sources.size(); i++) {
        if (!g_str_has_prefix(sources[i].url.c_str(), "http://") && 
            !g_str_has_prefix(sources[i].url.c_str(), "file://")) 
        {
            continue;
        }

        list_type = sources[i].type == isoftapp::RPM ? "pkglist" : "srclist";
        if (g_str_has_prefix(sources[i].url.c_str(), "http://")) {
            data = download_data_new();
            if (!data)
                continue;
            urihelper u(sources[i].url);
            file_path = base_dir + "/" + u.host() + u.path() + "." + list_type 
                + "." + sources[i].group;
            url = sources[i].url + "/base/" + list_type + "." + sources[i].group;
            data->file_path = file_path.c_str();
            data->url = url.c_str();
            data->handler = downloaded_source;
            data->handler_error = downloaded_source_error;
            download_routine(data);
        } else if (g_str_has_prefix(sources[i].url.c_str(), "file://")) {
            file_path = sources[i].url + "/base/" + list_type + "." + sources[i].group;
            file_path = file_path.substr(7, file_path.size() - 7);
            downloaded_source(file_path.c_str(), sources[i].url.c_str());
        }
    }
    std::cout << "Reading Package Lists... Done" << std::endl;
    std::cout << "Building Dependency Tree... Done" << std::endl;

    delete conf;
    conf = nullptr;

    time_t now = time(NULL);
    GError *error = NULL;
    char  nowStr[32]="";

    snprintf(nowStr,sizeof(nowStr),"%ld",now);
    if (!g_file_set_contents(ISOFTAPP_CACHE_TIMESTAMP_FILE,
                        nowStr,
                        strlen(nowStr),
                        &error) ) {

        printf("\n%s,%d,Failed to set file:%s,error info[%s]\n",__FUNCTION__,__LINE__,
                                  ISOFTAPP_CACHE_TIMESTAMP_FILE, error->message);
        g_error_free(error);

    }

}

/*
* 新封装一个接口，下载文件@src 到文件@dst
*/
int download_file(const char *src_file,const char *dst_file)
{
    DownloadData *data = NULL;
    data = download_data_new();
    if (!data)
        return -1;

    data->file_path = dst_file;
    data->url = src_file;
    download_routine(data);
    return 0;
}

static int openPkgDB()
{
    int ret = 0;
    if (g_db)
        return 0;

    sqlite3_initialize();
    ret = sqlite3_open(DB_FILENAME, &g_db);
    if (ret != SQLITE_OK) {
        std::cout << "Error,can not open db file:" << DB_FILENAME << std::endl;
        return -1;
    }

    return 0;
}

static int closePkgDB()
{
    if (g_db) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
    sqlite3_shutdown();

    return 0;
}

static bool uptDB(sqlite3 *db,const char *pkgName,const char *colunName,int value)
{
    if (!db) {
        return false;
    }
    int ret = 0;
    char *errmsg = NULL;
    char *sql = sqlite3_mprintf("UPDATE Package SET %s = %d ,"
                                "Datetime = datetime('now','localtime')  WHERE Name = '%s' " ,
            colunName,value,pkgName);

    printf("\n trace:sql[%s]\n",sql);

    ret = sqlite3_exec(db, sql, NULL, 0, &errmsg);
    if (ret != SQLITE_OK) {
        printf("\nERROR: sqlite exec error.ret[%d]errmsg[%s]sql[%s]\n",ret,errmsg,sql);
        if (errmsg) {
            sqlite3_free(errmsg);
            errmsg = NULL;
        }
        sqlite3_free(sql);
        return false;
    }

    sqlite3_free(sql);
    sql = NULL;
    return true;
}

/*
* no.1 install pkg a.
* no.2 pkg a need pkg b(to find b's real pkg name).
* no.3 then to check b, to find is in chkedProPkgList or not.
* no.4 if not exist in chkedProPkgList, insert b into chkedProPkgList.
*
*/
static bool chkProPkg(const char *pkgName, char action)
{
    bool found = false;
    string str(pkgName);

    if (action == 'i') {
        chkedProPkgList.push_back(str);
        return true;
    }

    list<string>::iterator it = find( chkedProPkgList.begin(), chkedProPkgList.end(), str );
    if (it != chkedProPkgList.end() ) {
        found = true;
    } else {
        found = false;
    }

    return found;
}

/*
* (if pkg a is installed, then insert into chkedInsPkgList).
*
* if find pkg a in chkedInsPkgList, return true(do not check a in sysdb/appdb).
*
*/
static bool chkInsPkg(const char *pkgName)
{
    bool found = false;
    string str(pkgName);

    list<string>::iterator it = find( chkedInsPkgList.begin(), chkedInsPkgList.end(), str );
    if (it != chkedInsPkgList.end() ) {
        found = true;
    } else {
        found = false;
    }

    return found;
}

bool insertChkPkgList(const char *pkgName)
{
    bool found = false;
    string str(pkgName);

    list<string>::iterator it = find( chkedInsPkgList.begin(), chkedInsPkgList.end(), str );
    if (it != chkedInsPkgList.end() ) {
        found = true;
    } else {
        found = false;
    }

    if(!found)
        chkedInsPkgList.push_back(str);

    return true;
}

void realUpgrade(const char *pkgName,bool onlyGetUpgPkg,string &upgPkgs);

bool pkgIsInstalled(const char *pkgName,bool isApp,bool reInstall=false)
{
    bool isInstalled = false;

    if (chkInsPkg(pkgName)) {
        return true;
    }

    if (g_osdb == NULL) {
        g_osdb = open_osdb(g_osdb);
        if (g_osdb == NULL) {
            std::cout << "Install:Error! Can not open os db.\n" << std::endl;
            return false;
        }
    }

    if (g_appdb == NULL) {
        g_appdb = open_appdb(g_appdb);
        if (g_appdb == NULL) {
            //close_rpmdb(osdb);
            std::cout << "Install:Error! Can not open app db.\n" << std::endl;
            return false;
        }
    }
    if (!isApp)
    isInstalled = check_rpm_dep( g_osdb, pkgName);

    if (!isInstalled) {
        isInstalled = check_rpm_dep( g_appdb, pkgName);
    }

    if (isInstalled || reInstall) {
        insertChkPkgList(pkgName);
    }

    if (reInstall)
        return false;

    return isInstalled;

}
vector< string> split( string str, string pattern)
{
    vector<string> ret;
    if (pattern.empty())
        return ret;
    size_t start=0,index=str.find_first_of(pattern,0);
    while (index!=str.npos) {
        if(start!=index)
        ret.push_back(str.substr(start,index-start));
        start=index+1;
        index=str.find_first_of(pattern,start);

    }

    if(!str.substr(start).empty())
        ret.push_back(str.substr(start));
    return ret;

}
bool isFileExist(const char *file_path,const char *md5Str)
{
    bool ret = false;

    if(!file_path || !md5Str)
        return ret;

    if (g_file_test(file_path, G_FILE_TEST_EXISTS) ) {
        unsigned char md5num[16] = {'\0'};
        char md5sumStr[33] = {'\0'};

        if (md5_from_file(file_path, md5num) == -1) {
            printf("ERROR: failed to get md5sum for %s\n", file_path);
            return ret;
        }
        md5_num2str(md5num, md5sumStr);

        //printf("\n file[%s]md5sumStr[%s] vs [%s] %s\n", file_path,md5sumStr,md5Str);

        if(strcmp(md5sumStr,md5Str) == 0)
            ret = true;
    }

    return ret;
}
int g_installed_num = 0;
void install(int argc, char *argv[],
             void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
             void *arg_data,
             bool reInstall);
static bool realInstall(void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
                        void *arg_data)

{
    list<t_PKGS>::iterator i;

    int argc = toInstPkgList.size() + 1;
    char **argv = NULL;
    int j = 0;
    bool done = false;
    if (argc <= 1) {
        return true;
    }

    printf("\ntrace:%s,%d.argc[%d]\n",__FUNCTION__,__LINE__,argc);

    argv =  (char **)malloc(sizeof(char *) * argc);
    if (argv == NULL) {
        cout<<"ERROR: out of memmory."<< endl;
        return false;
    }

    memset(argv, 0, sizeof(char *) * argc);

    for (j=0; j < argc; j++) {
        argv[j] = (char *)malloc(sizeof(char) * URI_PATH_MAX);
        if (argv[j] == NULL) {
            cout<<"ERROR: out of memmory."<< endl;
            goto cleanup;
        }
        memset(argv[j], 0, sizeof(char) * URI_PATH_MAX);
    }

    snprintf(argv[0], strlen(argv[0]) - 1, __func__);

    for(i = toInstPkgList.begin(),j=1; i != toInstPkgList.end();++i) {

        //snprintf(argv[j], URI_PATH_MAX-1,"%s", (*i).uri);
        //
#if 1
        DownloadData *data = NULL;
        char file_path[512]="";
        std::string url = "";
        char *p = strrchr((*i).uri,'/');
        if (p == NULL) {
            printf("\ntrace:%s,%d.url[%s] error!!!!!\n",__FUNCTION__,__LINE__,(*i).uri);
            continue;
        }
        p++;
        if (g_rpmPathMode.path[0] == 0)
            snprintf(file_path,sizeof(file_path),"%s/rpms/%s",ISOFTAPP_CACHE_DIR,p) ;
        else {
            snprintf(file_path,sizeof(file_path),"%s/%s",g_rpmPathMode.path,p) ;
        }

        printf("\ntrace:%s,%d.file[%s],md5[%s]\n",__FUNCTION__,__LINE__,file_path,(*i).md5Str);

        g_installed_num ++;

        if (isFileExist(file_path, (*i).md5Str) ) {
            snprintf(argv[j], URI_PATH_MAX-1,"%s", file_path);
            done = true;
            j++;
            printf("\ntrace:%s,%d.file exist,do not dl[%s]\n",__FUNCTION__,__LINE__,file_path);
            continue;
        }
        data = download_data_new();
        if (!data)
            continue;

        url = (*i).uri;
        data->file_path = file_path;
        data->url = url.c_str();
        data->handler = downloaded_source;
        data->handler_error = downloaded_source_error;
        data->handler_progress = report_progress;
        data->arg_data = arg_data;
        download_routine(data);

        snprintf(argv[j], URI_PATH_MAX-1,"%s", file_path);
        done = true;
        j++;
#else
        snprintf(argv[j], URI_PATH_MAX-1,"%s", (*i).uri);
        done = true;
        j++;
#endif

    }

    if (done) {
        rpmInstall(j,argv,report_progress,arg_data);

        static sqlite3 *upt_db = NULL;
        int rc = sqlite3_open_v2( DB_FILENAME, &upt_db, SQLITE_OPEN_READWRITE, NULL );
        if ( rc != SQLITE_OK){
            printf("\n\n\ntrace:%s,%d.error!Cannot go here!!!open db file[%s]error\n\n",__FUNCTION__,__LINE__,DB_FILENAME);
            sqlite3_close( upt_db );
            goto cleanup;
        }
        // to deal with xxx.rpm:1-delete now;2-after one week;other-pass
        if (g_rpmPathMode.mode == 1) {
            for(int x =1;x<j;x++) {
                unlink(argv[x]);
            }
        }

        for(i = toInstPkgList.begin(),j=1; i != toInstPkgList.end();++i,j++) {
            snprintf(argv[1], URI_PATH_MAX-1,"%s", (*i).pkgName);
            int ret = rpmCheck(2,argv);
            if (ret == 0)
                uptDB(upt_db,(*i).pkgName,"Status",1);
        }
        sqlite3_close( upt_db );
        upt_db = NULL;
    }


cleanup:
    if (argv) {
        for (j = 0; j < argc; j++) {
            if (argv[j]) {
                free(argv[j]);
                argv[j] = NULL;
            }
        }
        free(argv);
        argv = NULL;
    }

    return true;
}

bool insertInstPkgList(t_PKGS *pkg)
{
    bool find = false;
    list<t_PKGS>::iterator i;

    if (pkg == NULL)
        return false;

    for(i = toInstPkgList.begin(); i != toInstPkgList.end();++i) {
        if(strcmp((*i).pkgName,pkg->pkgName) == 0) {
            find = true;
            break;
        }
    }

    if(!find)
        toInstPkgList.push_back(*pkg);

    return true;
}

/*
* to get pkg name by Require pkgName and inser into providePkgsList
*/
//list<string> providePkgsList;
static bool getEveryProvidePkgName(char *pkgName,list<string> &providePkgsList)
{
    int ret = 0;
    char *sql = sqlite3_mprintf(
                "SELECT Name,Provides FROM Package WHERE Provides like '%%%s%%' ",
                pkgName);
    sqlite3_stmt *stmt;
    const char *tail;
    ret = sqlite3_prepare(g_db, sql, -1, &stmt, &tail);

    if (ret != SQLITE_OK) {
        printf("\nERROR:%s,%d,sqlite db prepare error.ret[%d],err[%s]\n",
               __FUNCTION__,__LINE__,ret,tail);
        return false;
    }

    ret = sqlite3_step(stmt);

    sqlite3_free(sql);

    if ( ret == SQLITE_ROW) {
        do {
            #ifdef DEBUG
            printf("%d----\n0[%s]\n1[%s]\n2[%s]- \n",__LINE__,
                (char *)sqlite3_column_text(stmt, 0),
                (char *)sqlite3_column_text(stmt, 1),
                (char *)sqlite3_column_text(stmt, 2));
            #endif

            string provides((char *)sqlite3_column_text(stmt, 1));
            string pattern =",";

            // result: store provides pkgs;
            vector< string> result=split(provides,pattern);
            for( int i = 0; i < result.size(); i++) {
                if (strcmp(result[i].c_str() ,pkgName) == 0) {

                    list<string>::iterator it =
                            find( providePkgsList.begin(), providePkgsList.end(), pkgName );
                    if (it == providePkgsList.end() ) {
                        string str((char *)sqlite3_column_text(stmt, 0));
                        providePkgsList.push_back(str);
                    }
                    break;
                }
            }


        } while(sqlite3_step(stmt) == SQLITE_ROW);
    }

    sqlite3_finalize( stmt );

    return true;

}

/*
* to install @pkgName if it is not been installed
*
* procedure:
* no.1 check @pkgName is installed or not.
* no.1.1 if need installation, insert into list @toInstPkgList.
* no.2 get required pkg list.
* no.2.1 to check if the required pkg is installed or not.
* no.2.2 if the required pkg is not been installed,call doInstall() recursively.
*
*/
static bool doInstall(const char *pkgName,bool reInstall=false)
{
    if (pkgName == NULL || pkgName[0] == 0)
        return true;

    if (!reInstall) {
    bool isInstalled = pkgIsInstalled(pkgName,false,reInstall);
    if (isInstalled) {
        return true;
    }

    }

    printf("\ntrace:%s,%d.pkgName[%s]--[%s]\n",__FUNCTION__,__LINE__,pkgName,reInstall?"re":"not reinstall");

    // pkgName is checked,do not re check
    if(chkProPkg(pkgName,'s'))
        return true;

    int ret = 0;
    char *sql = sqlite3_mprintf("SELECT Name,Version,Release,Arch,IGroup,Uri,Requires,Md5 " // 0-7
                                " FROM Package WHERE Name=%Q ", pkgName);
    sqlite3_stmt *stmt;
    const char *tail;
    ret = sqlite3_prepare(g_db, sql, -1, &stmt, &tail);
    sqlite3_free(sql);
    if (ret != SQLITE_OK) {
        ret = 0;
        printf("\nERROR:sqlite db prepare error.%d\n",__LINE__);
        goto cleanup;
    }
    ret = sqlite3_step(stmt);

    if ( ret == SQLITE_ROW) {
        do {
            #ifdef DEBUG
            printf("%d----\n0[%s]\n1[%s]\n2[%s]\n3[%s]\n4[%s]\n5[%s]\n6[%s]- \n",__LINE__,
                (char *)sqlite3_column_text(stmt, 0),
                (char *)sqlite3_column_text(stmt, 1),
                (char *)sqlite3_column_text(stmt, 2),
                (char *)sqlite3_column_text(stmt, 3),
                   (char *)sqlite3_column_text(stmt, 4),
                   (char *)sqlite3_column_text(stmt, 5),
                   (char *)sqlite3_column_text(stmt, 6));
            #endif

            // insert into list
            // uri: 5/RPMS.4/0-1-2.3.rpm
            t_PKGS pkg;
            memset(&pkg,0,sizeof(t_PKGS));
            snprintf(pkg.pkgName,sizeof(pkg.pkgName),"%s",pkgName);
            snprintf(pkg.uri,sizeof(pkg.uri),"%s/RPMS.%s/%s-%s-%s.%s.rpm",
                     (char *)sqlite3_column_text(stmt, 5),
                     (char *)sqlite3_column_text(stmt, 4),
                     pkgName,
                     (char *)sqlite3_column_text(stmt, 1),
                     (char *)sqlite3_column_text(stmt, 2),
                     (char *)sqlite3_column_text(stmt, 3));
            snprintf(pkg.md5Str,sizeof(pkg.md5Str),"%s",(char *)sqlite3_column_text(stmt, 7));

            // pkg will be installed, so insert into toInstPkgList
            //toInstPkgList.push_back(pkg);
            insertInstPkgList(&pkg);
            chkProPkg(pkgName,'i');


            string str((char *)sqlite3_column_text(stmt, 6) );
            string pattern =",";
            // result: store required pkgs;
            vector< string> result=split(str,pattern);
            for( int i=0; i<result.size(); i++) {
                char pkgName[512]="";
                snprintf(pkgName,sizeof(pkgName),"%s",result[i].c_str());

                // to check every required pkg;
                bool isInstalled = pkgIsInstalled(pkgName,false,false); // reinstall /
                printf("\ntrace:%s,%d.pkgName[%s]--[%s]\n",__FUNCTION__,__LINE__,pkgName,isInstalled?"in":"not");
                if (isInstalled)
                    continue;



                // required pkg need to be installed.
                // to get pkg name  by pkgName
                // (select Name from db where Provides like %pkgName%;
                //providePkgsList.clear();
                list<string> providePkgsList;
                if (!getEveryProvidePkgName(pkgName,providePkgsList)) {
                    continue;
                }

                list<string>::iterator it;
                for(it = providePkgsList.begin(); it != providePkgsList.end();it++) {
                    char pkg[512]="";
                    snprintf(pkg,sizeof(pkg),"%s",(*it).c_str());
                    if(pkg[0] == 0)
                        continue;

                    doInstall(pkg);
                }
            }

        } while(sqlite3_step(stmt) == SQLITE_ROW);
    }

    sqlite3_finalize( stmt );
cleanup:

    return true;
}


bool chkRemovePkg(const char *pkgName,char action,t_REMOVEPKGS *pkg)
{
    bool find = false;
    list<t_REMOVEPKGS>::iterator i;

    if (pkgName == NULL || pkgName[0]==0)
        return false;

    for(i = toRemovePkgList.begin(); i != toRemovePkgList.end();++i) {
        if(strcmp((*i).pkgName,pkgName) == 0) {
            find = true;
            break;
        }
    }

    if ((action == 'i' || action == 's') && !find) {
        if (pkg == NULL) {
            pkg = new t_REMOVEPKGS;
            snprintf(pkg->pkgName, sizeof(pkg->pkgName) - 1,"%s", pkgName);
            snprintf(pkg->removeName, sizeof(pkg->removeName) - 1,"%s", pkgName);
        }
        toRemovePkgList.push_back(*pkg);
    }

    return find;
}

/*
* to get pkg name by provide Name
* for every Provide pkg(Provide_pkg_name), to check if the Requires pkgs
*        include Provide pkg or not
*        [select pkgname as toRemovepkg from appdb where Requires like '%Provide_pkg_name%'];
* */
bool getNameByprovideName(char *provideName,list<string> &RemovepkgList)
{
    int ret = 0;
    char *sql = sqlite3_mprintf(
                "SELECT Name ,Requires FROM Package WHERE Requires like'%%%s%%'",
                provideName);

    sqlite3_stmt *stmt;
    const char *tail;
    ret = sqlite3_prepare(g_db, sql, -1, &stmt, &tail);

    if (ret != SQLITE_OK) {
        printf("\nERROR:%s,%d,sqlite db prepare error.ret[%d],err[%s]\n",
               __FUNCTION__,__LINE__,ret,tail);
        return false;
    }

    ret = sqlite3_step(stmt);

    sqlite3_free(sql);

    if ( ret == SQLITE_ROW) {
        do {
            #ifdef DEBUG
            printf("%d----\n0[%s]\n1[%s]\n2[%s]- \n",__LINE__,
                (char *)sqlite3_column_text(stmt, 0),
                (char *)sqlite3_column_text(stmt, 1),
                (char *)sqlite3_column_text(stmt, 2));
            #endif

            string requires((char *)sqlite3_column_text(stmt, 1));
            string pattern =",";

            // result: store requires pkgs;
            vector< string> result=split(requires,pattern);
            for( int i = 0; i < result.size(); i++) {
                if (strcmp(result[i].c_str() ,provideName) == 0) {
                    list<string>::iterator it =
                            find( RemovepkgList.begin(), RemovepkgList.end(), provideName );
                    if (it == RemovepkgList.end() ) {
                        string str((char *)sqlite3_column_text(stmt, 0));
                        RemovepkgList.push_back(str);
                    }
                    break;
                }
            }

        } while(sqlite3_step(stmt) == SQLITE_ROW);
    }

    sqlite3_finalize( stmt );

    return true;
}

/*
* to remove @pkgName if it is installed.
*
* procedure:
* no.1 check @pkgName is installed or not.
* no.1.1 if is installed, insert into list @toRemovePkgList.
* no.2 get @pkgName's Provides pkgs.
*      [select Provides from appdb where pkgname = @pkgName];
* no.2.1 for every Provide pkg(Provide_pkg_name), to check if the Requires pkgs
*        include Provide pkg or not
*        [select pkgname as toRemovepkg from appdb where Requires like '%Provide_pkg_name%'];
* no.2.2 if toRemovepkg is installed,call doRemove() recursively.
* no.3 if not installed ,return;
*/
static bool doRemove(const char *pkgName)
{
    if (pkgName == NULL || pkgName[0] == 0)
        return true;

    bool isInstalled = pkgIsInstalled(pkgName,true);
    if (!isInstalled) {
        return true;
    }

#ifdef DEBUG
    printf("\ntrace:%s,%d,pkgName1[%s].\n",__FUNCTION__,__LINE__,pkgName);
#endif

    // pkgName is checked(is in toRemovePkgList),do not re check
    if(chkRemovePkg(pkgName,'s',NULL))
        return true;

    int ret = 0;
    char *sql = sqlite3_mprintf(
                "SELECT Version,Release,Arch,Provides FROM Package WHERE Name=%Q ",
                pkgName);
    sqlite3_stmt *stmt;
    const char *tail;
    ret = sqlite3_prepare(g_db, sql, -1, &stmt, &tail);
    sqlite3_free(sql);
    if (ret != SQLITE_OK) {
        ret = 0;
        printf("\nERROR:sqlite db prepare error.%d\n",__LINE__);
        goto cleanup;
    }
    ret = sqlite3_step(stmt);

    if ( ret == SQLITE_ROW) {
        do {
            #ifdef DEBUG
            printf("%d----\n0[%s]\n1[%s]\n2[%s]\n3[%s]\n4[%s]--- \n",__LINE__,
                (char *)sqlite3_column_text(stmt, 0),
                (char *)sqlite3_column_text(stmt, 1),
                   (char *)sqlite3_column_text(stmt, 2),
                   (char *)sqlite3_column_text(stmt, 3),
                   (char *)sqlite3_column_text(stmt, 4));
            #endif

            t_REMOVEPKGS pkg;
            snprintf(pkg.pkgName,sizeof(pkg.pkgName),"%s",pkgName);
            //python-crypto-2.6.1-9.x86_64
            snprintf(pkg.removeName,sizeof(pkg.removeName),"%s-%s-%s.%s",
                     pkgName,
                     (char *)sqlite3_column_text(stmt, 0),
                     (char *)sqlite3_column_text(stmt, 1),
                     (char *)sqlite3_column_text(stmt, 2));

            // all pkgs need to be removed will insert into this list
            chkRemovePkg(pkgName,'i',&pkg);

            // provides
            string str((char *)sqlite3_column_text(stmt, 3) );
            string pattern =",";
            // result: store Provides pkgs;
            vector< string> result=split(str,pattern);
            for( int i=0; i<result.size(); i++) {
                char provideName[512]="";
                snprintf(provideName,sizeof(provideName),"%s",result[i].c_str());

                list<string> toRemovepkgList;
                if (!getNameByprovideName(provideName,toRemovepkgList)) {
                    continue;
                }

                list<string>::iterator it;
                for(it = toRemovepkgList.begin(); it != toRemovepkgList.end();it++) {
                    char pkg[512]="";
                    snprintf(pkg,sizeof(pkg),"%s",(*it).c_str());
                    if(pkg[0] == 0)
                        continue;

                    doRemove(pkg);
                }
            }

        } while(sqlite3_step(stmt) == SQLITE_ROW);
    }

    sqlite3_finalize( stmt );
cleanup:

    return true;
}

static bool realRemove(void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
                       void *arg_data,
                       bool nodeps = true)
{
    list<t_REMOVEPKGS>::iterator i;

    int argc = toRemovePkgList.size() + 1;
    char **argv = NULL;
    int j = 0;
    bool done = false;
    if (argc <= 1) {
        return true;
    }

    argv = (char **)malloc(sizeof(char *) * argc);
    if (argv == NULL) {
        cout<<"ERROR: out of memmory."<< endl;
        return false;
    }

    memset(argv, 0, sizeof(char *) * argc);

    for (j = 0; j < argc; j++) {
        argv[j] = (char *)malloc(sizeof(char) * URI_PATH_MAX);
        if (argv[j] == NULL) {
            cout<<"ERROR: out of memmory."<< endl;
            goto cleanup;
        }
        memset(argv[j], 0, sizeof(char) * URI_PATH_MAX);
    }

    snprintf(argv[0], strlen(argv[0]) - 1, __func__);

    std::cout << "These packages will be removed:" << std::endl;
    for (i = toRemovePkgList.begin(),j=1; i != toRemovePkgList.end();++i,j++) {
        snprintf(argv[j], URI_PATH_MAX-1,"%s", (*i).removeName);
        done = true;

        uptDB(g_db,(*i).pkgName,"Status",0);
    }
    //std::cout << "Do you really want to remove? [y/n]" << std::endl;

    //char c = cin.get();
    if (done/* && (c == 'y' || c == '\n')*/)
        rpmremove(argc, argv,report_progress,arg_data, nodeps);

cleanup:
    if (argv) {
        for (j = 0; j < argc; j++) {
            if (argv[j]) {
                free(argv[j]);
                argv[j] = NULL;
            }
        }
        free(argv);
        argv = NULL;
    }

    toRemovePkgList.clear();
    return true;
}

void clearInstallList()
{
    chkedInsPkgList.clear();
    toInstPkgList.clear();
    chkedProPkgList.clear();
    chkedDepList.clear();
    toRemovePkgList.clear();
    toUpPkgList.clear();

}

void upgrade(int argc, char *argv[],
             void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
             void *arg_data);

void install(int argc, char *argv[],
             void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
             void *arg_data,
             bool reInstall)
{
    char pkgName[512]="";
    bool isInstalled = false;
    bool needInst = false;
    if (argc < 3) {
        usage();
        return;
    }

    printf("\ntrace:%s,%d.[%s]\n",__FUNCTION__,__LINE__,reInstall?"reinstall":"not re");

    openPkgDB();

    // V2.0:only upgrade use reInstall
    if (reInstall) {
        for (int i =2; i< argc; i++) {
            memset(pkgName,0,sizeof(pkgName));
            isInstalled = false;
            snprintf(pkgName,sizeof(pkgName),"%s",argv[i]);
            isInstalled = pkgIsInstalled(pkgName,false,true);

            printf("\ntrace:%s,%d.\n",__FUNCTION__,__LINE__);

            doInstall(pkgName,true);

         }
        realInstall(report_progress,arg_data);
        goto cleanup;

    }

    for (int i =2; i< argc; i++) {
        snprintf(pkgName,sizeof(pkgName),"%s",argv[i]);
        isInstalled = pkgIsInstalled(pkgName,false);
        if (!isInstalled) {
            doInstall(pkgName,false);
            //realInstall();
            needInst = true;
        }
     }

    if (needInst) {
        if (toInstPkgList.size() <1) {
            if (report_progress) {
                double progress = 0.0;printf("\ntrace:%s,%d.\n",__FUNCTION__,__LINE__);
                report_progress(&progress, arg_data,pkgName,STATUS_INSTALL_ERROR);
            }
            goto cleanup;
        }
        realInstall(report_progress,arg_data);
    } else {
        if (report_progress) {
            double progress = 0.0;printf("\ntrace:%s,%d.\n",__FUNCTION__,__LINE__);
            report_progress(&progress, arg_data,pkgName,STATUS_INSTALLED);
        }
    }

cleanup:

    if (g_osdb) {
        close_rpmdb(g_osdb);
        g_osdb = NULL;
    }
    if (g_appdb) {
        close_rpmdb(g_appdb);
        g_appdb = NULL;
    }

    closePkgDB();
    clearInstallList();
    printf("\ntrace:%s,%d.\n",__FUNCTION__,__LINE__);
}


void remove(int argc, char *argv[],
            void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
            void *arg_data)
{
    char pkgName[512]="";
    bool isInstalled = false;
    bool needRemove = false;
    bool nodeps = false;

    if (argc < 3) {
        usage();
        return;
    }

    openPkgDB();

    for (int i =2; i< argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            nodeps = true;
            continue;
        }
        snprintf(pkgName,sizeof(pkgName),"%s",argv[i]);
        isInstalled = pkgIsInstalled(pkgName,true);
        if (isInstalled) {
            doRemove(pkgName);
            needRemove = true;
        }
    }

    if (needRemove) {
        if (toRemovePkgList.size() <1) {
            if (report_progress) {
                double progress = 0.0;
                report_progress(&progress, arg_data,pkgName,STATUS_REMOVE_ERROR);
            }
            goto cleanup;
        }

        realRemove(report_progress,arg_data,nodeps);
    } else {
        if (report_progress) {
            double progress = 0.0;
            report_progress(&progress, arg_data,pkgName,STATUS_REMOVE_ERROR);
        }
    }

cleanup:

    if (g_osdb) {
        close_rpmdb(g_osdb);
        g_osdb = NULL;
    }
    if (g_appdb) {
        close_rpmdb(g_appdb);
        g_appdb = NULL;
    }

    closePkgDB();

    clearInstallList();
}

bool chkedDep(const char *pkgName)
{
    if(pkgName == NULL || pkgName[0]==0) {
        return true;
    }

    bool found = false;
    string str(pkgName);

    list<string>::iterator it = find( chkedDepList.begin(), chkedDepList.end(), str );
    if (it != chkedDepList.end() ) {
        found = true;
    } else {
        found = false;
    }

    if(!found) {
        chkedDepList.push_back(str);
        found = pkgIsInstalled(pkgName,false);
    }

    return found;
}

/*
* @pkgName NULL-- check all;
* no.1 select name as pkgname from appdb;
* no.2 for every pkg name,select Requires as pkgReq from appdb where name = pkgname;
* no.2.1 check pkgReq is in appdb or not, if not exist,print pkgReq and pkgName.
*/
bool chkDep(const char *pkgName,
            void (*result_handle)(void *arg_data, char *pkgName),
            void *arg_data)
{
    int ret = 0;
    char *sql = NULL;
    char buffer[512]="";
    bool isOK = true;

    sqlite3_stmt *stmt;
    const char *tail;
    if (pkgName == NULL || pkgName[0]==0) {
        sql = sqlite3_mprintf("SELECT Name ,Version,Release,Arch,Requires FROM Package ");
    } else {
        sql = sqlite3_mprintf(
                    "SELECT Name ,Version,Release,Arch,Requires FROM Package WHERE Name = %Q",
                    pkgName);
    }
    ret = sqlite3_prepare(g_db, sql, -1, &stmt, &tail);

    if (ret != SQLITE_OK) {
        printf("\nERROR:%s,%d,sqlite db prepare error.ret[%d],err[%s]\n",
               __FUNCTION__,__LINE__,ret,tail);
        if (result_handle) {
            snprintf(buffer,sizeof(buffer),"sqlite db prepare error!");
            result_handle(arg_data,buffer);
        }
        return false;
    }

    ret = sqlite3_step(stmt);

    sqlite3_free(sql);
    bool first = true;
    char pkgStr[256]="";

    if ( ret == SQLITE_ROW) {
        do {

            // pkg is not installed, continue.
            if (!pkgIsInstalled((char *)sqlite3_column_text(stmt, 0),true)) {
                continue;
            }

            first = true;
            // python-crypto-2.6.1-9.x86_64
            snprintf(pkgStr,sizeof(pkgStr),"%s-%s-%s.%s",
                     (char *)sqlite3_column_text(stmt, 0),
                     (char *)sqlite3_column_text(stmt, 1),
                     (char *)sqlite3_column_text(stmt, 2),
                     (char *)sqlite3_column_text(stmt, 3));

            string requires((char *)sqlite3_column_text(stmt, 4));
            string pattern =",";

            // result: store requires pkgs;
            vector< string> result=split(requires,pattern);
            for( int i = 0; i < result.size(); i++) {

                if (chkedDep(result[i].c_str())) {
                    continue;
                } else {
                    if (first) {
                        first = false;
                        //cout << endl;
                        snprintf(buffer,sizeof(buffer),"Unsatisfied dependencies for %s:\n",
                                 pkgStr);
                        cout << buffer;
                    }

                    snprintf(buffer,sizeof(buffer),"\t %s is needed by %s \n",
                             result[i].c_str(),pkgStr);
                    if (result_handle) {
                        result_handle(arg_data,buffer);
                    }
                    isOK = false;

                    cout << buffer;
                }
            }

        } while(sqlite3_step(stmt) == SQLITE_ROW);
    }

    sqlite3_finalize( stmt );

    return isOK;
}

void check(int argc, char *argv[],
           void (*result_handle)(void *arg_data, char *pkgName),
           void *arg_data)
{
    cout << "Check..." << endl;
    bool allIsOk = true;
    char buffer[32]="";
    openPkgDB();

    if (argc == 3) {
        allIsOk = chkDep(argv[2],result_handle,arg_data);
    } else {
#if 0
        chkDep(NULL,result_handle,arg_data);
#else
        list<string> instList;
        getAllInstalledPkgs(instList,g_db,false);
        instList.sort();

        list<string>::iterator it;
        for(it = instList.begin(); it != instList.end();it++) {
            //cout << *it << endl;
            if (allIsOk)
                allIsOk = chkDep((*it).c_str(),result_handle,arg_data);
            else
                chkDep((*it).c_str(),result_handle,arg_data);
        }
#endif
    }

    if (allIsOk) {
        if (result_handle) {
            snprintf(buffer,sizeof(buffer),"All seem ok.");
            result_handle(arg_data,buffer);
        }
    }

    if (g_osdb) {
        close_rpmdb(g_osdb);
        g_osdb = NULL;
    }
    if (g_appdb) {
        close_rpmdb(g_appdb);
        g_appdb = NULL;
    }

    closePkgDB();

    clearInstallList();

    cout << "Check end." << endl;
}

bool getNewerPkgVer(const char *pkgName,char nvrNewStr[256],char uri[512])
{
    int ret = 0;
    char *sql = NULL;

    sqlite3_stmt *stmt;
    const char *tail;
    if (pkgName == NULL || pkgName[0]==0) {
        return false;
    }
    sql = sqlite3_mprintf(
                "SELECT Name ,Version,Release,Arch,IGroup,Uri FROM Package WHERE Name = %Q",
                pkgName);

    ret = sqlite3_prepare(g_db, sql, -1, &stmt, &tail);
    if (ret != SQLITE_OK) {
        printf("\nERROR:%s,%d,sqlite db prepare error.ret[%d],err[%s]\n",
               __FUNCTION__,__LINE__,ret,tail);
        return false;
    }

    ret = sqlite3_step(stmt);
    sqlite3_free(sql);

    if ( ret == SQLITE_ROW) {
        do {
            #if 0
            printf("%d----\n0[%s]\n1[%s]\n2[%s]- \n",__LINE__,
                (char *)sqlite3_column_text(stmt, 0),
                (char *)sqlite3_column_text(stmt, 1),
                (char *)sqlite3_column_text(stmt, 2));
            #endif

            // python-crypto-2.6.1-9
            snprintf(nvrNewStr,255,"%s-%s-%s",
                     (char *)sqlite3_column_text(stmt, 0),
                     (char *)sqlite3_column_text(stmt, 1),
                     (char *)sqlite3_column_text(stmt, 2));

            snprintf(uri,500,"%s/RPMS.%s/%s-%s-%s.%s.rpm",
                     (char *)sqlite3_column_text(stmt, 5),
                     (char *)sqlite3_column_text(stmt, 4),
                     pkgName,
                     (char *)sqlite3_column_text(stmt, 1),
                     (char *)sqlite3_column_text(stmt, 2),
                     (char *)sqlite3_column_text(stmt, 3));

            break;

        } while(sqlite3_step(stmt) == SQLITE_ROW);
    }

    sqlite3_finalize( stmt );

    return true;
}

/*
* insert into toUpPkgList
*/
void realUpgrade(const char *pkgName,bool onlyGetUpgPkg,string &upgPkgs)
{
    if (pkgName == NULL || pkgName[0] == 0)
        return ;

    char nvrStr[256]="";
    char nvrNewStr[256]="";
    char uri[512]="";

    // find current version.
    if (!getInstalledPkgVer(pkgName,nvrStr) || nvrStr[0] == 0 ) {
        cout << pkgName << " not installed."<< endl;
        return;
    }

    // to find newer version
    if (!getNewerPkgVer(pkgName,nvrNewStr,uri) || nvrNewStr[0] == 0 ) {
#ifdef DEBUG
        cout <<  pkgName << " not in repository." << endl;
#endif
        return;
    }

    if (strcmp(nvrNewStr,nvrStr) > 0) {
        cout << nvrStr << " => " << nvrNewStr <<endl;

        t_PKGS pkg;
        snprintf(pkg.pkgName,sizeof(pkg.pkgName),"%s",pkgName);
        snprintf(pkg.uri,sizeof(pkg.uri),"%s",uri);

        bool find = false;
        list<t_PKGS>::iterator i;

        for(i = toUpPkgList.begin(); i != toUpPkgList.end();++i) {
            if(strcmp((*i).pkgName,pkgName) == 0) {
                find = true;
                break;
            }
        }

        if(!find) {
            toUpPkgList.push_back(pkg);
            if (onlyGetUpgPkg) { // for qjade:to get all upd pkgs[name,old ver,new ver|]
                upgPkgs += pkgName;
                upgPkgs += ",";
                upgPkgs += nvrStr;
                upgPkgs += ",";
                upgPkgs += nvrNewStr;
                upgPkgs += "|";
            }
        }

    }

    return;
}

/*
* use toUpPkgList to do upgrade
*/
static bool doUpgrade(void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
         void *arg_data)
{
    list<t_PKGS>::iterator i;

    int argc = toUpPkgList.size() + 2; // arg[0] arg[1] for install()
    char **argv = NULL;
    int j = 0;
    bool done = false;
    if (argc <= 2) {
        return true;
    }

    argv =  (char **)malloc(sizeof(char *) * argc);
    if (argv == NULL) {
        cout<<"ERROR: out of memmory."<< endl;
        return false;
    }

    memset(argv, 0, sizeof(char *) * argc);

    for (j=0; j < argc; j++) {
        argv[j] = (char *)malloc(sizeof(char) * URI_PATH_MAX);
        if (argv[j] == NULL) {
            cout<<"ERROR: out of memmory."<< endl;
            goto cleanup;
        }
        memset(argv[j], 0, sizeof(char) * URI_PATH_MAX);
    }

    snprintf(argv[0], URI_PATH_MAX - 10, __func__);
    snprintf(argv[1], URI_PATH_MAX - 10, __func__);

    for(i = toUpPkgList.begin(),j=2; i != toUpPkgList.end();++i,j++) {
        snprintf(argv[j], URI_PATH_MAX-1,"%s", (*i).pkgName);
        done = true;
    }

    if (done) {
        /* here:
         * argv: pkgs witch need upgrade
         * just re install these pkgs: use re-Install(true)
        */
        install(argc,argv,report_progress,arg_data,true);
    }

cleanup:
    if (argv) {
        for (j = 0; j < argc; j++) {
            if (argv[j]) {
                free(argv[j]);
                argv[j] = NULL;
            }
        }
        free(argv);
        argv = NULL;
    }

    return true;

}

void upgrade(int argc, char *argv[],
             void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
             void *arg_data)
{
    openPkgDB();

    cout << "Upgrade..." << endl;
    string uptPkg ="";

    if (argc == 3 && argv[2] != NULL && argv[2][0] != 0 && strcmp(argv[2],"0") !=0 ) {
        realUpgrade(argv[2],false,uptPkg);
        if (toUpPkgList.size() <1) {
            if (report_progress) {
                double progress = 0.0;
                report_progress(&progress, arg_data,argv[2],STATUS_UPGRADE_ERROR);
            }
            goto cleanup;
        }

        doUpgrade(report_progress,arg_data);
    } else {
        list<string> instList;
        getAllInstalledPkgs(instList,g_db,false);
        instList.sort();

        list<string>::iterator it;
        for(it = instList.begin(); it != instList.end();it++) {
            realUpgrade((*it).c_str(),false,uptPkg);
        }

        // no pkg need upgrade
        if (toUpPkgList.size() <1) {
            if (report_progress) {
                double progress = 0.0;
                report_progress(&progress, arg_data,"",STATUS_UPGRADE_ERROR);
            }
            goto cleanup;
        }

        doUpgrade(report_progress,arg_data);
        //realInstall(report_progress,arg_data);

    }
cleanup:
    if (g_osdb) {
        close_rpmdb(g_osdb);
        g_osdb = NULL;
    }
    if (g_appdb) {
        close_rpmdb(g_appdb);
        g_appdb = NULL;
    }

    closePkgDB();

    clearInstallList();

    cout << "Upgrade end." << endl;
}

/*
* search results will be stored in the list:
*/
list<t_PKGS> searchResList;

static void clearSearchList()
{
    searchResList.clear();
}


list<string> instList;
static time_t g_last_tm = 0;
void listInstalled(int argc, char *argv[],bool needSignal,
                          void (*result_handle)(void *arg_data, char *pkgName),
                          void *arg_data)
{
    char pkgName[256]="";
    openPkgDB();
#if 0
    list<string> instList;
    getAllInstalledPkgs(instList,g_db,false);
#else
    // can not call listInstalled() frequently!
    if (abs(g_last_tm - time(NULL) ) < 1) {
        //return;
    } else {

        cout << g_last_tm <<" vs " << time(NULL)  << endl;

        g_last_tm = time(NULL);
        instList.clear();
        getAllInstalledPkgs(instList,g_db,false);
    }
#endif

    instList.sort();

    if (argc == 3 && argv[2]!= NULL && argv[2][0]!=0) {
        snprintf(pkgName,sizeof(pkgName),"%s",argv[2]);
    }

    cout << "The following packages are installed:" << endl;

    list<string>::iterator it;
    for(it = instList.begin(); it != instList.end();it++) {
        //cout << *it << endl;
        if (needSignal) {
            if(result_handle) {
                #if 1
                char retStr[256]="";
                memset(retStr,0,sizeof(retStr));
                snprintf(retStr,sizeof(retStr),"1|%s",(*it).c_str());
                result_handle(arg_data,retStr); // installed pkg:[1|pkgName]
                #else
                result_handle(arg_data,(*it).c_str());
                #endif
            }
            continue;
        }

        if (pkgName[0] != 0 && strcasestr((*it).c_str(),pkgName) ) {
            t_PKGS pkg;
            snprintf(pkg.pkgName,sizeof(pkg.pkgName),"%s",(*it).c_str());
            snprintf(pkg.uri,sizeof(pkg.uri),"%d",SearchStatus::INSTALLED);
            searchResList.push_back(pkg);
        } else if (pkgName[0] == 0) { // only for list-all:搜索所有包
            t_PKGS pkg;
            snprintf(pkg.pkgName,sizeof(pkg.pkgName),"%s",(*it).c_str());
            snprintf(pkg.uri,sizeof(pkg.uri),"%d",SearchStatus::INSTALLED);
            searchResList.push_back(pkg);
        }

    }

    closePkgDB();

    cout << endl;
}
/*
* searchAll is true:only for list-all:搜索所有包
*/
void listUnInstalled(int argc, char *argv[],
            bool needSignal,
            void (*result_handle)(void *arg_data, char *pkgName),
            void *arg_data,
            bool searchAll)
{
    int ret = 0;
    char *sql = NULL;
    char pkgStr[256]="";
    char pkgName[256]="";
    list<string> unInstList;

    if (argc == 3 && argv[2]!= NULL && argv[2][0]!=0) {
        snprintf(pkgName,sizeof(pkgName),"%s",argv[2]);
    }

    openPkgDB();
    sqlite3_stmt *stmt;
    const char *tail;
    sql = sqlite3_mprintf("SELECT Name ,Version,Release,Arch,Size,Datetime,status FROM Package WHERE Status = 0 %s ",
                          searchAll ? " or Status = 1" : " ");

    ret = sqlite3_prepare(g_db, sql, -1, &stmt, &tail);
    if (ret != SQLITE_OK) {
        printf("\nERROR:%s,%d,sqlite db prepare error.ret[%d],err[%s]\n",
               __FUNCTION__,__LINE__,ret,tail);
        sqlite3_free(sql);
        sqlite3_finalize( stmt );
        closePkgDB();
        return;
    }

    ret = sqlite3_step(stmt);
    sqlite3_free(sql);
    if ( ret == SQLITE_ROW) {
        do {
            if (needSignal) {
                if(result_handle) {
                    #if 1
                    char retStr[256]="";
                    memset(retStr,0,sizeof(retStr));
                    snprintf(retStr,sizeof(retStr),"0|%s",(char *)sqlite3_column_text(stmt, 0));
                    result_handle(arg_data,retStr); // uninstalled pkg:[0|pkgName]
                    #else
                    result_handle(arg_data,(char *)sqlite3_column_text(stmt, 0));
                    #endif
                }
                continue;
            }

            if (1) {
#if 1
                snprintf(pkgStr,sizeof(pkgStr),"%s",
                         (char *)sqlite3_column_text(stmt, 0));
#else
                snprintf(pkgStr,sizeof(pkgStr),"%s-%s-%s.%s",
                         (char *)sqlite3_column_text(stmt, 0),
                         (char *)sqlite3_column_text(stmt, 1),
                         (char *)sqlite3_column_text(stmt, 2),
                         (char *)sqlite3_column_text(stmt, 3));
#endif
                string str(pkgStr);
                unInstList.push_back(str);

                if (searchAll) { // only for list-all:搜索所有包
                    t_PKGS pkg;
                    snprintf(pkg.pkgName,sizeof(pkg.pkgName),"%s",(char *)sqlite3_column_text(stmt, 0));

                    if ((char *)sqlite3_column_text(stmt, 6)!= NULL) {
                        snprintf(pkg.uri,sizeof(pkg.uri),"%d",
                                 atoi((char *)sqlite3_column_text(stmt, 6)) == 0 ?
                                     SearchStatus::UNINSTALLED :
                                     SearchStatus::INSTALLED);
                    } else
                        snprintf(pkg.uri,sizeof(pkg.uri),"%d",SearchStatus::UNINSTALLED);
                    snprintf(pkg.size,sizeof(pkg.size),"%s",(char *)sqlite3_column_text(stmt, 4));
                    snprintf(pkg.datetime,sizeof(pkg.datetime),"%s",(char *)sqlite3_column_text(stmt, 5));
                    snprintf(pkg.version,sizeof(pkg.version),"%s-%s",
                             (char *)sqlite3_column_text(stmt, 1),
                             (char *)sqlite3_column_text(stmt, 2));

                    searchResList.push_back(pkg);
                }

            } else {
                continue;
            }

        } while(sqlite3_step(stmt) == SQLITE_ROW);
    }

    sqlite3_finalize( stmt );

    closePkgDB();

    unInstList.sort();

    cout << "The following packages are uninstalled:" << endl;
    list<string>::iterator it;
    for(it = unInstList.begin(); it != unInstList.end();it++) {
        //cout << *it << endl;

        if (pkgName[0] != 0 && strcasestr((*it).c_str(),pkgName) ) {
            t_PKGS pkg;
            snprintf(pkg.pkgName,sizeof(pkg.pkgName),"%s",(*it).c_str());
            snprintf(pkg.uri,sizeof(pkg.uri),"%d",SearchStatus::UNINSTALLED);
            searchResList.push_back(pkg);
        }  else if (pkgName[0] == 0) { // only for list-all:搜索所有包
            break;// done.
#if 0
            t_PKGS pkg;
            snprintf(pkg.pkgName,sizeof(pkg.pkgName),"%s",(*it).c_str());
            snprintf(pkg.uri,sizeof(pkg.uri),"%d",SearchStatus::UNINSTALLED);
            snprintf(pkg.size,sizeof(pkg.size),"%d",);

            searchResList.push_back(pkg);
#endif
        }
    }
    cout << endl;
}

void search(int argc, char *argv[],
             void (*result_handle)(void *arg_data_, char *pkgName, gint64 *arg_status),
             void *arg_data,bool getAllPkg)
{
    long int status = -1;

    if (argc == 3 && argv[2] != NULL && argv[2][0] != 0) {

        //searchResList.sort();
        if (getAllPkg) {
            char *argv2[3];
            argv2[0]=(char *)"";
            argv2[1]=(char *)"";
            argv2[2]=(char *)"";

            listUnInstalled(argc,argv2,false,NULL,NULL,true);

            list<t_PKGS>::iterator i;
            string allPkg;
            for(i = searchResList.begin(); i != searchResList.end();i++) {
                if ((*i).uri == NULL || (*i).uri[0] == 0)
                    continue;
                status = atol((*i).uri);
                // status,name,size,datetime,version|status,...
                allPkg += (*i).uri;
                allPkg += ",";
                allPkg += (*i).pkgName;
                allPkg += ",";
                allPkg += (*i).size;
                allPkg += ",";
                allPkg += (*i).datetime;
                allPkg += ",";
                allPkg += (*i).version;
                allPkg += "|";
            }

            //string allPkg2 = allPkg + allPkg + allPkg +allPkg +allPkg; // seems ok

            if(result_handle)
                result_handle(arg_data,allPkg.c_str(),&status);

            goto cleanup;
        }

        listInstalled(argc,argv,false,NULL,NULL);
        listUnInstalled(argc,argv,false,NULL,NULL,false);

        if (searchResList.size() <1) {
            if(result_handle)
                result_handle(arg_data,argv[2],&status);
            goto cleanup;
        }

        list<t_PKGS>::iterator it;
        for(it = searchResList.begin(); it != searchResList.end();it++) {
            if ((*it).uri == NULL || (*it).uri[0] == 0)
                continue;
            status = atol((*it).uri);

            if(result_handle)
                result_handle(arg_data,(*it).pkgName,&status);

        }

    } else {

    }
cleanup:
    clearSearchList();
    cout << "Search end: " <<argv[2]  << endl;
}

void listUpdate(int argc, char *argv[],
             void (*result_handle)(void *arg_data, char *pkgName),
             void *arg_data)
{
    printf("\ntrace:%s,%d\n",__FUNCTION__,__LINE__);
    openPkgDB();

    string allUpgPkgs="";
    list<string> instList;
    getAllInstalledPkgs(instList,g_db,false);
    instList.sort();

    list<string>::iterator it;
    for(it = instList.begin(); it != instList.end();it++) {
        realUpgrade((*it).c_str(),true,allUpgPkgs);
    }

    if(result_handle)
        result_handle(arg_data,allUpgPkgs.c_str());

    instList.clear();

    if (g_osdb) {
        close_rpmdb(g_osdb);
        g_osdb = NULL;
    }
    if (g_appdb) {
        close_rpmdb(g_appdb);
        g_appdb = NULL;
    }

    closePkgDB();

    clearInstallList();


    return;
}

// TODO: SELECT Provides FROM Package WHERE Name = pkgName
// 找到Provides中application(wine-qq.desktop)，将wine-qq.desktop返回
/*
* SELECT Provides FROM Package WHERE Name = 'VirtualBox'
* [VirtualBox,VirtualBox(x86-64),application(),application(virtualbox.desktop),
* mimehandler(application/x-virtualbox-ova)]
*/
char *getDesktopName(char *pkgName,char desktopName[512])
{
    int ret = 0;
    char *sql = NULL;
    sqlite3_stmt *stmt=NULL;
    const char *tail=NULL;
    char *pApp = (char *)"application(";
    int   iApp = strlen(pApp);
    if (pkgName == NULL || pkgName[0]==0) {
        return NULL;
    }

    openPkgDB();

    sql = sqlite3_mprintf(
                "SELECT Provides FROM Package WHERE Name = %Q",
                pkgName);

    ret = sqlite3_prepare(g_db, sql, -1, &stmt, &tail);
    if (ret != SQLITE_OK) {
        printf("\nERROR:%s,%d,sqlite db prepare error.ret[%d],err[%s]\n",
               __FUNCTION__,__LINE__,ret,tail);
        closePkgDB();
        return NULL;
    }

    ret = sqlite3_step(stmt);
    sqlite3_free(sql);

    if ( ret == SQLITE_ROW) {
        do {

            string requires((char *)sqlite3_column_text(stmt, 0));
            string pattern =",";

            vector< string> result=split(requires,pattern);
            for( int i = 0; i < result.size(); i++) {

                if (strncmp(result[i].c_str(),pApp,strlen(pApp)) != 0) {
                    continue;
                } else {
                    char buffer[1024]="";
                    snprintf(buffer,sizeof(buffer),"%s",result[i].c_str());
                    char *p = strstr(buffer,".desktop");
                    if ( p != NULL) {
                        *(p+ strlen(".desktop") ) = 0;
                        int dskLen = strlen(buffer+iApp);
                        if (dskLen >= 512) {
                            dskLen = 500;
                        }
                        strncpy(desktopName,buffer+iApp,dskLen);
                        desktopName[dskLen] = 0;
                        printf("\n%s,%d,find1[%s]vs [%s]\n",
                               __FUNCTION__,__LINE__,buffer+iApp,desktopName);
                        break;

                    }
                }
            }

            break;

        } while(sqlite3_step(stmt) == SQLITE_ROW);
    }

    sqlite3_finalize( stmt );
    closePkgDB();

    return desktopName;

}

static void usage() 
{
    std::cout << "Usage: isoftapp [options] command\n"
                 "       isoftapp [options] install|remove pkg1 [pkg2 ...]\n" << std::endl;

    std::cout << "isoftapp is a simple command line interface for downloading and\n"
                 "installing packages. The most frequently used commands are update\n"
                 "and install.\n" << std::endl;

    std::cout << "Commands:\n"
                 "   update - Retrieve new lists of packages\n"
                 "   upgrade - Perform an upgrade\n"
                 "   install - Install new packages (pkg is libc6 not libc6.rpm)\n"
                 "   remove - Remove packages\n"
                 "   check - Verify that there are no broken dependencies\n"
                 "   list-installed - list \"All installed package\" in \"isoftapp RPM DB\"\n"
                 "   list-uninstalled - list \"All uninstalled packages\" in \"Repos\"\n" << std::endl;

    std::cout << "Options:\n"
                 "   -f\tforce to remove packages nodeps" << std::endl;
}

bool getRpmPathMode()
{
    int ret = -1;
    int fd = open(ISOFTAPP_CACHE_RPMPATHMODE_FILE,O_RDONLY);
    if (fd < 1) {
        g_rpmPathMode.mode = -1;
        g_rpmPathMode.path[0] = 0;
        return false;
    }

    memset(&g_rpmPathMode,0,sizeof(t_RPMPATHMODE));

    ret = read(fd,&g_rpmPathMode,sizeof(t_RPMPATHMODE));
    if (ret < sizeof(t_RPMPATHMODE) ) {
        close(fd);
        g_rpmPathMode.mode = -1;
        g_rpmPathMode.path[0] = 0;
        return false;
    }

    close(fd);

    return true;
}

#include "daemon.h"
int main(int argc, char *argv[]) 
{
#if 1


    getRpmPathMode();

    std::string base_dir = ISOFTAPP_CACHE_DIR + std::string("/rpms");
    if (g_rpmPathMode.path[0] != 0) {
        base_dir = g_rpmPathMode.path;
        printf("test:[%s]\n",base_dir.c_str());
    }

    if (g_mkdir_with_parents(base_dir.c_str(), 0755) == -1) {
        printf("ERROR: failed to create base directory\n");
        return -1;
    }

    isoftAppDaemon(argc,argv);
/*
    //install(argc, argv,NULL,NULL,false);
    char *arg[3];
    int i =0;
    for(i =0;i<1101;i++) {
        arg[0]=(char *)"";
        arg[1]="libtommath";
        system("rpm -e --isoftapp libtommath");
        printf("\n%d--will install\n",i);
        arg[0]=(char *)"";
        arg[1]=(char *)"/var/cache/isoftapp/rpms/libtommath-0.42.0-7.x86_64.rpm";
        //arg[1]=(char *)"/var/cache/isoftapp/rpms/grilo-plugins-0.2.16-2.x86_64.rpm";
    #if 1
        rpmInstall(2,arg,NULL,NULL);
        arg[1]="libtommath";
        rpmCheck(2,arg);
        for(int j = 10;j < 3000;j++) {
            close(j);
        }
    #else
        system("rpm -ivh --isoftapp /var/cache/isoftapp/rpms/libtommath-0.42.0-7.x86_64.rpm");
    #endif
        //arg[1]=(char *)"libtommath";
        //arg[2]=(char *)"libtommath";
        //install(3, arg,NULL,NULL,false);
    }
    return 0;

*/
#else
    if (strcmp(argv[1], "update") == 0)
        update();
    else if (strcmp(argv[1], "install") == 0)
        install(argc, argv,NULL,NULL,false);
    else if (strcmp(argv[1], "remove") == 0)
        remove(argc, argv,NULL,NULL);
    else if (strcmp(argv[1], "check") == 0)
        check(argc, argv);
    else if (strcmp(argv[1], "upgrade") == 0)
        upgrade(argc, argv,NULL,NULL);
    else if (strcmp(argv[1], "list-installed") == 0)
        listInstalled(argc, argv,false,NULL,NULL);
    else if (strcmp(argv[1], "list-uninstalled") == 0)
        listUnInstalled(argc, argv,false,NULL,NULL,false);
    else if (strcmp(argv[1], "search") == 0)
        search(argc, argv,NULL,NULL);
    else
        usage();
#endif
    return 0;
}
