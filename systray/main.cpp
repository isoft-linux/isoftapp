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

#include <QtSolutions/QtSingleApplication>

#ifndef QT_NO_SYSTEMTRAYICON

#include <unistd.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <QMessageBox>
#include "systray.h"

int main(int argc, char *argv[])
{
    return 0;
    QtSingleApplication app(argc, argv);
    if (app.isRunning())
        return 0;
    app.setQuitOnLastWindowClosed(false);
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        sleep(13);
    }
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, DATADIR "/locale");
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
    SysTray systray;
    return app.exec();
}

#else

#include <QLabel>
#include <QDebug>

int main(int argc, char *argv[])
{
    QtSingleApplication app(argc, argv);
    if (app.isRunning())
        return 0;
    QString text(_("QSystemTrayIcon is not supported on this platform"));

    QLabel *label = new QLabel(text);
    label->setWordWrap(true);

    label->show();
    qDebug() << text;

    app.exec();
}

#endif
