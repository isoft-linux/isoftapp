/*
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include "categorymodel.h"
#include "globaldeclarations.h"
#include <cstdio>
#include "globallist.h"

//#define USEOLDROUTINE

static bool isQjadePkgListInited = false;
int g_cate_number=0;
int g_cate_act_number=0;
CategoryModel::CategoryModel(HttpGet* parent) 
  : HttpGet(parent) 
{
    get(CATEGORY_URI);

    g_cate_number = 0;
    g_cate_act_number = 0;

    //-------------------------------------------------------------------------
    // FIXME: Hot Today is Chinglish, please someone else change it more native 
    //        English
    //-------------------------------------------------------------------------
#ifdef USEOLDROUTINE
    m_dataList.append(new CategoryObject("all-pkg", tr("All_Pkg"),  //hot-today,Hot Today
        "hot-today.png",0));
#endif
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
    int xx =1;
    g_cate_number =0;
    g_cate_act_number = 0;
    foreach (const QJsonValue & val, arr) {
        QJsonObject obj = val.toObject();
        #ifdef USEOLDROUTINE
        m_dataList.append(
            new CategoryObject("category-" + obj["name"].toString(), 
                obj["title"].toString(),//+"67",
                obj["icon"].toString(),xx++));

        printf("\n######%:%s,%d,name[%s]title[%s]icon[%d]\n",__FUNCTION__,__LINE__,
            qPrintable(obj["name"].toString()),qPrintable(obj["title"].toString()),
                obj["icon"].toString());

        #endif

        if (!isQjadePkgListInited) {
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

            connect(&m_pks, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(getPackagesFinished(QNetworkReply*)));

            m_pks.get(QNetworkRequest(CATEGORY_PACKAGE_URI + obj["name"].toString() + "/" +
                QLocale::system().name().toLower()));
        }

    }

    // my pkgs
    #ifdef USEOLDROUTINE
    m_dataList.append(new CategoryObject("my-pkgs", tr("My_pkgs"),
        "",xx++)); // ICON is null.
    #endif

    //isQjadePkgListInited = true;
    //emit categoryChanged();
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

    if (!jsondoc.isObject()) return;
    QJsonObject obj = jsondoc.object();
    QJsonArray arr = obj["packages"].toArray();
    if (arr.size() == 0) {
        emit error();
        return;
    }

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
                iFind = i;
                g_categoryList.at(iFind).catPkgList.clear();
                break;
            }
        }
    }

    foreach (const QJsonValue & val, arr) {
        // to compare with g_cate_ace_number;
        g_cate_number ++;

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

    if (AllPkgList.size() < 10 ) {
        QTime t;
        t.start();
        while(AllPkgList.size() < 10 && t.elapsed() < 10000) {
            QCoreApplication::processEvents();
        }
    }
    // g_qjadePkgList 保存了服务器和isoft-daemon的交集.
    for (int j = 0; j < AllPkgList.size(); ++j) {
        if (AllPkgList.at(j).pkgName  == pkgName) {
            isFind = true;
            break;
        }
    }
    if (isFind) {
        isFind = false;
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
    g_cate_act_number ++;

    if (g_cate_number != 0 && (g_cate_number == g_cate_act_number) ) {

    #ifndef USEOLDROUTINE
        g_cate_number = 0;

        int pkgNumber = 0;
        for (i = 0; i < g_qjadePkgList.size(); ++i) {
            if (g_qjadePkgList[i].name.isEmpty()) {
                continue;
            }
            QString size ="";
            for (int j = 0; j < AllPkgList.size(); ++j) {
                if (AllPkgList.at(j).pkgName  == g_qjadePkgList[i].name) {
                    size = AllPkgList.at(j).size;
                    break;
                }
            }
            QString dstSize ="";
            getStrSize(size,dstSize);
            if(dstSize.isEmpty()) {
                continue;
            }

            pkgNumber ++;
        }
        m_dataList.append(new CategoryObject("all-pkg", tr("All_Pkg"),  //hot-today,Hot Today
            "hot-today.png",pkgNumber));


        bool isFind = false;
        for (i = 0; i < g_categoryList.size(); ++i) {
            pkgNumber = 0;
            for (int j = 0; j < g_qjadePkgList.size(); ++j) {
                if (g_qjadePkgList[j].category == g_categoryList[i].title) {
                    pkgNumber ++;
                }
            }
            m_dataList.append(
                new CategoryObject("category-" + g_categoryList[i].name,
                    g_categoryList[i].title,
                    "",pkgNumber));
        }

        /*
        m_dataList.append(
            new CategoryObject("category-" + obj["name"].toString(),
                obj["title"].toString(),//+"67",
                obj["icon"].toString(),2));
        */

        pkgNumber = 0;
        for (i = 0; i < AllPkgList.size(); ++i) {
            if(AllPkgList.at(i).datetime == "0") {
                continue;
            }
            for(int j =0;j< g_qjadePkgList.size();j++) {
                if (AllPkgList.at(i).pkgName == g_qjadePkgList.at(j).name) {
                    pkgNumber ++;
                }
            }
        }
        m_dataList.append(new CategoryObject("my-pkgs", tr("My_pkgs"),
            "",pkgNumber)); // ICON is null.
        emit categoryChanged();
    #endif
    }

}
