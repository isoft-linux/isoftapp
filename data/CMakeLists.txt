install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/org.isoftlinux.Isoftapp.xml" DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/dbus-1/interfaces")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/org.isoftlinux.Isoftapp.conf" DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/dbus-1/system.d")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/org.isoftlinux.Isoftapp.service.in" "${CMAKE_CURRENT_BINARY_DIR}/org.isoftlinux.Isoftapp.service")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/org.isoftlinux.Isoftapp.service" DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/dbus-1/system-services")

if(SYSTEMD_FOUND)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/isoftapp-daemon.service.in" "${CMAKE_CURRENT_BINARY_DIR}/isoftapp-daemon.service")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/isoftapp-daemon.service" DESTINATION "${SYSTEMD_SYSTEM_UNIT_DIR}")
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/default.conf.example" DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/isoftapp")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other.conf.example" DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/isoftapp/config.d/")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/pkgcache.db" DESTINATION ${ISOFTAPP_DATA_DIR})
