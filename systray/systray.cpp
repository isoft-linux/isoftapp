/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
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

#include "systray.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include <glib/gi18n.h>

#include <QAction>
#include <QMessageBox>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QApplication>
#include <QDesktopWidget>

#define PANEL_HEIGHT 30

SysTray::SysTray(QWidget *parent)
  : QWidget(parent)
{
    createActions();
    createTrayIcon();

    setIcon();

    connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    isoftapp = new org::isoftlinux::Isoftapp("org.isoftlinux.Isoftapp", 
                                             "/org/isoftlinux/Isoftapp", 
                                             QDBusConnection::systemBus(), 
                                             this);
    connect(isoftapp, &org::isoftlinux::Isoftapp::Finished, 
            [this](const QString &pkgName,qlonglong status) {
        if (progress)
            progress->hide();
        if (upgrade)
            upgrade->hide();
        trayIcon->setVisible(false);
        showPopup = false;
    });
    connect(isoftapp, &org::isoftlinux::Isoftapp::PercentChanged, 
            [this](qlonglong status, const QString &file, double percent) {
        trayIcon->setVisible(true);
    	if (progress) {
            progress->setPercent(status, file, percent);
            if (!showPopup)
                progress->show();
            showPopup = true;
        } else {
            progress = new ProgressWidget(this);
            progress->move(QApplication::desktop()->width() - progress->width(), 
                QApplication::desktop()->height() - progress->height() - PANEL_HEIGHT);
        }
	});
    connect(isoftapp, &org::isoftlinux::Isoftapp::UpgradeChanged, 
            [this](const QString &pkgName) {
        trayIcon->setVisible(true);
        if (upgrade) {
            upgrade->AddUpgrade(pkgName);
            upgrade->show();
        } else {
            upgrade = new UpgradeWidget(isoftapp, this);
            upgrade->move(QApplication::desktop()->width() - progress->width(),
                QApplication::desktop()->height() - progress->height() - PANEL_HEIGHT);
        }
    });
}

SysTray::~SysTray() 
{
    if (isoftapp) {
        delete isoftapp;
        isoftapp = Q_NULLPTR;
    }

    if (progress) {
        delete progress;
        progress = Q_NULLPTR;
    }

    if (upgrade) {
        delete upgrade;
        upgrade = Q_NULLPTR;
    }
}

void SysTray::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        QMessageBox::information(this, tr("Systray"),
                                 tr("The program will keep running in the "
                                    "system tray. To terminate the program, "
                                    "choose <b>Quit</b> in the context menu "
                                    "of the system tray entry."));
        hide();
        event->ignore();
    }
}

void SysTray::setIcon()
{
    QIcon icon = QIcon::fromTheme("task-new");
    trayIcon->setIcon(icon);
#if DEBUG
    trayIcon->setToolTip(_("DEBUG MODE"));
    trayIcon->setVisible(true);
#endif
}

void SysTray::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        showPopup = false;
        break;
    case QSystemTrayIcon::DoubleClick:
        showPopup = false;
        break;
    case QSystemTrayIcon::MiddleClick:
        showMessage();
        break;
    default:
        break;
    }
}

void SysTray::showMessage()
{
#if DEBUG
    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(0);
    QString debug = "DEBUG: " + QString(__PRETTY_FUNCTION__);
    trayIcon->showMessage(debug, debug, icon, 13000);
#endif
}

void SysTray::messageClicked()
{
#if DEBUG
    QMessageBox::information(0, tr("Systray"),
                             tr("Sorry, I already gave what help I could.\n"
                                "Maybe you should try asking a human?"));
#endif
}

void SysTray::createActions()
{
    testProgressAction = new QAction(_("Test Progress"), this);
    connect(testProgressAction, &QAction::triggered, [this](){
        if (!progress)
            progress = new ProgressWidget(this);
        progress->move(QApplication::desktop()->width() - progress->width(), 
            QApplication::desktop()->height() - progress->height() - PANEL_HEIGHT);
        progress->show();
    });

    testUpgradeAction = new QAction(_("Test Upgrade"), this);
    connect(testUpgradeAction, &QAction::triggered, [this](){
        if (!upgrade)
            upgrade = new UpgradeWidget(isoftapp, this);
        upgrade->move(QApplication::desktop()->width() - upgrade->width(),
            QApplication::desktop()->height() - upgrade->height() - PANEL_HEIGHT);
        upgrade->show();
    });
    
    quitAction = new QAction(_("&Quit"), this);
    connect(quitAction, &QAction::triggered, [=]{
        QCoreApplication::quit();
    });
}

void SysTray::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(testProgressAction);
    trayIconMenu->addAction(testUpgradeAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
#if DEBUG
    trayIcon->setContextMenu(trayIconMenu);
#endif
}

#endif
