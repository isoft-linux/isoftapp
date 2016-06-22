/*
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "categorymodel.h"
#include "globaldeclarations.h"
#include <cstdio>
#include "globallist.h"

static bool isQjadePkgListInited = false;

CategoryModel::CategoryModel(HttpGet* parent) 
  : HttpGet(parent) 
{
    get(CATEGORY_URI);

    printf("\n%s,%d,[%s]\n",__FUNCTION__,__LINE__,qPrintable(CATEGORY_URI));

    //-------------------------------------------------------------------------
    // FIXME: Hot Today is Chinglish, please someone else change it more native 
    //        English
    //-------------------------------------------------------------------------
    m_dataList.append(new CategoryObject("all-pkg", tr("All_Pkg"),  //hot-today,Hot Today
        "hot-today.png"));
}

CategoryModel::~CategoryModel() 
{
    //-------------------------------------------------------------------------
    // TODO: Please do not forget to cleanup allocated space
    //-------------------------------------------------------------------------
    foreach (QObject *obj, m_dataList) {
        if (obj) delete obj; obj = NULL;
    }
    m_dataList.clear();
}

void CategoryModel::finished(QNetworkReply *reply) 
{
    QByteArray data = reply->readAll();
    QString strData(data);
    QJsonDocument jsondoc = QJsonDocument::fromJson(strData.toUtf8());
    if (strData == "") {
        emit error();
        return;
    }
#if QJADE_DEBUG
    qDebug() << strData;
#endif

    printf("\n%s,%d,strData[%s]\n",__FUNCTION__,__LINE__,qPrintable(strData));
    //-------------------------------------------------------------------------
    // TODO: To forbit json original string is not object
    //-------------------------------------------------------------------------
    if (!jsondoc.isObject()) return;
    QJsonObject obj = jsondoc.object();
    QJsonArray arr = obj["categories"].toArray();

    bool isExist = false;

    //-------------------------------------------------------------------------
    // TODO: traverse all the QJsonObject from QJsonArray
    //-------------------------------------------------------------------------
    foreach (const QJsonValue & val, arr) {
        QJsonObject obj = val.toObject();
        m_dataList.append(
            new CategoryObject("category-" + obj["name"].toString(), 
                obj["title"].toString(),//+"67",
                obj["icon"].toString()));

        // to init globle list g_qjadePkgList(declared in main.cpp)
        if (!isQjadePkgListInited) {

            // to init g_categoryList
            isExist = false;
            QString name = obj["name"].toString();
            for (int i = 0; i < g_categoryList.size(); ++i) {
                if (g_categoryList[i].name == name) {
                    isExist = true;
                    break;
                }
            }
            if (!isExist) {
                t_CATEGORY_INFO pkg;
                pkg.name = name;
                pkg.title = obj["title"].toString();
                //pkg.catPkgList to insert ...
                g_categoryList.append(pkg);
            }
            printf("\n######%:%s,%d,[%s]name[%s]title[%s]\n",__FUNCTION__,__LINE__,
                   isExist?"exist":"not exist",qPrintable(name),qPrintable(obj["title"].toString()));


            connect(&m_pks, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(getPackagesFinished(QNetworkReply*)));
            m_pks.get(QNetworkRequest(CATEGORY_PACKAGE_URI + obj["name"].toString() + "/" +
                QLocale::system().name().toLower()));

            QString tmp = PACKAGE_URI + obj["name"].toString() + "/" +
                                         QLocale::system().name().toLower();
            printf("\n######%s,%d,to get pkgs name by cat name:[%s]\n",__FUNCTION__,__LINE__,qPrintable(tmp));

        }

    }

    // my pkgs
    m_dataList.append(new CategoryObject("my-pkgs", tr("My_pkgs"),
        "")); // ICON is null.

    isQjadePkgListInited = true;

    emit categoryChanged();
}

QList<QObject*> CategoryModel::categories() const
{
    return m_dataList;
}


void CategoryModel::getPackagesFinished(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();
    QString strData(data);
    QJsonDocument jsondoc = QJsonDocument::fromJson(strData.toUtf8());
#if QJADE_DEBUG
    qDebug() << "DEBUG:" << strData;
#endif
    if (!jsondoc.isObject()) return;
    QJsonObject obj = jsondoc.object();
    QJsonArray arr = obj["packages"].toArray();
    if (arr.size() == 0) {
        emit error();
        return;
    }

    printf("\n######%s,%d,pkg url:[%s],const url[%s]\n",__FUNCTION__,__LINE__,
           qPrintable(reply->url().toString() ),qPrintable(CATEGORY_PACKAGE_URI));
    // to find node by cate name in g_categoryList
    // url[http://appstore.isoft-linux.org/appstore/category-packages/Games/zh_cn]
    char catPkgStr[256]="";
    char urlStr[512]="";
    char catName[128]="";
    snprintf(catPkgStr,sizeof(catPkgStr),"%s",qPrintable(CATEGORY_PACKAGE_URI));


    int  iFind  = -1;
    for (int i = 0; i < g_categoryList.size(); ++i) {
        memset(urlStr,0,sizeof(urlStr));
        snprintf(urlStr,sizeof(urlStr),"%s",qPrintable(reply->url().toString()));
        if (strlen(urlStr) <1)
            continue;
        char *p = strstr(urlStr,catPkgStr);
        if (p != NULL) {

            char *dst = p + strlen(catPkgStr);
            int j = 0;
            while(*dst != '/' && j<127 && *dst != 0 ) {
                catName[j] = *dst;
                dst ++;
                j++;
            }
            catName[j] = 0;
            QString dstCatName = catName;

            if (g_categoryList[i].name == dstCatName) {

                printf("\n######%s,%d,cat in list:[%s],dst[%s]\n",__FUNCTION__,__LINE__,
                       qPrintable(g_categoryList[i].name),qPrintable(dstCatName));

                iFind = i;
                g_categoryList.at(iFind).catPkgList.clear();
                break;
            }


        }
    }

    foreach (const QJsonValue & val, arr) {
        QJsonObject obj = val.toObject();
        QString pkname = obj["name"].toString();
        //---------------------------------------------------------------------
        // TODO: 根据package-name通过HTTP GET从web后端获得package信息
        //---------------------------------------------------------------------
        connect(&m_pk, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(getPackageFinished(QNetworkReply*)));
        m_pk.get(QNetworkRequest(PACKAGE_URI + pkname + "/" +
            QLocale::system().name().toLower()));

        QString tmp = PACKAGE_URI + pkname + "/" +
                                     QLocale::system().name().toLower();

        if (iFind >= 0) {
            g_categoryList.at(iFind).catPkgList.append(pkname);

        }

    }
    //disconnect(&m_pks, SIGNAL(finished(QNetworkReply*)),
    //    this, SLOT(getPackagesFinished(QNetworkReply*)));
}

void CategoryModel::getPackageFinished(QNetworkReply *reply)
{
    int i = 0;
    QByteArray data = reply->readAll();
    QString strData(data);
    QJsonDocument jsondoc = QJsonDocument::fromJson(strData.toUtf8());
#if QJADE_DEBUG
    qDebug() << "DEBUG: " << strData;
#endif
    if (!jsondoc.isObject()) return;
    QJsonObject obj = jsondoc.object();
    QString pkgName = obj["name"].toString();

    t_QJADE_PKGINFO pkg;
    pkg.name = pkgName;
    pkg.description = obj["description"].toString();
    pkg.title = obj["title"].toString();
    pkg.url = obj["url"].toString();
    pkg.icon = obj["icon"].toString();
    QString cateName ="";
    bool isFind = false;
    for (i = 0; i < g_categoryList.size(); ++i) {
        for (int j = 0; j < g_categoryList[i].catPkgList.size(); ++j) {

            if (g_categoryList[i].catPkgList[j] == pkg.title) {
                cateName = g_categoryList[i].title;
                isFind = true;
                break;
            }
        }
        if (isFind) {
            break;
        }
    }
    pkg.category = isFind? cateName : "NULL";

    if (pkg.description.length() > 25) {
        pkg.description.resize(25);
        pkg.description += "...";
    }

    //printf("\n######%s,%d,all pkgs: name [%s]\n",__FUNCTION__,__LINE__,qPrintable(pkg.name));

    /*
     * 点击左侧的每个类别时，同步数据到appstore服务端数据列表g_qjadePkgList.
    */
    for (i = 0; i < g_qjadePkgList.size(); ++i) {
        if (g_qjadePkgList[i].name == pkgName) {
            g_qjadePkgList.removeAt(i);
            break;
        }
    }

    g_qjadePkgList.append(pkg);

}
