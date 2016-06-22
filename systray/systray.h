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

#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QSystemTrayIcon>

#ifndef QT_NO_SYSTEMTRAYICON

#include <QWidget>
#include <QMenu>

#include "isoftapp-generated.h"
#include "progresswidget.h"
#include "upgradewidget.h"

class SysTray : public QWidget
{
    Q_OBJECT

public:
    explicit SysTray(QWidget *parent = Q_NULLPTR);
    ~SysTray();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void setIcon();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showMessage();
    void messageClicked();

private:
    void createActions();
    void createTrayIcon();

    QAction *testProgressAction;
    QAction *testUpgradeAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    org::isoftlinux::Isoftapp *isoftapp;
    bool showPopup = false;
    ProgressWidget *progress = Q_NULLPTR;
    UpgradeWidget *upgrade = Q_NULLPTR;
};

#endif // QT_NO_SYSTEMTRAYICON

#endif  // SYSTRAY_H
