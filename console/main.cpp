/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 * Copyright (C) 2015 fujiang <fujiang.zhu@i-soft.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <QCoreApplication>
#include <QObject>
#include <iostream>
#include <string>
#include <glib.h>
#include <glib/gi18n.h>
#include <cstring>
#include <iomanip>

#include "isoftapp-generated.h"

using namespace std;

/* The following enums defined in ../src/daemon.h;*/
typedef enum {
    STATUS_INSTALL,
    STATUS_UPGRADE,
    STATUS_REMOVE,
    STATUS_INSTALLED,
    STATUS_REMOVED,
    STATUS_UPDATED,
    STATUS_UPGRADED,
    STATUS_SEARCHED,
    STATUS_INSTALL_ERROR,
    STATUS_REMOVE_ERROR,
    STATUS_UPGRADE_ERROR,
    STATUS_UNKNOWN,
} Status;

typedef enum {
    INSTALLED,
    UNINSTALLED,
    SEARCH_UNKNOWN,
} SearchStatus;

typedef enum {
    ERROR_FAILED,
    ERROR_PERMISSION_DENIED,
    ERROR_NOT_SUPPORTED,
    ERROR_SEARCH,
    ERROR_TASK_LOCKED,
    ERROR_CHECK,
    ERROR_INSTALL,
    ERROR_REMOVE,
    ERROR_UPDATE,
    ERROR_PKG_NOT_EXIST,
    ERROR_PKG_NOT_REMOVED,
    ERROR_PKG_NOT_UPGRADED,
    NUM_ERRORS
} Error;

typedef enum {
    ERROR_CODE_NETWORK,
    ERROR_CODE_DOWNLOAD,
    ERROR_CODE_INSTALL,
    ERROR_CODE_REMOVE,
    ERROR_CODE_SEARCH,
    ERROR_CODE_UPDATEING,
    ERROR_CODE_OTHERS
} ErrorCode;

org::isoftlinux::Isoftapp *m_isoftapp = Q_NULLPTR;

static void usage() 
{
    std::cout << _("Usage: isoftapp [options] command\n"
                   "       isoftapp [options] install|remove pkg1 [pkg2 ...]\n") << std::endl;

    std::cout << _("isoftapp is a simple command line interface for downloading and\n"
                   "installing packages. The most frequently used commands are update\n"
                   "and install.\n") << std::endl;

    std::cout << _("Commands:\n"
                   "   search - Search packages\n"
                   "   update - Retrieve new lists of packages\n"
                   "   upgrade - Perform an upgrade\n"
                   "   install - Install new package (pkg is libc6 not libc6.rpm)\n"
                   "   remove - Remove package\n"
                   "   check - Verify that there are no broken dependencies\n"
                   "   list-installed - list \"All installed package(s)\" in \"isoftapp RPM DB\"\n"
                   "   list-uninstalled - list \"All uninstalled packages\" in \"Repos\"\n") << std::endl;

    std::cout << _("Options:\n"
                   "   -f\tforce to remove packages nodeps") << std::endl;
    std::cout << _("Tips:\n"
                   "   try to use \"isoftapp update\" if meet with any problems.") << std::endl;
}
static void PrintHashes(double Percent)
{
   int hashesTotal = 30;
   int hashesNeeded = int(hashesTotal * Percent / 100);

   cout << setw(hashesNeeded) << setfill('#') << "";
   cout << setw(hashesTotal-hashesNeeded) << setfill(' ') << "";
   cout << right << " [" << setw(3) << int(Percent) << "%]" << left;
   if (hashesTotal == hashesNeeded) {
      cout << endl;
   }
}
static QString g_fileName="";
static bool g_finished = false;
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, DATADIR "/locale");
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    if (argc < 2) {
        usage();
        return 0;
    }

    m_isoftapp = new org::isoftlinux::Isoftapp("org.isoftlinux.Isoftapp", 
                                               "/org/isoftlinux/Isoftapp", 
                                               QDBusConnection::systemBus());
    QObject::connect(m_isoftapp, &org::isoftlinux::Isoftapp::Finished, 
            [=](const QString &pkgName,qlonglong status){
        if (status == STATUS_UPDATED) {
            std::cout << "Reading Package Lists... Done" << std::endl;
            std::cout << "Building Dependency Tree... Done" << std::endl;
        }
        QCoreApplication::quit();
    });

    // ListAllChanged

    QObject::connect(m_isoftapp, &org::isoftlinux::Isoftapp::ListAllChanged,
            [=](const QString &pkgName, qlonglong status){
        std::cout << pkgName.toStdString() << " " <<  std::endl;
    });

    QObject::connect(m_isoftapp, &org::isoftlinux::Isoftapp::SearchChanged,
            [=](const QString &pkgName, qlonglong status){
        std::cout << pkgName.toStdString() << " " << 
            std::string(status ? _("Available") : _("Installed")) << std::endl;
    });
    QObject::connect(m_isoftapp, &org::isoftlinux::Isoftapp::PercentChanged, 
            [=](qlonglong status, const QString &file, double percent){
                
        if (status == STATUS_INSTALLED) {
            std::cout << file.toStdString() << " " << std::string(_("Installed")) << std::endl;
            return;
        } else if (status == STATUS_REMOVED) {
            std::cout << file.toStdString() << " " << std::string(_("Available")) << std::endl;
            return;
        } else if(status != STATUS_REMOVE && status !=STATUS_INSTALL ) {
            if ((int)(percent*100.0) < 0 || (int)(percent*100.0) > 100 ) {
                return;
            }
            if (g_fileName == file && (int)(percent*100.0) == 100 ) {
                if (g_finished)
                    return;
                g_finished = true;
            }
            if (g_fileName != file && !g_fileName.isEmpty()) {
                g_finished = false;
            }
            g_fileName = file;
        }
        
        std::cout.setf(std::ios_base::left);
        std::string s(file.toStdString());
        const int namemax = 40;
        std::cout << "\r";
        std::cout << std::setw(namemax) << s.substr(0, namemax) << " ";
#if 0
        int Percent = 100 * percent;
        int hashesTotal = 30;
        int hashesNeeded = int(hashesTotal * Percent / 100);

        std::cout << std::setw(hashesNeeded) << std::setfill('#') << "";
        std::cout << std::setw(hashesTotal-hashesNeeded) << std::setfill(' ') << "";
        std::cout << right << " [" << std::setw(3) << int(Percent) << "%]" << left;
        if (hashesTotal == hashesNeeded) {
            std::cout << std::endl;
        }
#else
        PrintHashes(percent*100.0);
#endif
        std::cout << flush;
	});
    QObject::connect(m_isoftapp, &org::isoftlinux::Isoftapp::ListChanged, 
            [=](const QString &pkgName) {
        std::cout << pkgName.section('|',-1).toStdString() << std::endl;
    });

    QObject::connect(m_isoftapp, &org::isoftlinux::Isoftapp::CheckChanged,
            [=](const QString &pkgName){
        if(pkgName.compare("All seem ok.") ==0) {
            std::cout << std::string(_("All seem ok.")) << std::endl;
        } else
            std::cout << pkgName.toStdString() << std::endl;
    });


    QObject::connect(m_isoftapp, &org::isoftlinux::Isoftapp::Error,
            [=](qlonglong error, const QString &details, qlonglong errcode){
        if (error == ERROR_PKG_NOT_EXIST) {
            if (errcode == ERROR_CODE_OTHERS) {
                std::cout << details.toStdString() << " " << std::string(_("No this package")) << std::endl;
            }
            return;
        } else if(error == ERROR_PKG_NOT_REMOVED) {
            if (errcode == ERROR_CODE_OTHERS) {
                std::cout << details.toStdString() << " " << std::string(_("No this package or not installed")) << std::endl;
            }
            return;
        } else if(error == ERROR_PKG_NOT_UPGRADED) {
            if (errcode == ERROR_CODE_OTHERS) {
                if(details == "0") {
                    std::cout <<  std::string(_("Not need upgradation")) << std::endl;
                } else
                std::cout << details.toStdString() << " " << std::string(_("Not need upgradation")) << std::endl;
            }
            return;
        } else if(error == ERROR_SEARCH) {
            if (errcode == ERROR_CODE_SEARCH) {
                std::cout << details.toStdString() << " " << std::string(_("No this package")) << std::endl;
            }
            return;
        }
    });

    if (strcmp(argv[1], "search") == 0) {
        if (argc < 3) {
            usage();
            return 0;
        }
        m_isoftapp->Search(argv[2]);
    } else if (strcmp(argv[1], "update") == 0) {
        m_isoftapp->Update();
    } else if (strcmp(argv[1], "upgrade") == 0) {
        m_isoftapp->Upgrade(argv[2] ? argv[2] : "0");
    } else if (strcmp(argv[1], "install") == 0) {
        if (argc < 3) {
            usage();
            return 0;
        }
        m_isoftapp->Install(argv[2]);
    } else if (strcmp(argv[1], "remove") == 0) {
        if (argc < 3) {
            usage();
            return 0;
        }
        if (strcmp(argv[2], "-f") == 0)
            m_isoftapp->Remove(argv[3], true);
        else 
            m_isoftapp->Remove(argv[2], false);
    } else if (strcmp(argv[1], "-f") == 0) {
        if (argc < 4 || strcmp(argv[2], "remove")!=0) {
            usage();
            return 0;
        }
        m_isoftapp->Remove(argv[3], true);
    } else if (strcmp(argv[1], "check") == 0) {
        m_isoftapp->Check();
    } else if (strcmp(argv[1], "list-installed") == 0) {
        m_isoftapp->ListInstalled();
    } else if (strcmp(argv[1], "list-uninstalled") == 0) {
        m_isoftapp->ListUninstalled();
    } else if (strcmp(argv[1], "list-all") == 0) {
        m_isoftapp->ListAll("abcc");
    } else {
        usage();
        return 0;
    }

    return app.exec();
}
