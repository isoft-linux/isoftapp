/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

#ifndef PACKAGE_BY_CATEGORY_MODEL_H
#define PACKAGE_BY_CATEGORY_MODEL_H

#include <QObject>
#include <QList>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class PackageObject;

class PackageByCategoryModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    Q_PROPERTY(QList<QObject*> packages READ packages NOTIFY packageChanged)

public:
    PackageByCategoryModel(QObject *parent = 0);
    ~PackageByCategoryModel();

    QString category() const;
    void setCategory(QString category);

    QList<QObject*> packages() const;

    Q_INVOKABLE void removeAt(int index);

Q_SIGNALS:
    void categoryChanged();
    void packageChanged();
    void error();

private Q_SLOTS:
    void getPackagesFinished(QNetworkReply *reply);
    void getPackageFinished(QNetworkReply *reply);

private:
    void m_get();
    void m_pkGet();
    void m_httpGet();

private:
    QList<QObject*> m_dataList;
    QNetworkAccessManager m_pks;
    int m_pks_size;
    int m_pks_act_size;
    QNetworkAccessManager m_pk;
    QString m_category;
};

class PackageObject : public QObject 
{
    Q_OBJECT
    
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)

    Q_PROPERTY(QString size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(QString needInstall READ needInstall WRITE setNeedInstall NOTIFY needInstallChanged)

public:
    PackageObject(QObject *parent = 0) : QObject(parent) {}
    PackageObject(const QString &name, const QString &title, 
        const QString &description, const QString &icon, const QString &url, const QString &size,const QString &needInstall,
        QObject *parent = 0)
      : QObject(parent) 
    {
        m_name = name; m_title = title; m_description = description; 
        m_icon = icon; m_url = url;m_size = size;
        m_needInstall = needInstall;
    }

    QString name() const { return m_name; }
    void setName(const QString &name) 
    { 
        if (name != m_name) m_name = name; emit nameChanged(); 
    }

    QString title() const { return m_title; }
    void setTitle(const QString &title) 
    {
        if (title != m_title) m_title = title; emit titleChanged();
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

    QString needInstall() const { return m_needInstall; }
    void setNeedInstall(const QString &needInstall)
    {
        if (needInstall != m_needInstall) m_needInstall = needInstall; emit needInstallChanged();
    }

Q_SIGNALS:
    void nameChanged();
    void titleChanged();
    void descriptionChanged();
    void iconChanged();
    void urlChanged();
    void sizeChanged();
    void needInstallChanged();

private:
    QString m_name;
    QString m_title;
    QString m_description;
    QString m_icon;
    QString m_url;
    QString m_size;
    QString m_needInstall;
};

#endif  // PACKAGE_BY_CATEGORY_MODEL_H
