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
                          ../include/exiv2/xmp.hpp
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
IF(NOT EXV_HAVE_TIMEGM )
    SET( LIBEXIV2_SRC     ${LIBEXIV2_SRC} localtime.c )
    SET(    EXIV2_SRC        ${EXIV2_SRC} localtime.c )
    SET( PATHTEST_SRC     ${PATHTEST_SRC} localtime.c )
ENDIF()

IF( MSVC )
    SET(    EXIV2_SRC        ${EXIV2_SRC} getopt_win32.c )
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

IF ( UNIX )
  IF ( NOT CYGWIN AND NOT MINGW )
    SET (LINUX 1)
  ENDIF()
  IF ( CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" )
    SET (FREEBSD 1)
  ENDIF()
ENDIF()

target_include_directories(exiv2lib PRIVATE ${CMAKE_SOURCE_DIR}/xmpsdk/include)

if ( MSVC )
    if ( EXIV2_ENABLE_STATIC )
        TARGET_LINK_LIBRARIES( exiv2lib zlibstatic ${ZLIB_LIBRARIES} )
    else()
        TARGET_LINK_LIBRARIES( exiv2lib PRIVATE ${ZLIB_LIBRARIES} )
    endif()
    source_group("Header Files" FILES ${LIBEXIV2_HDR} )
    source_group("Header Files" FILES ${LIBCURL_HDR} )
    source_group("Header Files" FILES ${SSH_HDR} )
    TARGET_LINK_LIBRARIES( exiv2lib PRIVATE ${ZLIB_LIBRARIES} ${CURL_LIBRARIES} ${SSH_LIBRARIES})
else()
    if ( UNIX AND NOT FREEBSD )
        TARGET_LINK_LIBRARIES( exiv2lib PRIVATE dl ${CURL_LIBRARIES} )
        TARGET_LINK_LIBRARIES( exiv2lib PRIVATE dl ${SSH_LIBRARIES} )
        TARGET_LINK_LIBRARIES( exiv2lib PRIVATE dl ${CMAKE_THREAD_LIBS_INIT} )
    else()
        TARGET_LINK_LIBRARIES( exiv2lib PRIVATE ${CURL_LIBRARIES} )
        TARGET_LINK_LIBRARIES( exiv2lib PRIVATE ${SSH_LIBRARIES} )
        TARGET_LINK_LIBRARIES( exiv2lib PRIVATE ${CMAKE_THREAD_LIBS_INIT} )
    endif()
endif()

if( EXIV2_ENABLE_LIBXMP )
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
     RUNTIME DESTINATION bin
     LIBRARY DESTINATION lib
     ARCHIVE DESTINATION lib
)
install(FILES ${LIBEXIV2_HDR} 
    DESTINATION include/exiv2 
)

include(../CMake_msvc.txt)
msvc_runtime_configure(${EXIV2_ENABLE_SHARED} ${EXIV2_ENABLE_DYNAMIC_RUNTIME})

# ******************************************************************************
# exiv2 application
ADD_EXECUTABLE( exiv2 ${EXIV2_SRC}  ${EXIV2_HDR} )
TARGET_LINK_LIBRARIES( exiv2 exiv2lib ${LIBINTL_LIBRARIES} )
install(TARGETS exiv2 
     RUNTIME DESTINATION bin
 )

# ******************************************************************************
# connection test application
ADD_EXECUTABLE( conntest ${CONNTEST} )
TARGET_LINK_LIBRARIES( conntest PRIVATE  exiv2lib ${CURL_LIBRARIES} ${SSH_LIBRARIES} ${LIBINTL_LIBRARIES} )

# ******************************************************************************
# exifprint application
# ADD_EXECUTABLE( exifprint ${EXIFPRINT} )
# TARGET_LINK_LIBRARIES( exifprint exiv2lib )

# ******************************************************************************
# remotetest application
ADD_EXECUTABLE( remotetest ${REMOTETEST} )
TARGET_LINK_LIBRARIES( remotetest exiv2lib ${LIBINTL_LIBRARIES} )


# ******************************************************************************
# Man page
INSTALL( FILES exiv2.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

# That's all Folks!
##
