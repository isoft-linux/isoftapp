/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

#include <QNetworkRequest>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLocale>

#include "packagebycategorymodel.h"
#include "globaldeclarations.h"
#include "util.h"
#include "globallist.h"

PackageByCategoryModel::PackageByCategoryModel(QObject *parent) 
  : QObject(parent), 
    m_category("") 
{
#if QJADE_DEBUG
    qDebug() << "DEBUG:" << __PRETTY_FUNCTION__;
#endif
}

PackageByCategoryModel::~PackageByCategoryModel() 
{
#if QJADE_DEBUG
    qDebug() << "DEBUG:" << __PRETTY_FUNCTION__;
#endif
    // qDeleteAll
    foreach (QObject *obj, m_dataList) {
        if (obj) delete obj; obj = NULL;                                           
    }                                                                              
    m_dataList.clear();
}

void PackageByCategoryModel::m_get() 
{
    m_dataList.clear();
    m_httpGet();
    printf("\n%s,%d,[%s]\n",__FUNCTION__,__LINE__,qPrintable(CATEGORY_PACKAGE_URI));
}

//-----------------------------------------------------------------------------
// TODO: 让QML来set category值
//-----------------------------------------------------------------------------
void PackageByCategoryModel::setCategory(QString category) 
{
    // 全部软件：直接使用cache；
    if (category == "all-pkg") {
        m_dataList.clear();
        for (int i = 0; i < g_qjadePkgList.size(); ++i) {
            if (g_qjadePkgList[i].name.isEmpty()) {
                continue;
            }

            QString size ="";
            int status = 9;// 2 need not install
            for (int j = 0; j < AllPkgList.size(); ++j) {
                if (AllPkgList.at(j).pkgName  == g_qjadePkgList[i].name) {
                    size = AllPkgList.at(j).size;
                    status = AllPkgList.at(j).status;
                    break;
                }
            }

            QString dstSize ="";
            getStrSize(size,dstSize);

            if(dstSize.isEmpty()) {
                continue;
            }

            QString needInstall = QString::number(status, 10);
            m_dataList.append(new PackageObject(g_qjadePkgList[i].name,
                g_qjadePkgList[i].title,
                g_qjadePkgList[i].description,
                g_qjadePkgList[i].icon,
                g_qjadePkgList[i].url,
                dstSize,needInstall));
        }
        printf("\n%s,%d,g_qjadePkgList num[%d] AllPkgList num[%d]\n",__FUNCTION__,__LINE__,
               g_qjadePkgList.size(),AllPkgList.size());
        emit packageChanged();
        return;
    }

    if (category == m_category) return;
    m_category = category; emit categoryChanged(); m_get();
}

//-----------------------------------------------------------------------------
// TODO: 通过HTTP GET从web后端获得
//-----------------------------------------------------------------------------
void PackageByCategoryModel::m_httpGet() 

{
    connect(&m_pks, SIGNAL(finished(QNetworkReply*)),                              
        this, SLOT(getPackagesFinished(QNetworkReply*)));
    m_pks.get(QNetworkRequest(CATEGORY_PACKAGE_URI + m_category.mid(9) + "/" + 
        QLocale::system().name().toLower()));
    QString tmp = CATEGORY_PACKAGE_URI + m_category.mid(9) + "/" +
            QLocale::system().name().toLower();
}

/*
* 点击左侧类别，在右侧显示软件包列表
*/
void PackageByCategoryModel::getPackageFinished(QNetworkReply *reply) 
{
    QByteArray data = reply->readAll();
    QString strData(data);
    QJsonDocument jsondoc = QJsonDocument::fromJson(strData.toUtf8());
#if QJADE_DEBUG
    qDebug() << "DEBUG: " << strData;
#endif
    if (!jsondoc.isObject()) return;
    QJsonObject obj = jsondoc.object();
    QString pkgName = obj["name"].toString();

    m_pks_act_size ++;
    bool isExist = false;
    QString size ="";
    int  status = 9;
    for (int i = 0; i < AllPkgList.size(); ++i) {
        if (AllPkgList.at(i).pkgName  == pkgName) {
            size = AllPkgList.at(i).size;
            status = AllPkgList.at(i).status;
            isExist = true;
            break;
        }
    }

    // pkg in server not exist in isoftapp-daemon.
    if (!isExist) {
        if (m_pks_act_size == m_pks_size) {
            printf("\n%s,%d\n",__FUNCTION__,__LINE__);
            emit packageChanged();
        }
        return;
    }

    QString dstSize ="";
    getStrSize(size,dstSize);

    t_QJADE_PKGINFO pkg;
    pkg.name = pkgName;
    pkg.description = obj["description"].toString();
    pkg.title = obj["title"].toString();
    pkg.url = obj["url"].toString();
    pkg.icon = obj["icon"].toString();
    pkg.category = "";// m_category.mid(9);

    int i = 0;
    for (i = 0; i < g_categoryList.size(); ++i) {
        if (g_categoryList[i].name == m_category.mid(9)) {
            pkg.category = g_categoryList[i].title;
            break;
        }
    }

    if (pkg.category.isEmpty()) {
        pkg.category = m_category.mid(9);
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

    if (pkg.description.length() > 26) {
        pkg.description.resize(26);
        pkg.description += "...";
    }

    QString needInstall = QString::number(status, 10);
    m_dataList.append(new PackageObject(pkgName, obj["title"].toString(),
        pkg.description, obj["icon"].toString(),
        obj["url"].toString(),
        dstSize,needInstall));

    g_qjadePkgList.append(pkg);
    if (m_pks_act_size == m_pks_size) {
        printf("\n%s,%d\n",__FUNCTION__,__LINE__);
        emit packageChanged();
    }
    //printf("\n%s,%d,data num[%d] vs pkg num[%d],name[%s],needinstall[%s]\n",__FUNCTION__,__LINE__,
    //       m_dataList.size(),m_pks_size,qPrintable(pkgName ),qPrintable(needInstall));

}

void PackageByCategoryModel::getPackagesFinished(QNetworkReply *reply) 
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
    m_pks_act_size = 0;
    m_pks_size = arr.size();
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
        //printf("\n%s,%d,to get info by name:[%s],pkg num[%d]\n",__FUNCTION__,__LINE__,qPrintable(tmp),m_pks_size);
    }
    disconnect(&m_pks, SIGNAL(finished(QNetworkReply*)),                           
        this, SLOT(getPackagesFinished(QNetworkReply*)));
}

QString PackageByCategoryModel::category() const { return m_category; }

QList<QObject*> PackageByCategoryModel::packages() const { return m_dataList; }

void PackageByCategoryModel::removeAt(int index) { m_dataList.removeAt(index); }
