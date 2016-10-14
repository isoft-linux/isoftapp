/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <unistd.h>

#include "iconmodel.h"
#include "globaldeclarations.h"
#include "globallist.h"
#define QJADE_DEBUG 1
extern bool g_offline;
static bool iconFristRun = false;
QList<QObject*> g_dataList;
IconModel::IconModel(HttpGet* parent) 
  : HttpGet(parent) 
{
    if (iconFristRun) {
        useCache();
    } else
    refresh();
}

IconModel::~IconModel() 
{
    m_cleanup();
}
void IconModel::useCacheFinished()
{
    if(g_offline) {
        emit error();
        return;
    }
    m_cleanup();
    foreach (IconObject *obj, g_dataList) {
        if (obj) {
            m_dataList.append(new IconObject(obj->name(), obj->title(),
                obj->icon(), obj->url()));
        }
    }
    emit iconChanged();
}
void IconModel::useCache()
{
    QTimer::singleShot(100, this, SLOT(useCacheFinished()));
}
void IconModel::refresh() 
{
    get(GRIDVIEW_URI);
}

void IconModel::m_cleanup() 
{
    foreach (QObject *obj, m_dataList) {
        if (obj) delete obj; obj = NULL;
    }
    m_dataList.clear();
}

void IconModel::finished(QNetworkReply *reply) 
{
    QByteArray data = reply->readAll();
    QString strData(data);
    QJsonDocument jsondoc = QJsonDocument::fromJson(strData.toUtf8());
    if (strData == "") {
        emit error();
        return;
    }

    if (!jsondoc.isObject()) return;
    QJsonObject obj = jsondoc.object();
    QJsonArray arr = obj["gridview"].toArray();
    m_cleanup();
    foreach (const QJsonValue & val, arr) {
        QJsonObject obj = val.toObject();
        QString pkgName = obj["name"].toString();
        bool isExist = false;
        for (int i = 0; i < AllPkgList.size(); ++i) {
            if(AllPkgList.at(i).pkgName == pkgName) {
                isExist = true;
                break;
            }
        }
        if ( !isExist ) {
            continue;
        }
        m_dataList.append(new IconObject(pkgName, obj["title"].toString(), 
            obj["icon"].toString(), obj["url"].toString()));

        if (!iconFristRun) {
            g_dataList.append(new IconObject(pkgName, obj["title"].toString(),
                obj["icon"].toString(), obj["url"].toString()));
        }
    }
    iconFristRun = true;
    printf("\n######%s,%d,IconModel size:[%d]\n",__FUNCTION__,__LINE__,m_dataList.size());
    emit iconChanged();
}

QList<QObject*> IconModel::icons() const
{
    return m_dataList;
}
