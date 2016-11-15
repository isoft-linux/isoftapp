/*
 * Copyright (C) 2016 fujiang <fujiang.zhu@i-soft.com.cn>
 */
#ifndef QJADE_GLOBALLIST_H
#define QJADE_GLOBALLIST_H
#include "util.h"

typedef struct {
    QString pkgName;
    int  status; //0-no this pkg;1-installed;2-not installed but existing;3-doing

    QString size;
    QString datetime;
    QString version; // version-resease
} t_PKGINFO;

/* 从isoftapp-daemon 获取所有软件包信息 */
extern QList<t_PKGINFO> AllPkgList;

typedef struct {
    QString name;
    QString title;
    QString description;
    QString icon;
    QString url;
    QString version;
    QString category;
    bool exist;
} t_QJADE_PKGINFO;

/* 从服务端搜集所有的安装包信息 */
extern QList<t_QJADE_PKGINFO> g_qjadePkgList;

typedef struct {
    QString name;  // en
    QString title; // zh_cn
    bool exist;
    QList<QString> catPkgList; // 此类型下面，所有的软件包名称的列表
} t_CATEGORY_INFO;
extern QList<t_CATEGORY_INFO> g_categoryList;

#endif

