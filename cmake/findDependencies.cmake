# set include path for FindXXX.cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Check if the conan file exist to find the dependencies
if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    set(USING_CONAN ON)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(NO_OUTPUT_DIRS KEEP_RPATHS)
endif()

find_package(Threads REQUIRED)

if (UNIX)
    find_package(Regex REQUIRED)
endif()

if( EXIV2_ENABLE_PNG )
    find_package( ZLIB REQUIRED )
    include_directories( ${ZLIB_INCLUDE_DIR} )
endif( )

if( EXIV2_ENABLE_WEBREADY )
    set( ENABLE_WEBREADY 1 )
    if( EXIV2_ENABLE_CURL )
        find_package(CURL REQUIRED)
    endif()

    if( EXIV2_ENABLE_SSH )
        find_package( SSH REQUIRED)
    endif( )
endif( )

if (EXIV2_ENABLE_XMP AND EXIV2_ENABLE_EXTERNAL_XMP)
    message(FATAL_ERROR "EXIV2_ENABLE_XMP AND EXIV2_ENABLE_EXTERNAL_XMP are mutually exclusive.  You can only choose one of them")
else()
    if (EXIV2_ENABLE_XMP)
        find_package(EXPAT REQUIRED)
    elseif (EXIV2_ENABLE_EXTERNAL_XMP)
        find_package(XmpSdk REQUIRED)
    endif ()
endif()


if( EXIV2_ENABLE_NLS )
    find_package(Intl)
    if(Intl_FOUND)
        include_directories(${Intl_INCLUDE_DIRS})
        set(LIBINTL_LIBRARIES ${Intl_LIBRARIES})
    else()
        set(LIBINTL_LIBRARIES)
    endif()
    set( ENABLE_NLS 1 )
    # TODO : This is assuming that Intl is always found. This check should be improved and remove
    # the manual check in cmake/generateConfigFile.cmake
endif( )

find_package(Iconv)
if( ICONV_FOUND )
    message ( "-- ICONV_INCLUDE_DIR : " ${Iconv_INCLUDE_DIR} )
    message ( "-- ICONV_LIBRARIES : " ${Iconv_LIBRARY} )
endif()

if( EXIV2_BUILD_PO )
    find_package(MSGFMT)
    if(MSGFMT_FOUND)
        message(STATUS "Program msgfmt found (${MSGFMT_EXECUTABLE})")
        set( EXIV2_BUILD_PO 1 )
    endif()
endif()

if( BUILD_WITH_CCACHE )
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        message(STATUS "Program ccache found")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif()
endif()

if (EXIV2_BUILD_UNIT_TESTS)
    find_package(GTest REQUIRED)
endif()

# On Windows we are interested in placing the DLLs together to the binaries in the install/bin
# folder, at the installation step. On other platforms we do not care about that, since the 
# RPATHs will point the locations where the libraries where found.
if (USING_CONAN AND WIN32)
    install(DIRECTORY ${PROJECT_BINARY_DIR}/conanDlls/ DESTINATION bin)
endif()
