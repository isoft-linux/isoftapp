/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <unistd.h>

#include "iconmodel.h"
#include "globaldeclarations.h"
#define QJADE_DEBUG 1
IconModel::IconModel(HttpGet* parent) 
  : HttpGet(parent) 
{
    refresh();
}

IconModel::~IconModel() 
{
    m_cleanup();
}

void IconModel::refresh() 
{
    printf("\n%s,%d,icon refresh[%s]\n",__FILE__,__LINE__,qPrintable(GRIDVIEW_URI));
    //http://appstore.isoft-linux.org/appstore/sticky/zh_cn
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
    printf("\n%s,%d\n",__FILE__,__LINE__);
    QByteArray data = reply->readAll();
    QString strData(data);
    QJsonDocument jsondoc = QJsonDocument::fromJson(strData.toUtf8());
    if (strData == "") {
        emit error();
        return;
    }
#if QJADE_DEBUG
    printf("\n%s,%d,data[%s]\n",__FILE__,__LINE__,qPrintable(strData));

    printf("\n######%s,%d,IconModel url:[%s]\n",__FUNCTION__,__LINE__,
           qPrintable(reply->url().toString() ));

    // finished,52,data[{"gridview":[{"name":"pidgin","title":"Pidgin",
    //"icon":"http://appstore.isoft-linux.org/sites/default/files/Pidgin.svg_.png"}
#endif
    if (!jsondoc.isObject()) return;
    QJsonObject obj = jsondoc.object();
    QJsonArray arr = obj["gridview"].toArray();
    m_cleanup();
    foreach (const QJsonValue & val, arr) {
        QJsonObject obj = val.toObject();
        QString pkgName = obj["name"].toString();
        m_dataList.append(new IconObject(pkgName, obj["title"].toString(), 
            obj["icon"].toString(), obj["url"].toString()));
    }
    emit iconChanged();
}

QList<QObject*> IconModel::icons() const
{
    printf("\n%s,%d\n",__FILE__,__LINE__);
    return m_dataList;
}
