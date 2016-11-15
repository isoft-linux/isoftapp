/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community
 *               2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 *               2016 fujiang <fujiang.zhu@i-soft.com.cn>
 */

#include <QApplication>
#include <QtQml>
#include <QtQuick/QQuickView>

#include "runguard.h"
#include "globaldeclarations.h"
#include "i18n.h"
#include "brand.h"
#include "appinfo.h"
#include "categorymodel.h"
#include "slideshowmodel.h"
#include "iconmodel.h"
#include "packagebycategorymodel.h"
#include "filedownloader.h"
#include "process.h"
#include "packageinfomodel.h"
#include "jadedbus.h"
#include "searchmodel.h"
#include "mypkgmodel.h"
#include "globallist.h"

static org::isoftlinux::Isoftapp *g_isoftapp = Q_NULLPTR;

/*
* 2016-03 新功能开发
* 新增加三个全局列表：
* 1.AllPkgList：从isoftapp-daemon里面，通过dbus接口获取所有包（内容包括名称和已安装/未安装标志）;[jadedbus.cpp]
* 2.QList <TaskQueue> m_taskQueue : 安装/卸载/升级时，存储需要操作的包，操作完成后，自动从此队列中删除包;[jadedbus.cpp]
* 3.g_qjadePkgList：保存appstore服务器上面的所有软件包的信息
* 4.在处理页面请求时，会用 g_qjadePkgList 和 AllPkgList 作比较，只有都在此两个表中的数据才会提交给页面。
*/


// get data from isoftapp-daemon(sqlite)
QList<t_PKGINFO> AllPkgList;

/*
* 此列表保存appstore服务器上面的所有软件包的信息，更新此表的时刻有：
* 1.qjade 启动时第一次获取数据：[CategoryModel::finished()]
* a、先获取类别 appstore/categories/zh_cn
* b、再获取每个类别里面的软件包的名称 appstore/category-packages/Video-Graphics/zh_cn
* c、再根据软件包名称，获取此包的详细信息：appstore/package/Gimp/zh_cn
* 2.点击左侧的每个类别时，会更新g_qjadePkgList.[src/packagebycategorymodel.cpp]
*
*/
QList<t_QJADE_PKGINFO> g_qjadePkgList;

/*
* 保存 appstore服务器上面的类型，包括 name,title.
* CategoryModel类构造时获取数据
* 页面根据 name（英文）获取title（中文或其它语言)
*/
QList<t_CATEGORY_INFO> g_categoryList;

QStringList g_backendList;

int main(int argc, char *argv[]) 
{                                                                        
    //-------------------------------------------------------------------------
    // TODO: Simple single instance.
    // It is better to add XRaiseWindow when running it again.
    //-------------------------------------------------------------------------
    RunGuard guard("{e01eb7c7-da64-40df-ae53-09ee6d49ab0d}");
    if (!guard.tryToRun()) {
        qWarning("QJade is already running!");
        return 0;
    }
    
    QApplication app(argc, argv);
    app.setApplicationName(CODE_NAME);
    app.setApplicationVersion(APPLICATION_VERSION);
    //-------------------------------------------------------------------------
    // TODO: Brand information
    //-------------------------------------------------------------------------
    Brand brand;
    app.setOrganizationName(brand.org());

    //-------------------------------------------------------------------------
    // TODO: i18n
    //-------------------------------------------------------------------------
    I18N i18n(TRANSLATIONS_PATH, APPLICATION_ENCODING);
    i18n.translate();
    
    const QString productName = brand.name();

    QString backfile = QString(DATADIR) + "/" + "qjade/backendlist.txt";
    QFile file(backfile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!file.atEnd()) {
            QString lineStr = QString(file.readLine());
            if (lineStr.endsWith("\n"))
                lineStr.truncate(lineStr.length() - 1);

            g_backendList.append(lineStr);
        }
    }

    //begin
    g_isoftapp = new org::isoftlinux::Isoftapp("org.isoftlinux.Isoftapp",
                                                   "/org/isoftlinux/Isoftapp",
                                                   QDBusConnection::systemBus());
    g_isoftapp->ListAll("pkgs");
    JadedBus jadedbus;
    //end

    //-------------------------------------------------------------------------
    // TODO: qmlRegisterType
    //-------------------------------------------------------------------------
    qmlRegisterType<JadedBus>("cn.com.isoft.qjade", 2, 0, "JadedBus");
    qmlRegisterType<Brand>("cn.com.isoft.qjade", 2, 0, "Brand");
    qmlRegisterType<AppInfo>("cn.com.isoft.qjade", 2, 0, "AppInfo");
    qmlRegisterType<CategoryModel>("cn.com.isoft.qjade", 2, 0, "CategoryModel");
    qmlRegisterType<SlideShowModel>("cn.com.isoft.qjade", 2, 0, "SlideShowModel");
    qmlRegisterType<IconModel>("cn.com.isoft.qjade", 2, 0, "IconModel");
    qmlRegisterType<PackageByCategoryModel>("cn.com.isoft.qjade", 2, 0, "PackageByCategoryModel");
    qmlRegisterType<FileDownloader>("cn.com.isoft.qjade", 2, 0, "FileDownloader");
    qmlRegisterType<Process>("cn.com.isoft.qjade", 2, 0, "Process");
    qmlRegisterType<PackageInfoModel>("cn.com.isoft.qjade", 2, 0, "PackageInfoModel");
    //qmlRegisterType<JadedBus>("cn.com.isoft.qjade", 2, 0, "JadedBus");
    qmlRegisterType<SearchModel>("cn.com.isoft.qjade", 2, 0, "SearchModel");
    qmlRegisterType<MypkgModel>("cn.com.isoft.qjade", 2, 0, "MypkgModel");

    //-------------------------------------------------------------------------
    // TODO: QQmlApplicationEngine
    //-------------------------------------------------------------------------
    QUrl filePath = QString(DATADIR) + "/" + "qjade/qml/main.qml";
    QQmlApplicationEngine engine(filePath);
    //QQmlApplicationEngine engine(QUrl("qrc:/qml/main.qml"));
    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel); 
    if (!window) {                                                 
        qWarning("Error: Your root item has to be a Window.");
        return -1;                                                       
    }
    window->setTitle(productName);
    window->setIcon(QIcon(brand.logo()));
    window->show();
    return app.exec();                                       
}
