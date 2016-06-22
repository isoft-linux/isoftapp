/*                                                                              
 * Copyright (C) 2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

#ifndef PACKAGE_INFO_MODEL_H
#define PACKAGE_INFO_MODEL_H

#include <QObject>
#include <QStringList>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonDocument>                                                           
#include <QJsonObject>                                                             
#include <QJsonArray>                                                              
#include <QLocale>

#include "globaldeclarations.h"
#include "globallist.h"

class PackageInfoModel : public QObject 
{
    Q_OBJECT
    
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QStringList snapshots READ snapshots NOTIFY snapshotsChanged)
    Q_PROPERTY(QString size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(QString datetime READ datetime WRITE setDatetime NOTIFY datetimeChanged)
    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY versionChanged)
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)


public:
    PackageInfoModel(QObject *parent = 0) : QObject(parent) {}
    PackageInfoModel(const QString &name, const QString &title, 
        const QString &description, const QString &icon, const QString &url,
        const QString &size,const QString &datetime,const QString &version,const QString &category,
        QObject *parent = 0)
      : QObject(parent) 
    {
        m_name = name; m_title = title; m_description = description; 
        m_icon = icon; m_url = url;
        m_size = size;m_datetime = datetime;m_version = version;m_category = category;
    }

    QString name() const { return m_name; }
    void setName(const QString &name) 
    { 
        if (name != m_name) m_name = name; emit nameChanged();
    }

    QString title() const { return m_title; }
    void setTitle(const QString &title)
    {
        if (title == m_title) return;

        m_title = title;
        connect(&m_pk, SIGNAL(finished(QNetworkReply*)),                           
                this, SLOT(getPackageFinished(QNetworkReply*)));                   
        m_pk.get(QNetworkRequest(PACKAGE_URI + m_title + "/" +                      
                 QLocale::system().name().toLower()));
    }

    QString description() const { return m_description; }
    void setDescription(const QString &description) 
    {
        if (description != m_description) {
            m_description = description; emit descriptionChanged();
        }
    }

    QString icon() const { return m_icon; }
    void setIcon(const QString &icon) 
    {
        if (icon != m_icon) m_icon = icon; emit iconChanged();
    }

    QString url() const { return m_url; }
    void setUrl(const QString &url) 
    {
        if (url != m_url) m_url = url; emit urlChanged();
    }

    QString size() const { return m_size; }
    void setSize(const QString &size)
    {
        if (size != m_size) m_size = size; emit sizeChanged();
    }

    QString datetime() const { return m_datetime; }
    void setDatetime(const QString &datetime)
    {
        if (datetime != m_datetime) m_datetime = datetime; emit datetimeChanged();
    }
    QString version() const { return m_version; }
    void setVersion(const QString &version)
    {
        if (version != m_version) m_version = version; emit versionChanged();
    }
    QString category() const { return m_category; }
    void setCategory(const QString &category)
    {
        if (category != m_category) m_category = category; emit categoryChanged();
    }


    QStringList snapshots() const { return m_snapshots; }

Q_SIGNALS:
    void nameChanged();
    void titleChanged();
    void descriptionChanged();
    void iconChanged();
    void urlChanged();
    void snapshotsChanged();

    void sizeChanged();
    void datetimeChanged();
    void versionChanged();
    void categoryChanged();

private Q_SLOTS:
    void getPackageFinished(QNetworkReply *reply)                
    {                                                                              
        QByteArray data = reply->readAll();                                        
        QString strData(data);                                                     
        QJsonDocument jsondoc = QJsonDocument::fromJson(strData.toUtf8());

        //qDebug() << "DEBUG:" << strData;
        QJsonObject obj = jsondoc.object();
        m_name = obj["name"].toString();
        m_description = obj["description"].toString();
        m_icon = obj["icon"].toString();
        m_url = obj["url"].toString();
        QJsonArray arr = obj["snapshot"].toArray();
        foreach (const QJsonValue & val, arr) {
            m_snapshots.append(val.toString());
        }

        //get size/datetime/version from AllPkgList
        int i =0;
        for (i = 0; i < AllPkgList.size(); ++i) {
            if (AllPkgList.at(i).pkgName  == m_name) {
                m_size = AllPkgList[i].size;
                m_datetime = AllPkgList[i].datetime;
                m_version = AllPkgList[i].version;
                break;
            }
        }

        if (m_size.toInt() > 0) {

            QString dstSize ="";
            getStrSize(m_size,dstSize);

            m_size = dstSize;
        }

        for (i = 0; i < g_qjadePkgList.size(); ++i) {
            if (g_qjadePkgList.at(i).name  == m_name) {
                m_category = g_qjadePkgList[i].category;
                break;
            }
        }

        disconnect(&m_pk, SIGNAL(finished(QNetworkReply*)),                         
                this, SLOT(getPackageFinished(QNetworkReply*)));                   
    
        emit titleChanged();
    }

private:
    QString m_name;
    QString m_title;
    QString m_description;
    QString m_icon;
    QString m_url;
    QStringList m_snapshots;
    QNetworkAccessManager m_pk;                                                    
    QString m_size;
    QString m_datetime;
    QString m_version;
    QString m_category;
};

#endif  // PACKAGE_INFO_MODEL_H
