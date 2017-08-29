# set include path for FindXXX.cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/config/")

find_package(Threads REQUIRED)
include( FindIconv )

if( EXIV2_ENABLE_PNG )
    find_package( ZLIB REQUIRED )
    include_directories( ${ZLIB_INCLUDE_DIR} )
endif( EXIV2_ENABLE_PNG )

if( EXIV2_ENABLE_WEBREADY )
    set( ENABLE_WEBREADY 1 )
    if( EXIV2_ENABLE_CURL )
        find_package( CURL REQUIRED)
    endif()

    if( EXIV2_ENABLE_SSH )
        find_package( SSH REQUIRED)
    endif( EXIV2_ENABLE_SSH )
endif( EXIV2_ENABLE_WEBREADY )

if (EXIV2_ENABLE_XMP)
    find_package(EXPAT REQUIRED)
endif (EXIV2_ENABLE_XMP)

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
    # the manual check in config/generateConfigFile.cmake
endif( EXIV2_ENABLE_NLS )

find_package(Iconv)
if( ICONV_FOUND )
    include_directories(${ICONV_INCLUDE_DIR})
    message ( "-- ICONV_INCLUDE_DIR : " ${ICONV_INCLUDE_DIR} )
    message ( "-- ICONV_LIBRARIES : " ${ICONV_LIBRARIES} )
    message ( "-- ICONV_ACCEPTS_CONST_INPUT : ${ICONV_ACCEPTS_CONST_INPUT}" )
endif( ICONV_FOUND )

if( EXIV2_ENABLE_BUILD_PO )
    find_package(MSGFMT)
    if(MSGFMT_FOUND)
        message(STATUS "Program msgfmt found (${MSGFMT_EXECUTABLE})")
        set( EXIV2_BUILD_PO 1 )
    endif(MSGFMT_FOUND)
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    message(STATUS "Program ccache found")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()
