/*                                                                              
 * Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 *               2016 fujiang <fujiang.zhu@i-soft.com.cn>
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
    QList<QString> realList ;
    foreach (const QJsonValue & val, arr) {
        QJsonObject obj = val.toObject();
        QString tmpStr = obj["name"].toString();
        if(tmpList.contains(tmpStr)) {
            continue;
        }
        tmpList.append(tmpStr);

        bool isExist = false;
        for (int j = 0; j < AllPkgList.size(); ++j) {
            if (AllPkgList.at(j).pkgName == obj["name"].toString()) {
                size = AllPkgList.at(j).size;
                isExist = true;
                break;
            }
        }
        if ( !isExist ) {
            continue;
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
        if(realList.contains(obj["name"].toString())) {
            continue;
        }

        bool find = false;
        int k = 0;
        for (k = 0; k < realList.size(); k++) {
            if (realList[k] > obj["name"].toString()) {
                find = true;
                break;
            }
        }
        if (!find ) {
            m_dataList.append(new SearchObject(obj["name"].toString(),
                    description,
                    obj["icon"].toString(),
                    dstSize,title));
            realList.append(obj["name"].toString());
        } else {
            m_dataList.insert(k,new SearchObject(obj["name"].toString(),
                    description,
                    obj["icon"].toString(),
                    dstSize,title));
            realList.insert(k, obj["name"].toString() );
        }
    }
    tmpList.clear();
    realList.clear();
    printf("\n######%s,%d,search result num:[%d]\n",__FUNCTION__,__LINE__,m_dataList.size());
    emit searchResultChanged(m_dataList.size());
}

QList<QObject*> SearchModel::searchResult() const
{
    return m_dataList;
}
