# CMake build system for exiv2 library and executables
# Copyright 2015-     Daniel Kaneider  <danielkaneider@users.sf.net>
# Copyright 2010-2012 Gilles Caulier   <caulier dot gilles at gmail dot com>
# Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>

# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if ( NOT MSVC )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# Private headers which are only needed for the library itself
SET( LIBEXIV2_PRIVATE_HDR canonmn_int.hpp
                          casiomn_int.hpp
                          cr2image_int.hpp
                          crwimage_int.hpp
                          fujimn_int.hpp
                          makernote_int.hpp
                          minoltamn_int.hpp
                          nikonmn_int.hpp
                          olympusmn_int.hpp
                          orfimage_int.hpp
                          panasonicmn_int.hpp
                          pentaxmn_int.hpp
                          pngchunk_int.hpp
                          rcsid_int.hpp
                          rw2image_int.hpp
                          samsungmn_int.hpp
                          sigmamn_int.hpp
                          sonymn_int.hpp
                          tags_int.hpp
                          tiffcomposite_int.hpp
                          tifffwd_int.hpp
                          tiffimage_int.hpp
                          tiffvisitor_int.hpp
   )

# Add standalone C++ header files to this list
SET( LIBEXIV2_HDR         ../include/exiv2/asfvideo.hpp
                          ../include/exiv2/basicio.hpp
                          ../include/exiv2/bmpimage.hpp
                          ../include/exiv2/config.h
                          ../include/exiv2/convert.hpp
                          ../include/exiv2/cr2image.hpp
                          ../include/exiv2/crwimage.hpp
                          ../include/exiv2/datasets.hpp
                          ../include/exiv2/easyaccess.hpp
                          ../include/exiv2/epsimage.hpp
                          ../include/exiv2/error.hpp
                          ../include/exiv2/exif.hpp
                          ../include/exiv2/exiv2.hpp
                          ../include/exiv2/futils.hpp
                          ../include/exiv2/gifimage.hpp
                          ../include/exiv2/http.hpp
                          ../include/exiv2/image.hpp
                          ../include/exiv2/ini.hpp
                          ../include/exiv2/iptc.hpp
                          ../include/exiv2/jp2image.hpp
                          ../include/exiv2/jpgimage.hpp
                          ../include/exiv2/matroskavideo.hpp
                          ../include/exiv2/metadatum.hpp
                          ../include/exiv2/mrwimage.hpp
                          ../include/exiv2/orfimage.hpp
                          ../include/exiv2/pgfimage.hpp
                          ../include/exiv2/preview.hpp
                          ../include/exiv2/properties.hpp
                          ../include/exiv2/psdimage.hpp
                          ../include/exiv2/quicktimevideo.hpp
                          ../include/exiv2/rafimage.hpp
                          ../include/exiv2/riffvideo.hpp
                          ../include/exiv2/rwlock.hpp
                          ../include/exiv2/rw2image.hpp
                          ../include/exiv2/tags.hpp
                          ../include/exiv2/tgaimage.hpp
                          ../include/exiv2/tiffimage.hpp
                          ../include/exiv2/types.hpp
                          ../include/exiv2/utilsvideo.hpp
                          ../include/exiv2/value.hpp
                          ../include/exiv2/version.hpp
                          ../include/exiv2/webpimage.hpp
                          ../include/exiv2/xmp_exiv2.hpp
                          ../include/exiv2/xmpsidecar.hpp
                          ../include/exiv2/utilsvideo.hpp
                          ${CMAKE_BINARY_DIR}/exv_conf.h
   )

# Add library C++ source files to this list
SET( LIBEXIV2_SRC         asfvideo.cpp
                          basicio.cpp
                          bmpimage.cpp
                          canonmn.cpp
                          casiomn.cpp
                          convert.cpp
                          cr2image.cpp
                          crwedit.cpp
                          crwimage.cpp
                          datasets.cpp
                          easyaccess.cpp
                          epsimage.cpp
                          error.cpp
                          exif.cpp
                          futils.cpp
                          fujimn.cpp
                          gifimage.cpp
                          http.cpp
                          image.cpp
                          ini.cpp
                          iptc.cpp
                          jp2image.cpp
                          jpgimage.cpp
                          makernote.cpp
                          matroskavideo.cpp
                          metadatum.cpp
                          minoltamn.cpp
                          mrwimage.cpp
                          nikonmn.cpp
                          olympusmn.cpp
                          orfimage.cpp
                          panasonicmn.cpp
                          pentaxmn.cpp
                          pgfimage.cpp
                          preview.cpp
                          properties.cpp
                          psdimage.cpp
                          quicktimevideo.cpp
                          rafimage.cpp
                          riffvideo.cpp
                          rw2image.cpp
                          samsungmn.cpp
                          sigmamn.cpp
                          sonymn.cpp
                          tags.cpp
                          tgaimage.cpp
                          tiffcomposite.cpp
                          tiffimage.cpp
                          tiffvisitor.cpp
                          types.cpp
                          utilsvideo.cpp
                          value.cpp
                          version.cpp
                          webpimage.cpp
                          xmp.cpp
                          xmpsidecar.cpp
                          utilsvideo.cpp
   )

if( EXIV2_ENABLE_WEBREADY )
    if( EXIV2_ENABLE_CURL)
        set( LIBEXIV2_SRC ${LIBEXIV2_SRC} easyaccess.cpp)
        set( LIBEXIV2_HDR ${LIBEXIV2_HDR} ../include/exiv2/easyaccess.hpp)
    endif()

    if( EXIV2_ENABLE_SSH )
        set( LIBEXIV2_SRC ${LIBEXIV2_SRC} ssh.cpp)
        set( LIBEXIV2_HDR ${LIBEXIV2_HDR} ../include/exiv2/ssh.hpp)
    endif()
endif( EXIV2_ENABLE_WEBREADY )

##
# add png*.cpp files if PNG support requested
IF( EXIV2_ENABLE_PNG )
    IF( ZLIB_FOUND )
        SET( LIBEXIV2_SRC ${LIBEXIV2_SRC}
                          pngchunk.cpp
                          pngimage.cpp
           )
        set( LIBEXIV2_HDR ${LIBEXIV2_HDR} ../include/exiv2/pngimage.hpp )
    ENDIF( ZLIB_FOUND )
ENDIF( EXIV2_ENABLE_PNG )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# ******************************************************************************
# exiv2lib library

if( EXIV2_ENABLE_XMP AND NOT EXIV2_ENABLE_LIBXMP )
    add_library( exiv2lib ${STATIC_FLAG} ${LIBEXIV2_SRC} ${LIBEXIV2_HDR} ${LIBEXIV2_PRIVATE_HDR} $<TARGET_OBJECTS:xmp_object>)
    target_link_libraries(exiv2lib PUBLIC ${EXPAT_LIBRARIES})
else()
    add_library( exiv2lib ${STATIC_FLAG} ${LIBEXIV2_SRC} ${LIBEXIV2_HDR} ${LIBEXIV2_PRIVATE_HDR})
endif()

SET_TARGET_PROPERTIES( exiv2lib PROPERTIES
    VERSION       ${GENERIC_LIB_VERSION}
    SOVERSION     ${GENERIC_LIB_SOVERSION}
    DEFINE_SYMBOL EXV_BUILDING_LIB
    OUTPUT_NAME   exiv2
)

if ( UNIX AND CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" )
    set (FREEBSD 1)
endif()

if ( EXIV2_ENABLE_XMP )
    target_include_directories(exiv2lib PRIVATE ${CMAKE_SOURCE_DIR}/xmpsdk/include)
endif ()

# TODO : We should not include include/exiv2 but only include !!!
target_include_directories(exiv2lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/exiv2>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

if (EXIV2_ENABLE_WEBREADY AND EXIV2_ENABLE_CURL)
    target_include_directories(exiv2lib SYSTEM PUBLIC ${CURL_INCLUDE_DIR} )
endif()

if (EXIV2_ENABLE_WEBREADY AND EXIV2_ENABLE_SSH)
    target_include_directories(exiv2lib SYSTEM PUBLIC ${SSH_INCLUDE_DIR} )
endif()

if ( MSVC )
    target_compile_definitions(exiv2lib PRIVATE PSAPI_VERSION=1) # to be compatible with <= WinVista (#905)

    if ( EXIV2_ENABLE_STATIC )
        TARGET_LINK_LIBRARIES( exiv2lib zlibstatic ${ZLIB_LIBRARIES} )
    else()
        TARGET_LINK_LIBRARIES( exiv2lib PRIVATE ${ZLIB_LIBRARIES} )
    endif()
    source_group("Header Files" FILES ${LIBEXIV2_HDR} )
    source_group("Header Files" FILES ${LIBCURL_HDR} )
    source_group("Header Files" FILES ${SSH_HDR} )
    target_link_libraries( exiv2lib PRIVATE ${ZLIB_LIBRARIES} ${CURL_LIBRARIES} ${SSH_LIBRARIES})
else()
    # TODO: Check if this is really needed.
    if ( UNIX AND NOT FREEBSD )
        target_link_libraries( exiv2lib PRIVATE dl)
    endif()
    target_link_libraries( exiv2lib PRIVATE Threads::Threads)
    target_link_libraries( exiv2lib PUBLIC ${CURL_LIBRARIES} ${SSH_LIBRARIES})
endif()

if( EXIV2_ENABLE_XMP AND EXIV2_ENABLE_LIBXMP )
    TARGET_LINK_LIBRARIES( exiv2lib PUBLIC xmp )
ENDIF()

IF( EXIV2_ENABLE_PNG )
    IF( ZLIB_FOUND )
        IF( MSVC )
            TARGET_LINK_LIBRARIES( exiv2lib PRIVATE optimized ${ZLIB_LIBRARIES} )
        ELSE()
            TARGET_LINK_LIBRARIES( exiv2lib PRIVATE ${ZLIB_LIBRARIES} )
        ENDIF()
    ENDIF()
ENDIF()

IF( EXIV2_ENABLE_NLS )
    TARGET_LINK_LIBRARIES( exiv2lib PRIVATE  ${LIBINTL_LIBRARIES} )
ENDIF( EXIV2_ENABLE_NLS )

IF( ICONV_FOUND )
    TARGET_LINK_LIBRARIES( exiv2lib PRIVATE  ${ICONV_LIBRARIES} )
ENDIF( ICONV_FOUND )

IF (CYGWIN OR MINGW)
    TARGET_LINK_LIBRARIES( exiv2lib PRIVATE  psapi ws2_32 )
ENDIF(CYGWIN OR MINGW)

install(TARGETS exiv2lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES ${LIBEXIV2_HDR}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/exiv2
)

# ******************************************************************************
# exiv2 application

set( EXIV2_SRC  exiv2.cpp
                exiv2app.hpp
                actions.cpp
                actions.hpp
                utils.cpp
                utils.hpp
)

if(EXIV2_ENABLE_TOOLS)
    add_executable( exiv2 ${EXIV2_SRC} )
    target_link_libraries( exiv2 PRIVATE exiv2lib )
    install(TARGETS exiv2 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# TODO This should not be needed here! we need to fix the previous TODO
target_include_directories(exiv2 PRIVATE ${CMAKE_SOURCE_DIR}/include/)

# modify source lists to suit environment
if(NOT EXV_HAVE_TIMEGM )
    target_sources(exiv2lib PRIVATE localtime.c)
    target_sources(exiv2    PRIVATE localtime.c)
endif()

if (MSVC)
    target_sources(exiv2lib PRIVATE getopt_win32.c)
    target_sources(exiv2    PRIVATE getopt_win32.c)
endif()

# ******************************************************************************
# Man page
INSTALL( FILES exiv2.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )
