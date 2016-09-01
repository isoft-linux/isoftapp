/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

#ifndef GLOBAL_DECLARATIONS_H
#define GLOBAL_DECLARATIONS_H

#include <QString>
#include <QLocale>

//-----------------------------------------------------------------------------
// TODO: Software relative information
//-----------------------------------------------------------------------------
const QString CODE_NAME = "qjade";
const QString APPLICATION_VERSION = "2.2.9";
const QString APPLICATION_ENCODING = "UTF-8";
const QString TRANSLATIONS_PATH = "translations";

const QString SERVER_HOST = "http://appstore.isoft-linux.org/"; 
const QString CATEGORY_URI = SERVER_HOST + "appstore/categories/" + QLocale::system().name().toLower();
const QString SLIDESHOW_URI = SERVER_HOST + "appstore/advertisements/" + QLocale::system().name().toLower();
const QString GRIDVIEW_URI = SERVER_HOST + "appstore/sticky/" + QLocale::system().name().toLower();
const QString CATEGORY_PACKAGE_URI = SERVER_HOST + "appstore/category-packages/";
const QString PACKAGE_URI = SERVER_HOST + "appstore/package/";
const QString SEARCH_URI = SERVER_HOST + "appstore/search/";

//-----------------------------------------------------------------------------
// TODO: package icon path
//-----------------------------------------------------------------------------
const QString CATEGORY_ALL = "category-all";
const QString USR_SHARE_PIXMAP_DIR = "/usr/share/pixmaps/";
const QString USR_SHARE_ICON_DIR = "/usr/share/icons/";
const QString IMAGE_EXT = ".png";
const QString OXYGEN_THEME = "oxygen";
const QString HICOLOR_THEME = "hicolor";

#endif // GLOBAL_DECLARATIONS_H
