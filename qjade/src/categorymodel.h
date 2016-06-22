/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

#ifndef CATEGORY_MODEL_H
#define CATEGORY_MODEL_H

#include <QObject>
#include <QList>

#include "httpget.h"

class CategoryObject;

class CategoryModel : public HttpGet
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject*> categories READ categories NOTIFY categoryChanged)

public:
    CategoryModel(HttpGet* parent = 0);
    ~CategoryModel();

    QList<QObject*> categories() const;

Q_SIGNALS:
    void categoryChanged();
    void error();

protected:
    void finished(QNetworkReply *reply);

private:
    QList<QObject*> m_dataList;

    QNetworkAccessManager m_pk;
    QNetworkAccessManager m_pks;

private Q_SLOTS:
    void getPackageFinished(QNetworkReply *reply);
    void getPackagesFinished(QNetworkReply *reply);
};

class CategoryObject : public QObject 
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)

public:
    CategoryObject(const QString &name, 
                   const QString &title, 
                   const QString &icon, 
                   QObject *parent = 0)
      : QObject(parent) 
    {
        m_name = name; m_title = title; m_icon = icon;
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

    QString icon() const { return m_icon; }
    void setIcon(const QString &icon) 
    {
        if (icon != m_icon) m_icon = icon; emit iconChanged();
    }

Q_SIGNALS:
    void nameChanged();
    void titleChanged();
    void iconChanged();

private:
    QString m_name;
    QString m_title;
    QString m_icon;
};

#endif  // CATEGORY_MODEL_H
