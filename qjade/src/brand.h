/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

#ifndef BRAND_H
#define BRAND_H

#include <QObject>
#include <QFile>
#include <QDomDocument>
#include <QDebug>

class Brand : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString logo READ logo NOTIFY logoChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString org READ org NOTIFY orgChanged)
    Q_PROPERTY(QString slogan READ slogan NOTIFY sloganChanged)

public:
    Brand(QObject *parent = 0);

    QString logo() const;
    QString name() const;
    QString org() const;
    QString slogan() const;

Q_SIGNALS:
    void logoChanged();
    void nameChanged();
    void orgChanged();
    void sloganChanged();

private:
    void m_parseElement(QDomElement e, QString locale, QString &v);
    void m_parse(QFile &file);
    
    QString m_logo;
    QString m_name;
    QString m_org;
    QString m_slogan;
};

#endif  // BRAND_H
