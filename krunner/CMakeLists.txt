add_definitions(-DTRANSLATION_DOMAIN=\"org.isoftlinux.Isoftapp\")

set(krunner_isoftapp_SRCS
    isoftapprunner.cpp
)

qt5_add_dbus_interface(krunner_isoftapp_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/org.isoftlinux.Isoftapp.xml
    isoftapp-generated
)

add_library(krunner_isoftapp MODULE ${krunner_isoftapp_SRCS})
target_link_libraries(krunner_isoftapp
    Qt5::Widgets
    Qt5::DBus
    KF5::Runner
    KF5::I18n
    KF5::Service
    KF5::KIOWidgets
)

install(TARGETS krunner_isoftapp DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES plasma-runner-isoftapp.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
