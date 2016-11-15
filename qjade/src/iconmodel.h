/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 *               2016 fujiang <fujiang.zhu@i-soft.com.cn>
 */

#ifndef ICON_MODEL_H
#define ICON_MODEL_H

#include <QObject>
#include <QList>

#include "httpget.h"

class IconObject;

class IconModel : public HttpGet
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject*> icons READ icons NOTIFY iconChanged)

public:
    IconModel(HttpGet* parent = 0);
    ~IconModel();

    Q_INVOKABLE void refresh();

    QList<QObject*> icons() const;

Q_SIGNALS:
    void iconChanged();
    void error();

protected:
    void finished(QNetworkReply *reply);

private:
    void m_cleanup();
    void useCache();

private:
    QList<QObject*> m_dataList;

private Q_SLOTS:
    void useCacheFinished();
};

class IconObject : public QObject 
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)

public:
    IconObject(const QString &name, 
               const QString &title, 
               const QString &icon, 
               const QString &url, 
               QObject *parent = 0)
      : QObject(parent) 
    {
        m_name = name; m_title = title; m_icon = icon; m_url = url;
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

    QString url() const { return m_url; }
    void setUrl(const QString &url) 
    {
        if (url != m_url) m_url = url; emit urlChanged();
    }

Q_SIGNALS:
    void nameChanged();
    void titleChanged();
    void iconChanged();
    void urlChanged();

private:
    QString m_name;
    QString m_title;
    QString m_icon;
    QString m_url;
};

#endif  // ICON_MODEL_H
