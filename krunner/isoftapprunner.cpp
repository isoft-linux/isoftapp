/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2014 Vishesh Handa <vhanda@kde.org>
 *   Copyright (C) 2016 Cjacker <cjacker@foxmail.com>
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

#include "isoftapprunner.h"

#include <iostream>
#include <QMimeData>
#include <QIcon>
#include <QDebug>
#include <QUrl>
#include <QMessageBox>
#include <QEventLoop>
#include <KLocalizedString>
#include <KRun>
#include <KService>

ISoftAppRunner::ISoftAppRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)

    isoftapp = new org::isoftlinux::Isoftapp("org.isoftlinux.Isoftapp", 
                                             "/org/isoftlinux/Isoftapp", 
                                             QDBusConnection::systemBus(), 
                                             this);
    connect(isoftapp, &org::isoftlinux::Isoftapp::Error, 
            [this](qlonglong error, const QString &details, qlonglong errcode) {
        errored = true;
    });
    connect(isoftapp, &org::isoftlinux::Isoftapp::Finished, 
            [this](const QString &pkgName,qlonglong status) {
        if (status == STATUS_SEARCHED) {
            Q_EMIT searchFinished();
        } else if (status == STATUS_INSTALLED) {
            QString desktopName = isoftapp->GetDesktopName(servPkgName).value();
            if (!desktopName.isEmpty())
                desktopName = desktopName.left(desktopName.size() - 8);
            KService::Ptr service = KService::serviceByDesktopName(desktopName);
            if (!service) {
                service = KService::serviceByDesktopName(servPkgName);
                if (!service)
                    return;
            }
            if (!service->exec().isEmpty() && !service->noDisplay()) {
                QFile::link(service->entryPath(), 
                    QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + 
                    "/" + service->name() + ".desktop");
                QMessageBox box(QMessageBox::Question, i18n("iSoftApp"), 
                    i18n("Do you want to run %1?", service->name()), 
                    QMessageBox::Open | QMessageBox::Cancel);
                box.setDefaultButton(QMessageBox::Cancel);
                box.setWindowIcon(QIcon::fromTheme(service->icon()));
                if (box.exec() == QMessageBox::Open)
                    KRun::run(*service, QList<QUrl>(), 0);
            }
        }
    });
    connect(isoftapp, &org::isoftlinux::Isoftapp::SearchChanged, 
            [this](const QString &pkgName, qlonglong status) {
        Q_FOREACH (const pkg_t &pkg, pkgs) {
            if (pkg.pkgName == pkgName)
                return;
        }
        pkg_t pkg;
        pkg.pkgName = pkgName;
        pkg.status = status;
        pkgs << pkg;
    });

    setObjectName(QLatin1String("iSoftApp"));
    setPriority(AbstractRunner::HighestPriority);

    addSyntax(Plasma::RunnerSyntax(":q:", i18n("Find App matches :q:")));
}

ISoftAppRunner::~ISoftAppRunner()
{
    if (isoftapp) {
        delete isoftapp;
        isoftapp = Q_NULLPTR;
    }
}

QStringList ISoftAppRunner::categories() const
{
    QStringList cat;
    cat << i18n("iSoftApp");

    return cat;
}

QIcon ISoftAppRunner::categoryIcon(const QString& category) const
{
    if (category == i18n("iSoftApp"))
        return QIcon::fromTheme("security-high");

    return Plasma::AbstractRunner::categoryIcon(category);
}

void ISoftAppRunner::enterLoop() 
{
    QEventLoop eventLoop;
    connect(this, SIGNAL(searchFinished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void ISoftAppRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    QList<Plasma::QueryMatch> matches;

    if (term.length() <= 4) {
        QEventLoop loop;
        QTimer timer;
        connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.setSingleShot(true);
        timer.start(100);
        loop.exec();

        if (!context.isValid()) {
            return;
        }
    }

    errored = false;
    pkgs.clear();
    isoftapp->Search(term);
    if (errored) {
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setText(i18n("daemon is not ready! please wait..."));
        match.setData(-1);
        match.setIcon(QIcon::fromTheme("clock"));
        if (context.isValid())
            context.addMatch(match);
    }
    enterLoop();

    float relevance = 1.0;

    Q_FOREACH (const pkg_t &pkg, pkgs) {
        if (pkg.pkgName.contains(term, Qt::CaseInsensitive)) {
            Plasma::QueryMatch match(this);
            //Match type help sorting, ExactMatch will always on top no matter it's installed or not.
            if (pkg.pkgName.toUpper() == term.toUpper())
                match.setType(Plasma::QueryMatch::ExactMatch);
            else
                match.setType(Plasma::QueryMatch::HelperMatch);
            match.setText((pkg.status ? i18n("Install") : i18n("Uninstall")) + " " + pkg.pkgName);
            match.setData(pkg.status);
            match.setSubtext(pkg.pkgName);
            match.setIcon(pkg.status ? QIcon::fromTheme("list-add") : QIcon::fromTheme("list-remove"));
            //Relevance will help sorting after sorted by Match type.
            //Here keeping uninstalled on top, and installed below.
            //a range of 50 should be enough for display.
            match.setRelevance(pkg.status ? relevance : relevance - 0.5);
            relevance -= 0.01;
            matches << match;
        }
    }

    if (context.isValid())
        context.addMatches(matches);
}

void ISoftAppRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context);
    if (match.data().toInt() == 1) {
        servPkgName = match.subtext();
        isoftapp->Install(match.subtext());
    } else if (match.data().toInt() == 0) {
        servPkgName = match.subtext();
        QString desktopName = isoftapp->GetDesktopName(servPkgName).value();
        if (!desktopName.isEmpty())
            desktopName = desktopName.left(desktopName.size() - 8);
        KService::Ptr service = KService::serviceByDesktopName(desktopName);
        if (!service)
            service = KService::serviceByDesktopName(servPkgName);
        if (service) {
            QFile::remove(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + 
                "/" + service->name() + ".desktop");
        }
		isoftapp->Remove(match.subtext(), false);
    }
}

QMimeData *ISoftAppRunner::mimeDataForMatch(const Plasma::QueryMatch &match) 
{
    QMimeData *result = new QMimeData();
    QList<QUrl> urls;
    urls << QUrl("isoftapp://" + match.subtext());
    result->setUrls(urls);
    return result;
}

K_EXPORT_PLASMA_RUNNER(isoftapp, ISoftAppRunner)

#include "isoftapprunner.moc"
