include_directories(
    ${GLIB2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARRY_DIRECTORY}
)

set(isoftapp_systray_SRCS
    main.cpp
    systray.cpp
    progresswidget.cpp
    upgradewidget.cpp
)

qt5_add_dbus_interface(isoftapp_systray_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/org.isoftlinux.Isoftapp.xml
    isoftapp-generated
)

add_executable(isoftapp_systray ${isoftapp_systray_SRCS})

target_link_libraries(isoftapp_systray
    Qt5::DBus
    Qt5::Widgets
    ${QtSingleApplication_LIBRARIES}
    ${GLIB2_LIBRARIES}
)

install(TARGETS isoftapp_systray RUNTIME DESTINATION bin)
install(FILES isoftapp_systray.desktop DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/xdg/autostart)
