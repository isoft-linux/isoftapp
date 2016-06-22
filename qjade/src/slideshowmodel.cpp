/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "slideshowmodel.h"
#include "globaldeclarations.h"

SlideShowModel::SlideShowModel(HttpGet* parent) : HttpGet(parent) { refresh(); }

SlideShowModel::~SlideShowModel() { m_cleanup(); }

void SlideShowModel::refresh() 
{
    printf("\n%s,%d,[%s]\n",__FUNCTION__,__LINE__,qPrintable(SLIDESHOW_URI));
    get(SLIDESHOW_URI);
}

void SlideShowModel::m_cleanup() 
{
    foreach (QObject *obj, m_dataList) {
        if (obj) delete obj; obj = NULL;
    }
    m_dataList.clear();
}

void SlideShowModel::finished(QNetworkReply *reply) 
{
    QByteArray data = reply->readAll();
    QString strData(data);
    QJsonDocument jsondoc = QJsonDocument::fromJson(strData.toUtf8());
    if (strData == "") {
        emit error();
        return;
    }
#if QJADE_DEBUG
    qDebug() << strData << jsondoc.isObject();
#endif
    if (!jsondoc.isObject()) return;
    QJsonObject obj = jsondoc.object();
    QJsonArray arr = obj["slideshow"].toArray();
    m_cleanup();
    int i = 0;
    foreach (const QJsonValue & val, arr) {
        if (i == 5)
            break;
        QJsonObject obj = val.toObject();
        m_dataList.append(
            new SlideShowObject(obj["name"].toString(), 
                obj["title"].toString(), obj["icon"].toString()));
        i++;
    }
    emit slideshowChanged();
}

QList<QObject*> SlideShowModel::slideshow() const { return m_dataList; }
