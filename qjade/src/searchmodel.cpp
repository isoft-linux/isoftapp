/*                                                                              
 * Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <unistd.h>

#include "searchmodel.h"
#include "globaldeclarations.h"
#include "globallist.h"

SearchModel::SearchModel(HttpGet* parent) 
  : HttpGet(parent) 
{
}

SearchModel::~SearchModel() 
{
    m_cleanup();
}

void SearchModel::search(QString keyword) 
{
#if QJADE_DEBUG
    qDebug() << "DEBUG:" << __PRETTY_FUNCTION__ << SEARCH_URI + keyword;
#endif
    printf("\n%s,%d,[%s]\n",__FUNCTION__,__LINE__,qPrintable(SEARCH_URI + keyword));
    get(SEARCH_URI + keyword);
}

void SearchModel::m_cleanup() 
{
    foreach (QObject *obj, m_dataList) {
        if (obj) delete obj; obj = NULL;
    }
    m_dataList.clear();
}

void SearchModel::finished(QNetworkReply *reply) 
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
    QJsonArray arr = obj["packages"].toArray();
    m_cleanup();
    QString size ="";
    QList<QString> tmpList ;
    foreach (const QJsonValue & val, arr) {
        QJsonObject obj = val.toObject();
        QString tmpStr = obj["name"].toString();
        if(tmpList.contains(tmpStr)) {
            continue;
        }
        tmpList.append(tmpStr);
        for (int j = 0; j < AllPkgList.size(); ++j) {
            if (AllPkgList.at(j).pkgName == obj["name"].toString()) {
                size = AllPkgList.at(j).size;
                break;
            }
        }
        QString dstSize ="";
        getStrSize(size,dstSize);
        QString description = obj["description"].toString();

        if (description.length() > 26) {
            description.resize(26);
            description += "...";
        }

        QString title ="";
        for (int j = 0; j < g_qjadePkgList.size(); ++j) {
            if (g_qjadePkgList.at(j).name == obj["name"].toString()) {
                title = g_qjadePkgList.at(j).title;
                break;
            }
        }
        if (title.isEmpty()) {
            title = obj["name"].toString();
        }


        m_dataList.append(new SearchObject(obj["name"].toString(),
                    description,
                    obj["icon"].toString(),
                    dstSize,title));
    }
    tmpList.clear();
    emit searchResultChanged(m_dataList.size());
}

QList<QObject*> SearchModel::searchResult() const
{
    return m_dataList;
}
