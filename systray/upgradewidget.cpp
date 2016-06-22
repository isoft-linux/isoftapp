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

#include "upgradewidget.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>

UpgradeWidget::UpgradeWidget(org::isoftlinux::Isoftapp *isoftapp, 
    QWidget *parent, 
    Qt::WindowFlags f)
  : QWidget(parent, f)
{
    resize(300, 200);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setAlignment(Qt::AlignVCenter);
    pkgList = new QListWidget;
    vbox->addWidget(pkgList);
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setAlignment(Qt::AlignRight);
    vbox->addLayout(hbox);
    QPushButton *button = new QPushButton(_("Upgrade"));
    connect(button, &QPushButton::clicked, [=](){
        for (int row = 0; row < pkgList->count(); row++) {
            QCheckBox *obj = (QCheckBox *)pkgList->itemWidget(pkgList->item(row));
            if (obj->isChecked())
                isoftapp->Upgrade(obj->text());
        }
    });
    button->setMaximumWidth(100);
    hbox->addWidget(button);
    setLayout(vbox);
}

UpgradeWidget::~UpgradeWidget() 
{
    if (pkgList) {
        delete pkgList;
        pkgList = Q_NULLPTR;
    }
}

void UpgradeWidget::focusOutEvent(QFocusEvent *event) 
{
    hide();
}

void UpgradeWidget::AddUpgrade(const QString &pkgName) 
{
    QCheckBox *choose = new QCheckBox(pkgName);
    choose->setChecked(true);
    QListWidgetItem *item = new QListWidgetItem;
    pkgList->addItem(item);
    pkgList->setItemWidget(item, choose);
}
