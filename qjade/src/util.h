/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

#ifndef UTIL_H
#define UTIL_H

#include <QObject>

bool isFileExists(QString filePath);

QString absIconPath(QString pkIconPath);
bool getStrSize(QString &srcSize,QString &dstSize);
#endif // UTIL_H
