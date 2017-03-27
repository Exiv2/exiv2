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
SET( LIBEXIV2_HDR         ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/asfvideo.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/basicio.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/bmpimage.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/config.h
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/convert.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/cr2image.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/crwimage.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/datasets.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/easyaccess.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/epsimage.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/error.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/exif.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/exiv2.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/futils.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/gifimage.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/http.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/image.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/ini.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/iptc.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/jp2image.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/jpgimage.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/matroskavideo.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/metadatum.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/mrwimage.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/orfimage.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/pgfimage.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/preview.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/properties.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/psdimage.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/quicktimevideo.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/rafimage.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/riffvideo.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/rwlock.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/rw2image.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/tags.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/tgaimage.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/tiffimage.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/types.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/utilsvideo.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/value.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/version.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/webpimage.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/xmp.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/xmpsidecar.hpp
                          ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/utilsvideo.hpp
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

IF( EXIV2_ENABLE_WEBREADY )
    ##
    # add curl support
    IF( EXIV2_ENABLE_CURL )
        IF( CURL_FOUND )
            SET( LIBEXIV2_SRC ${LIBEXIV2_SRC}
                              easyaccess.cpp
               )
            set( LIBEXIV2_HDR ${LIBEXIV2_HDR}
               ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/easyaccess.hpp
            )
        ENDIF()
    ENDIF()

    ##
    # add ssh support
    IF( EXIV2_ENABLE_SSH )
        IF( SSH_FOUND )
            SET( LIBEXIV2_SRC ${LIBEXIV2_SRC}
                              ssh.cpp
               )
            set( LIBEXIV2_HDR ${LIBEXIV2_HDR}
            ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/ssh.hpp
            )
        ENDIF()
    ENDIF()
ENDIF( EXIV2_ENABLE_WEBREADY )

##
# add png*.cpp files if PNG support requested
IF( EXIV2_ENABLE_PNG )
    IF( ZLIB_FOUND )
        SET( LIBEXIV2_SRC ${LIBEXIV2_SRC}
                          pngchunk.cpp
                          pngimage.cpp
           )
        set( LIBEXIV2_HDR ${LIBEXIV2_HDR} ${CMAKE_CURRENT_LIST_DIR}/../include/exiv2/pngimage.hpp )
    ENDIF( ZLIB_FOUND )
ENDIF( EXIV2_ENABLE_PNG )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

##
# add xmp source if xmp is requested and xmp library not built
IF( EXIV2_ENABLE_XMP )
    IF( NOT EXIV2_ENABLE_LIBXMP )
        SET( LIBEXIV2_SRC ${XMPSRC} ${LIBEXIV2_SRC} )
    ENDIF( NOT EXIV2_ENABLE_LIBXMP )
ENDIF( EXIV2_ENABLE_XMP )

##
# Create source file lists for applications
# exiv2 application
SET( EXIV2_SRC            exiv2.cpp
                          actions.cpp
                          utils.cpp
)
SET( EXIV2_HDR            exiv2app.hpp
                          actions.hpp
                          utils.hpp
)


# connection test application
SET( CONNTEST             ../samples/conntest.cpp)
# exifprint application
SET( EXIFPRINT            ../samples/exifprint.cpp)
# cmdfiletest application
SET( REMOTETEST           ../samples/remotetest.cpp)

##
# modify source lists to suit environment
IF(NOT HAVE_TIMEGM )
    SET( LIBEXIV2_SRC     ${LIBEXIV2_SRC} localtime.c    )
    SET( EXIV2_SRC        ${EXIV2_SRC}    localtime.c    )
    SET( PATHTEST_SRC     ${PATHTEST_SRC} localtime.c    )
ENDIF( NOT HAVE_TIMEGM )

IF( MSVC )
    SET( EXIV2_SRC        ${EXIV2_SRC}    getopt_win32.c )
    SET( LIBEXIV2_SRC     ${LIBEXIV2_SRC} getopt_win32.c )
ENDIF( MSVC )

##
# msvn tuning
IF( MSVC )
    include(../CMake_msvc.txt)
    msvc_runtime_configure(${EXIV2_ENABLE_SHARED} ${EXIV2_ENABLE_DYNAMIC_RUNTIME})
ENDIF( MSVC )

# ******************************************************************************
# exiv2lib library

ADD_LIBRARY( exiv2lib ${STATIC_FLAG} ${LIBEXIV2_SRC} ${LIBEXIV2_HDR} ${LIBEXIV2_PRIVATE_HDR} )

SET_TARGET_PROPERTIES( exiv2lib PROPERTIES
    VERSION       ${GENERIC_LIB_VERSION}
    SOVERSION     ${GENERIC_LIB_SOVERSION}
    DEFINE_SYMBOL EXV_BUILDING_LIB
    OUTPUT_NAME   exiv2
)

IF ( UNIX )
  IF ( NOT CYGWIN AND NOT MINGW )
    SET (LINUX 1)
  ENDIF()
  IF ( CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" )
    SET (FREEBSD 1)
  ENDIF()
ENDIF()

if ( MSVC )
    source_group("Header Files" FILES ${LIBEXIV2_HDR} )
    TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} ${ZLIB_LIBRARIES} )
    TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} ${EXPAT_LIBRARIES} )
    source_group("Header Files" FILES ${LIBCURL_HDR} )
    TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} ${CURL_LIBRARIES} )
    source_group("Header Files" FILES ${SSH_HDR} )
    TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} ${SSH_LIBRARIES} )
else()
    if ( UNIX AND NOT FREEBSD )
        TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} dl ${EXPAT_LIBRARIES} )
        TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} dl ${CURL_LIBRARIES} )
        TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} dl ${SSH_LIBRARIES} )
    else()
        TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} ${EXPAT_LIBRARIES} )
        TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} ${CURL_LIBRARIES} )
        TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} ${SSH_LIBRARIES} )
    endif()
endif()

if( EXIV2_ENABLE_LIBXMP )
    ADD_DEPENDENCIES( exiv2lib xmp )
    if  ( MSVC )
        LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH}/$(ConfigurationName))
    else()
        ADD_DEPENDENCIES( exiv2lib xmp Threads::Threads)
        TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} ${EXPAT_LIBRARIES} Threads::Threads)
    endif(MSVC)
    TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} xmp )
ENDIF()

IF( EXIV2_ENABLE_PNG )
    IF( ZLIB_FOUND )
        IF( MSVC )
            TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} optimized ${ZLIB_LIBRARIES} )
        ELSE()
            TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR} ${ZLIB_LIBRARIES} )
        ENDIF()
    ENDIF()
ENDIF()

IF( EXIV2_ENABLE_NLS )
    TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR}  ${LIBINTL_LIBRARIES} )
ENDIF( EXIV2_ENABLE_NLS )

IF( ICONV_FOUND )
    TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR}  ${ICONV_LIBRARIES} )
ENDIF( ICONV_FOUND )

IF (CYGWIN OR MINGW)
    TARGET_LINK_LIBRARIES( exiv2lib ${PRIVATE_VAR}  psapi ws2_32 )
ENDIF(CYGWIN OR MINGW)

INSTALL( TARGETS exiv2lib ${INSTALL_TARGET_STANDARD_ARGS} )

include(../CMake_msvc.txt)
msvc_runtime_configure(${EXIV2_ENABLE_SHARED} ${EXIV2_ENABLE_DYNAMIC_RUNTIME})

# ******************************************************************************
# exiv2 application
ADD_EXECUTABLE( exiv2 ${EXIV2_SRC}  ${EXIV2_HDR} )
TARGET_LINK_LIBRARIES( exiv2 exiv2lib )
INSTALL( TARGETS exiv2 ${INSTALL_TARGET_STANDARD_ARGS} )

# ******************************************************************************
# connection test application
ADD_EXECUTABLE( conntest ${CONNTEST} )
TARGET_LINK_LIBRARIES( conntest ${PRIVATE_VAR}  exiv2lib ${CURL_LIBRARIES} ${SSH_LIBRARIES})

# ******************************************************************************
# exifprint application
# ADD_EXECUTABLE( exifprint ${EXIFPRINT} )
# TARGET_LINK_LIBRARIES( exifprint exiv2lib )

# ******************************************************************************
# remotetest application
ADD_EXECUTABLE( remotetest ${REMOTETEST} )
TARGET_LINK_LIBRARIES( remotetest exiv2lib )

# ******************************************************************************
# Headers
INSTALL( FILES ${LIBEXIV2_HDR} DESTINATION include/exiv2 )

# ******************************************************************************
# Man page
INSTALL( FILES exiv2.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

# That's all Folks!
##
