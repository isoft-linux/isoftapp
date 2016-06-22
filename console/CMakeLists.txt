include_directories(
    ${GLIB2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARRY_DIRECTORY}
)

set(isoftapp_SRCS
    main.cpp
)

qt5_add_dbus_interface(isoftapp_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/org.isoftlinux.Isoftapp.xml
    isoftapp-generated
)

add_executable(isoftapp ${isoftapp_SRCS})

target_link_libraries(isoftapp
    Qt5::DBus
    Qt5::Core
    ${GLIB2_LIBRARIES}
)

install(TARGETS isoftapp RUNTIME DESTINATION bin)
