/*                                                                              
 * Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

#ifndef SEARCH_MODEL_H
#define SEARCH_MODEL_H

#include <QObject>
#include <QList>

#include "httpget.h"

class SearchObject;

class SearchModel : public HttpGet
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject*> searchResult READ searchResult NOTIFY searchResultChanged)

public:
    SearchModel(HttpGet* parent = 0);
    ~SearchModel();

    Q_INVOKABLE void search(QString keyword);

    QList<QObject*> searchResult() const;

Q_SIGNALS:
    void searchResultChanged(int count);
    void error();

protected:
    void finished(QNetworkReply *reply);

private:
    void m_cleanup();

private:
    QList<QObject*> m_dataList;
};

class SearchObject : public QObject 
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setTitle NOTIFY descriptionChanged)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QString size READ size WRITE setSize NOTIFY sizeChanged)

public:
    SearchObject(const QString &name, 
               const QString &description, 
               const QString &icon, 
               const QString &size,
               QObject *parent = 0)
      : QObject(parent) 
    {
        m_name = name; m_description = description; m_icon = icon;m_size = size;
    }

    QString name() const { return m_name; }
    void setName(const QString &name) 
    { 
        if (name != m_name) m_name = name; emit nameChanged(); 
    }

    QString description() const { return m_description; }
    void setTitle(const QString &description) 
    {
        if (description != m_description) m_description = description; emit descriptionChanged();
    }

    QString icon() const { return m_icon; }
    void setIcon(const QString &icon) 
    {
        if (icon != m_icon) m_icon = icon; emit iconChanged();
    }
    QString size() const { return m_size; }
    void setSize(const QString &size)
    {
        if (size != m_size) m_size = size; emit sizeChanged();
    }

Q_SIGNALS:
    void nameChanged();
    void descriptionChanged();
    void iconChanged();
    void sizeChanged();

private:
    QString m_name;
    QString m_description;
    QString m_icon;
    QString m_size;
};

#endif  // SEARCH_MODEL_H
