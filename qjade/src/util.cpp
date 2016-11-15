/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 *               2016 fujiang <fujiang.zhu@i-soft.com.cn>
 */

#include <QFile>
#include <QDebug>

#include "util.h"
#include "globaldeclarations.h"

bool isFileExists(QString filePath) 
{
    QFile file(filePath);
    return file.exists();
}

QString absIconPath(QString pkIconPath) 
{
#if QJADE_DEBUG
    if (pkIconPath != "")
        qDebug() << "DEBUG:" << __PRETTY_FUNCTION__ << pkIconPath;
#endif
    QString ret = pkIconPath;
    
    //-------------------------------------------------------------------------
    // TODO: 无法使用（基于Qt4的）libkdeui的KIconLoader
    // 需要实现一个通过PackageKit的pkIconPath得到真正的图标路径
    // PackageKit的pkIconPath可能是：
    // 1. package-name.png
    // 2. 空
    // 3. package-name
    // 4. 图标路径（这是最喜欢的）
    //-------------------------------------------------------------------------
   
    // 例如，空
    if (ret == "")
        return ret;
    if (ret == "lib")
        return "";
    if (pkIconPath == "supertuxkart_64")                                       
        return USR_SHARE_PIXMAP_DIR + "supertuxkart_128" + IMAGE_EXT;

    // 例如，是CMakeSetup32.png
    if (ret.endsWith(IMAGE_EXT) || ret.endsWith(".xpm")) {
        if (!isFileExists(ret)) {
            ret = USR_SHARE_PIXMAP_DIR + ret;
            if (!isFileExists(ret)) 
                ret = USR_SHARE_ICON_DIR + pkIconPath;
        }
    }
    // 例如，chromium
    if (!ret.startsWith("/")) {
        ret = USR_SHARE_PIXMAP_DIR + ret + IMAGE_EXT;
        if (!isFileExists(ret)) 
            ret = USR_SHARE_PIXMAP_DIR + pkIconPath + ".xpm";
        if (!isFileExists(ret)) 
            ret = USR_SHARE_ICON_DIR + pkIconPath + IMAGE_EXT;
        if (!isFileExists(ret)) { 
            ret = USR_SHARE_ICON_DIR + OXYGEN_THEME + "/48x48/apps/" + 
                pkIconPath + IMAGE_EXT;
        }
        if (!isFileExists(ret)) {
            ret = USR_SHARE_ICON_DIR + HICOLOR_THEME + "/48x48/apps/" + 
                pkIconPath + IMAGE_EXT;
        }
        if (!isFileExists(ret)) {
            ret = USR_SHARE_ICON_DIR + OXYGEN_THEME + "/48x48/categories/" +             
                pkIconPath + IMAGE_EXT;
        }
        if (!isFileExists(ret)) {                                                  
            ret = USR_SHARE_ICON_DIR + OXYGEN_THEME + "/48x48/devices/" +       
                pkIconPath + IMAGE_EXT;                                            
        }
        if (!isFileExists(ret)) {                                                  
            ret = USR_SHARE_ICON_DIR + OXYGEN_THEME + "/48x48/actions/" +          
                pkIconPath + IMAGE_EXT;                                            
        }
        if (!isFileExists(ret)) {                                                  
            ret = USR_SHARE_ICON_DIR + HICOLOR_THEME + "/128x128/apps/" +            
                pkIconPath + IMAGE_EXT;                                            
        }
        if (!isFileExists(ret)) {                                                  
            ret = USR_SHARE_ICON_DIR + OXYGEN_THEME + "/32x32/devices/" +             
                pkIconPath + IMAGE_EXT;                                            
        }
        if (!isFileExists(ret)) {                                                  
            ret = USR_SHARE_ICON_DIR + HICOLOR_THEME + "/32x32/apps/" +           
                pkIconPath + IMAGE_EXT;                                            
        }
        if (!isFileExists(ret)) {                                                  
            ret = USR_SHARE_ICON_DIR + HICOLOR_THEME + "/22x22/apps/" +           
                pkIconPath + IMAGE_EXT;                                            
        }
        if (!isFileExists(ret)) {
            ret = USR_SHARE_ICON_DIR + HICOLOR_THEME + "/scalable/apps/" + 
                pkIconPath + ".svg";
        }
        if (!isFileExists(ret)) {
            ret = USR_SHARE_ICON_DIR + HICOLOR_THEME + "/128x128/mimetypes/" + 
                pkIconPath + IMAGE_EXT;
        }
    }

    return ret;
}

bool getStrSize(QString &size,QString &dstSize)
{
    if (size.isEmpty())
        return false;

    int iSize = size.toInt();
    int kb = iSize/1024;
    double mb =0.0;
    char sUnit[4]="";
    iSize = kb/1024;
    if(iSize > 0) {
        mb = (double)kb/1024.0;
        strcpy(sUnit,"MB");
    } else {
        mb = (double)size.toInt()/1024.0;
        strcpy(sUnit,"KB");
    }
    char sSize[32]="";
    snprintf(sSize,sizeof(sSize),"%.02f%s",mb,sUnit);
    dstSize = QString(sSize);
    return true;
}

