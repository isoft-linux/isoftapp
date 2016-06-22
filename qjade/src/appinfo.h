/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

#ifndef APP_INFO_H
#define APP_INFO_H

#include <QObject>

#include "globaldeclarations.h"

class AppInfo : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString version READ version NOTIFY versionChanged)

public:
    QString version() const { return APPLICATION_VERSION; }

Q_SIGNALS:
    void versionChanged();
};

#endif  // APP_INFO_H
