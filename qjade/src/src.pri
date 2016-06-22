DBUS_INTERFACES +=  \
    $$PWD/../../data/org.isoftlinux.Isoftapp.xml

DBUS_INTERFACES +=  \
    $$PWD/jaded.xml

HEADERS += \
    $$PWD/globaldeclarations.h  \
    $$PWD/runguard.h    \
    $$PWD/brand.h   \
    $$PWD/i18n.h    \
    $$PWD/appinfo.h \
    $$PWD/httpget.h \
    $$PWD/categorymodel.h   \
    $$PWD/slideshowmodel.h  \
    $$PWD/iconmodel.h   \
    $$PWD/util.h    \
    $$PWD/packagebycategorymodel.h  \
    $$PWD/mypkgmodel.h  \
    $$PWD/filedownloader.h  \
    $$PWD/process.h \
    $$PWD/packageinfomodel.h    \
    $$PWD/jadedbus.h    \
    $$PWD/searchmodel.h

SOURCES += \
    $$PWD/main.cpp  \
    $$PWD/runguard.cpp  \
    $$PWD/brand.cpp \
    $$PWD/i18n.cpp  \
    $$PWD/httpget.cpp   \
    $$PWD/categorymodel.cpp \
    $$PWD/slideshowmodel.cpp    \
    $$PWD/iconmodel.cpp \
    $$PWD/util.cpp  \
    $$PWD/packagebycategorymodel.cpp    \
    $$PWD/mypkgmodel.cpp    \
    $$PWD/filedownloader.cpp    \
    $$PWD/process.cpp   \
    $$PWD/jadedbus.cpp  \
    $$PWD/searchmodel.cpp
