cmake_minimum_required(VERSION 2.8.8)

project(isoftapp)

# Set version
set(ISOFTAPP_VERSION_MAJOR 2)
set(ISOFTAPP_VERSION_MINOR 2)
set(ISOFTAPP_VERSION_PATCH 7)
set(ISOFTAPP_VERSION_STRING "${ISOFTAPP_VERSION_MAJOR}.${ISOFTAPP_VERSION_MINOR}.${ISOFTAPP_VERSION_PATCH}")
set(PROJECT_VERSION ISOFTAPP_VERSION_STRING)
add_definitions("-DISOFTAPP_VERSION_STRING=\"${ISOFTAPP_VERSION_STRING}\"")
add_definitions("-DPROJECT_VERSION=\"${PROJECT_VERSION}\"")

set(GETTEXT_PACKAGE "org.isoftlinux.Isoftapp")

# Definitions
set(DATADIR ${CMAKE_INSTALL_PREFIX}/share)
add_definitions(-std=c++11 -fpermissive)
add_definitions(-DHAVE_TIMEGM)
add_definitions("-DGETTEXT_PACKAGE=\"${GETTEXT_PACKAGE}\"")
add_definitions("-DDATADIR=\"${DATADIR}\"")

option(ENABLE_NOSIZY "Enable nosizy debug" OFF)
if(ENABLE_NOSIZY)
    add_definitions(-DNOSIZY)
endif()

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Handle build type
if(CMAKE_BUILD_TYPE MATCHES [Dd]ebug)
    message(STATUS "Debug build")
    add_definitions(-DDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
else()
    message(STATUS "Release build")
    add_definitions(-DNDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

# Default absolute paths
if(NOT DEFINED CMAKE_INSTALL_SYSCONFDIR)
    set(CMAKE_INSTALL_SYSCONFDIR "/etc")
endif()
if(NOT DEFINED CMAKE_INSTALL_LOCALSTATEDIR)
    set(CMAKE_INSTALL_LOCALSTATEDIR "/var")
endif()

set(ISOFTAPP_CONF_DIR ${CMAKE_INSTALL_SYSCONFDIR}/isoftapp)
add_definitions("-DISOFTAPP_CONF_DIR=\"${ISOFTAPP_CONF_DIR}\"")

set(ISOFTAPP_CACHE_DIR ${CMAKE_INSTALL_LOCALSTATEDIR}/cache/isoftapp)
add_definitions("-DISOFTAPP_CACHE_DIR=\"${ISOFTAPP_CACHE_DIR}\"")

set(ISOFTAPP_DATA_DIR ${CMAKE_INSTALL_PREFIX}/share/isoftapp)
add_definitions("-DISOFTAPP_DATA_DIR=\"${ISOFTAPP_DATA_DIR}\"")

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# PKG-CONFIG
find_package(PkgConfig)

find_program(GDBUS_CODEGEN_EXECUTABLE NAMES gdbus-codegen DOC "gdbus-codegen executable")
if(NOT GDBUS_CODEGEN_EXECUTABLE)
    message(FATAL_ERROR "Executable gdbus-codegen not found")
endif()


if(NOT NO_SYSTEMD)
    pkg_check_modules(SYSTEMD "systemd")
endif()

if(SYSTEMD_FOUND)
    add_definitions(-DHAVE_SYSTEMD)
    set(CMAKE_AUTOMOC_MOC_OPTIONS -DHAVE_SYSTEMD)

    if (NOT DEFINED SYSTEMD_SYSTEM_UNIT_DIR)
        execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=systemdsystemunitdir systemd OUTPUT_VARIABLE SYSTEMD_SYSTEM_UNIT_DIR)
        string(REGEX REPLACE "[ \t\n]+" "" SYSTEMD_SYSTEM_UNIT_DIR ${SYSTEMD_SYSTEM_UNIT_DIR})
    endif()
endif()


pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO2 REQUIRED gio-2.0)
pkg_check_modules(GIOUNIX REQUIRED gio-unix-2.0)
pkg_check_modules(RPM REQUIRED rpm)
pkg_check_modules(POPT REQUIRED popt)
pkg_check_modules(LIBURIPARSER REQUIRED liburiparser)
pkg_check_modules(LIBCURL REQUIRED libcurl)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
pkg_check_modules(NM REQUIRED NetworkManager)

find_package(Qt5 REQUIRED COMPONENTS DBus Widgets Core)
find_package(qtsingleapplication REQUIRED)
find_package(KF5 REQUIRED COMPONENTS Runner I18n Service KDELibs4Support)

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMPackageConfigHelpers)

# Add subdirectories
add_subdirectory(data)
add_subdirectory(src)
add_subdirectory(systray)
add_subdirectory(console)
add_subdirectory(krunner)
add_subdirectory(po)
add_subdirectory(qjade)

# Display feature summary
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
