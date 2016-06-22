/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

#ifndef SLIDESHOW_MODEL_H
#define SLIDESHOW_MODEL_H

#include <QObject>
#include <QList>

#include "httpget.h"

class SlideShowObject;

class SlideShowModel : public HttpGet
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject*> slideshow READ slideshow NOTIFY slideshowChanged)

public:
    SlideShowModel(HttpGet* parent = 0);
    ~SlideShowModel();

    Q_INVOKABLE void refresh();

    QList<QObject*> slideshow() const;

Q_SIGNALS:
    void slideshowChanged();
    void error();

protected:
    void finished(QNetworkReply *reply);

private:
    void m_cleanup();

private:
    QList<QObject*> m_dataList;
};

class SlideShowObject : public QObject 
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)

public:
    SlideShowObject(const QString &name, 
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

#endif  // SLIDESHOW_MODEL_H
