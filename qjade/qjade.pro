QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

lessThan(QT_VER_MAJ, 5) | lessThan(QT_VER_MIN, 2) {
	error(QJade is only tested under Qt 5.2!)
}

QT += qml quick network svg xml dbus
!android: !ios: !blackberry: qtHaveModule(widgets): QT += widgets
TARGET = qjade
TRANSLATIONS = translations/$${TARGET}_zh_CN.ts
CODECFORSRC = UTF-8

include(src/src.pri)

OTHER_FILES += \
    qml/main.qml \
    qml/DragArea.qml    \
    qml/MyToolButton.qml \
    qml/StoreView.qml \
    qml/HotTodayView.qml  \
    qml/PackageByCategoryView.qml   \
    qml/UpdateView.qml \
    qml/UninstallView.qml   \
    qml/SearchView.qml  \
    qml/AboutView.qml   \
    qml/MyLoader.qml    \
    qml/MyWindowButton.qml  \
    qml/SlideShow.qml   \
    qml/PackageInfoView.qml \
    qml/TaskQueueView.qml   \
    qml/global.js

RESOURCES += \
    resources.qrc

!isEmpty(QJADE_DEBUG) {                                                             
    DEFINES += QJADE_DEBUG                                                          
}

unix {
    #VARIABLES
    isEmpty(PREFIX) {
        PREFIX = /usr
    }
    isEmpty(BRAND) {
        BRAND = isoft
    }
    BINDIR = $$PREFIX/bin
    DATADIR = $$PREFIX/share

    DEFINES += PREFIX=\\\"$$PREFIX\\\"
    DEFINES += TARGET=\\\"$$TARGET\\\"
    DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"
    DEFINES += BRAND=\\\"$$BRAND\\\"

    #MAKE INSTALL

    INSTALLS += target desktop brand

    target.path = $$BINDIR

    desktop.path = $$DATADIR/applications
    desktop.files += $${TARGET}.desktop

    icon.path = $$DATADIR/icons
    icon.files += images/$${TARGET}.png

    brand.path = $$DATADIR/$$TARGET
    brand.files += brand/$${BRAND}.xml brand/$${BRAND}.png
}
