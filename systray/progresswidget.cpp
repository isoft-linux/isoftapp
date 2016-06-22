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

#include "progresswidget.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <QVBoxLayout>
#include <QPainter>
#include <QBitmap>

ProgressWidget::ProgressWidget(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
    resize(300, 200);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setAlignment(Qt::AlignTop);
    title = new QLabel(_("<span style=\"font-size:16px;\">Installing</span>"));
    title->setAlignment(Qt::AlignHCenter);
    vbox->addWidget(title);
    vbox->addSpacing(50);
    label = new QLabel;
    vbox->addWidget(label);
    progress = new QProgressBar;
    vbox->addWidget(progress);
    setLayout(vbox);
}

ProgressWidget::~ProgressWidget() 
{
    if (title) {
        delete title;
        title = Q_NULLPTR;
    }

    if (progress) {
        delete progress;
        progress = Q_NULLPTR;
    }

    if (label) {
        delete label;
        label = Q_NULLPTR;
    }
}

/*
 defined in ../src/daemon.h
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
    STATUS_DOWNLOAD,
    STATUS_UNKNOWN,
} Status;
*/
void ProgressWidget::setPercent(qlonglong status, const QString &file, double percent) 
{
    QString text;
    if(status == 0 || status == 1) {
        text = _("<span style=\"font-size:16px;\">Installing</span>");
    } else if(status == 2) {
        text = _("<span style=\"font-size:16px;\">Uninstalling</span>");
    }else if(status == 11) {
        text = _("<span style=\"font-size:16px;\">Downloading</span>");
    }
    //title->setText(status == 0 ? (_("<span style=\"font-size:16px;\">Installing</span>")) : (_("<span style=\"font-size:16px;\">Uninstalling</span>")));
    title->setText(text);
    label->setText(file);
    progress->setValue(percent * 100.0);
}

void ProgressWidget::focusOutEvent(QFocusEvent *event) 
{
    hide();
}

void ProgressWidget::paintEvent(QPaintEvent *)
{
	QBitmap bmp(size());
    bmp.fill();
    QPainter p(&bmp);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(bmp.rect(), 10, 10);
    setMask(bmp);
}
