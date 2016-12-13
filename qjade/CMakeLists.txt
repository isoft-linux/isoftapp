
add_definitions(-DTARGET=\"qjade\" -DBRAND=\"isoft\")

find_package(KF5CoreAddons REQUIRED)
find_package(KF5XmlGui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

qt5_add_resources(qjade_RC_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc)

set(qjade_SRCS
    src/brand.cpp
    src/categorymodel.cpp
    src/filedownloader.cpp
    src/httpget.cpp
    src/i18n.cpp
    src/iconmodel.cpp
    src/jadedbus.cpp
    src/main.cpp
    src/packageinfomodel.h
    src/appinfo.h
    src/packagebycategorymodel.cpp
    src/mypkgmodel.cpp
    src/process.cpp
    src/runguard.cpp
    src/searchmodel.cpp
    src/slideshowmodel.cpp
    src/util.cpp
    ${qjade_RC_SRCS}
    )

qt5_add_dbus_interface(qjade_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/org.isoftlinux.Isoftapp.xml
    isoftapp-generated
)

add_executable(qjade ${qjade_SRCS})

target_link_libraries(qjade
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::DBus
    Qt5::Network
    Qt5::Xml
    Qt5::Widgets
    KF5::I18n
    KF5::Service
)

#install -m 755 -p qjade /usr/bin/qjade
#install -m 644 -p /home/test/gits/isoftapp/qjade/qjade.desktop /usr/share/applications/
#install -m 644 -p /home/test/gits/isoftapp/qjade/brand/isoft.xml /usr/share/qjade/
#install -m 644 -p /home/test/gits/isoftapp/qjade/brand/isoft.png /usr/share/qjade/

install(TARGETS qjade RUNTIME DESTINATION bin)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/qjade.isoft.desktop DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/xdg/autostart)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/qjade.isoft.desktop DESTINATION  ${DATADIR}/applications)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/brand/isoft.xml DESTINATION ${DATADIR}/qjade)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/brand/isoft.png DESTINATION ${DATADIR}/qjade)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/brand/title.png DESTINATION ${DATADIR}/qjade)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/brand/isoft.png DESTINATION ${DATADIR}/icons/oxygen/32x32/apps)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/brand/qjade.png DESTINATION ${DATADIR}/icons/oxygen/32x32/apps)
##install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/qml DESTINATION ${DATADIR}/qjade)
##install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/images DESTINATION ${DATADIR}/qjade)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/qjade_zh_CN.qm DESTINATION /usr/share/qt5/translations)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/backendlist.txt DESTINATION ${DATADIR}/qjade)
