/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

#include "brand.h"

Brand::Brand(QObject *parent) 
  : QObject(parent), 
    m_name(""), 
    m_org(""), 
    m_slogan("") 
{

    QString filePath = QString(DATADIR) + "/" + QString(TARGET)  + "/" + 
        QString(BRAND) + ".xml";
    QFile file(filePath);
    //-------------------------------------------------------------------------
    // TODO: open the XML file with read-only mode
    //-------------------------------------------------------------------------
    if (file.open(QIODevice::ReadOnly)) 
        m_parse(file);
    else 
        qDebug() << "ERROR: fail to open brand file" << filePath;

    printf("\n trace %s,%d,DATADIR[%s],TARGET[%s]BRAND[%s]filePath[%s]!!\n",
           __FUNCTION__,__LINE__ ,DATADIR,TARGET,BRAND,qPrintable(filePath));
}

void Brand::m_parseElement(QDomElement e, QString locale, QString &v) 
{
    for (int i = 0; i < e.childNodes().count(); i++) {
        QDomElement child = e.childNodes().item(i).toElement();
        if (child.nodeName() == locale) {
            v = child.text(); 
            return;
        }
    }
}

void Brand::m_parse(QFile &file) 
{
    QString locale = QLocale::system().name().toLower();
    QDomDocument doc;
    doc.setContent(&file);
    QDomElement root = doc.documentElement();
    //-------------------------------------------------------------------------
    // Sorry I have a bad coding style, I just wanna write the relative source 
    // code in a single line
    //-------------------------------------------------------------------------
    QDomElement logo = root.firstChildElement("logo"); 
    m_logo = "file://" + QString(DATADIR) + "/" + QString(TARGET) + "/" + logo.text(); 
    emit logoChanged();
    m_parseElement(root.firstChildElement("name"), locale, m_name); 
    emit nameChanged();
    m_parseElement(root.firstChildElement("org"), locale, m_org); 
    emit orgChanged();
    m_parseElement(root.firstChildElement("slogan"), locale, m_slogan); 
    emit sloganChanged();

    printf("\n trace %s,%d,name[%s],org[%s]slogan[%s]m_logo[%s]!!\n",__FUNCTION__,__LINE__ ,
           qPrintable(m_name),qPrintable(m_org),qPrintable(m_slogan),qPrintable(m_logo));
}

QString Brand::logo() const { return m_logo; }

QString Brand::name() const { return m_name; }

QString Brand::org() const { return m_org; }

QString Brand::slogan() const { return m_slogan; }
