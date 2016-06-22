/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
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

#ifndef UPGRADE_WIDGET_H
#define UPGRADE_WIDGET_H

#include <QWidget>
#include <QListWidget>

#include "isoftapp-generated.h"

class UpgradeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UpgradeWidget(org::isoftlinux::Isoftapp *isoftapp, 
        QWidget *parent = Q_NULLPTR, 
        Qt::WindowFlags f = Qt::Popup);
    ~UpgradeWidget();

    void AddUpgrade(const QString &pkgName);

protected:
    void focusOutEvent(QFocusEvent *event);

private:
    QListWidget *pkgList;
};

#endif  // UPGRADE_WIDGET_H
