find_library(QtSingleApplication_LIBRARIES NAMES Qt5Solutions_SingleApplication-2.6)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(qtsingleapplication REQUIRED_VARS 
                                  QtSingleApplication_LIBRARIES)
