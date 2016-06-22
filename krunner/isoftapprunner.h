/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2015 - 2016 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef ISOFTAPP_RUNNER_H
#define ISOFTAPP_RUNNER_H

#include <QList>
#include <krunner/abstractrunner.h>

#include "isoftapp-generated.h"

typedef enum {
    STATUS_INSTALL,
    STATUS_UPGRADE,
    STATUS_REMOVE,
    STATUS_INSTALLED,
    STATUS_REMOVED,
    STATUS_UPDATED,
    STATUS_UPGRADED,
    STATUS_SEARCHED,
    STATUS_UNKNOWN,
} Status;

typedef struct {
    QString pkgName;
    int status;
} pkg_t;

class ISoftAppRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    explicit ISoftAppRunner(QObject *parent, const QVariantList &args);
    ~ISoftAppRunner();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action);
    virtual QStringList categories() const;
    virtual QIcon categoryIcon(const QString& category) const;

Q_SIGNALS:
    void searchFinished();

protected Q_SLOTS:
    QMimeData *mimeDataForMatch(const Plasma::QueryMatch &match);

protected:
    void setupMatch(const QString &term, Plasma::QueryMatch &action);

private:
    org::isoftlinux::Isoftapp *isoftapp = Q_NULLPTR;
    QList<pkg_t> pkgs;
    QString servPkgName = QString();
    bool errored = false;

    void enterLoop();
};

#endif
