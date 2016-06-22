include_directories(
    ${GLIB2_INCLUDE_DIRS} 
    ${GIO2_INCLUDE_DIRS}
    ${GIOUNIX_INCLUDE_DIRS}
    ${RPM_INCLUDE_DIRS}
    ${POPT_INCLUDE_DIRS}
    ${LIBURIPARSER_INCLUDE_DIRS}
    ${LIBCURL_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${NM_INCLUDE_DIRS}
)

set(genbase_SRCS
    error.cpp
    tagfile.cpp
    configuration.cpp
    cached_md5.cpp
    rhash.cpp
    strutl.cpp
    fileutl.cpp
)

add_executable(isoft-genpkglist 
    genpkglist.cpp
    ${genbase_SRCS}
)

target_link_libraries(isoft-genpkglist
    ${GLIB2_LIBRARIES}
    ${GIO2_LIBRARIES}
    ${RPM_LIBRARIES}
    ${POPT_LIBRARIES}
    ${NM_LIBRARIES}
)

install(TARGETS isoft-genpkglist RUNTIME DESTINATION bin)

add_executable(isoft-gensrclist
    gensrclist.cpp
    ${genbase_SRCS}
)

target_link_libraries(isoft-gensrclist
    ${GLIB2_LIBRARIES}
    ${GIO2_LIBRARIES}
    ${RPM_LIBRARIES}
    ${POPT_LIBRARIES}
)

install(TARGETS isoft-gensrclist RUNTIME DESTINATION bin)

add_executable(isoft-countpkglist
    countpkglist.cpp
)

target_link_libraries(isoft-countpkglist
    ${RPM_LIBRARIES}
)

install(TARGETS isoft-countpkglist RUNTIME DESTINATION bin)

install(PROGRAMS isoft-genbasedir DESTINATION bin)

execute_process(COMMAND ${GDBUS_CODEGEN_EXECUTABLE} --generate-c-code
    ${CMAKE_CURRENT_BINARY_DIR}/isoftapp-generated --c-namespace Gdbus --interface-prefix org.isoftlinux.
                        ${CMAKE_CURRENT_SOURCE_DIR}/../data/org.isoftlinux.Isoftapp.xml
                        RESULT_VARIABLE codegen_failed)
if(codegen_failed)
    message(FATAL_ERROR "Error in generating code for isoftapp-generated using gdbus-codegen")
endif()

add_executable(isoftapp-daemon 
    isoftapp.cpp
    download.cpp
    rpmpm.cpp
    conf-helper.cpp
    splitstring.cpp
    daemon.cpp
    md5.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/isoftapp-generated.c
)

target_link_libraries(isoftapp-daemon 
    ${GLIB2_LIBRARIES}
    ${GIO2_LIBRARIES}
    ${RPM_LIBRARIES}
    ${POPT_LIBRARIES}
    ${LIBURIPARSER_LIBRARIES}
    ${LIBCURL_LIBRARIES}
    ${SQLITE3_LIBRARIES}
)

install(TARGETS isoftapp-daemon RUNTIME DESTINATION bin)
