# CMakeLists.txt for exiv2 library and command-line program

# C++ source files
set( LIBEXIV2_SRC         asfvideo.cpp
                          basicio.cpp
                          bigtiffimage.cpp
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

                          cr2image_int.cpp
   )

# Public C++ header files
set( LIBEXIV2_HDR         ../include/exiv2/asfvideo.hpp
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

# Private headers only needed to build the library
set( LIBEXIV2_PRIVATE_HDR canonmn_int.hpp
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


if( EXIV2_ENABLE_WEBREADY )
    if( EXIV2_ENABLE_CURL)
        set( LIBEXIV2_SRC ${LIBEXIV2_SRC} easyaccess.cpp)
        set( LIBEXIV2_HDR ${LIBEXIV2_HDR} ../include/exiv2/easyaccess.hpp)
    endif()

    if( EXIV2_ENABLE_SSH )
        set( LIBEXIV2_SRC ${LIBEXIV2_SRC} ssh.cpp)
        set( LIBEXIV2_HDR ${LIBEXIV2_HDR} ../include/exiv2/ssh.hpp)
    endif()
endif()

##
# add png*.cpp files if PNG support requested
if( EXIV2_ENABLE_PNG )
    set( LIBEXIV2_SRC ${LIBEXIV2_SRC} pngchunk.cpp pngimage.cpp)
    set( LIBEXIV2_HDR ${LIBEXIV2_HDR} ../include/exiv2/pngimage.hpp )
endif()

source_group("Header Files" FILES ${LIBEXIV2_HDR} )
source_group("Header Files" FILES ${LIBCURL_HDR} )
source_group("Header Files" FILES ${SSH_HDR} )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# ******************************************************************************
# exiv2lib library

add_library( exiv2lib ${LIBEXIV2_SRC} ${LIBEXIV2_HDR} ${LIBEXIV2_PRIVATE_HDR})

set_target_properties( exiv2lib PROPERTIES
    VERSION       ${GENERIC_LIB_VERSION}
    SOVERSION     ${GENERIC_LIB_SOVERSION}
    OUTPUT_NAME   exiv2
    COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
)

target_compile_definitions(exiv2lib PRIVATE EXV_LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}" EXV_BUILDING_LIB )
if ( BUILD_SHARED_LIBS )
    target_compile_definitions(exiv2lib PRIVATE EXV_HAVE_DLL )
endif()

if ( ${BUILD_SHARED_LIBS} )
    target_compile_definitions(exiv2lib PUBLIC EXV_HAVE_DLL)
endif()

if ( UNIX AND CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" )
    set (FREEBSD 1)
endif()

if ( EXIV2_ENABLE_XMP )
    #target_include_directories(exiv2lib PRIVATE ${CMAKE_SOURCE_DIR}/xmpsdk/include)
    target_link_libraries(exiv2lib PUBLIC xmp)
endif()

# TODO : We should not include include/exiv2 but only include !!!
target_include_directories(exiv2lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/exiv2>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if (EXIV2_ENABLE_WEBREADY)

    if( EXIV2_ENABLE_SSH )
        target_include_directories(exiv2lib SYSTEM PUBLIC ${SSH_INCLUDE_DIR} )
        target_link_libraries( exiv2lib PUBLIC ${SSH_LIBRARIES})
    endif()

    if( EXIV2_ENABLE_CURL )
        target_include_directories(exiv2lib SYSTEM PUBLIC ${CURL_INCLUDE_DIR} )

        if (USING_CONAN)
            # TODO : Improve libcurl recipe so we do not need to have all these conditionals here
            if ( MSVC )
                target_link_libraries( exiv2lib PUBLIC ${CURL_LIBRARY})
            else()
                target_link_libraries( exiv2lib PUBLIC ${CONAN_LIBS_LIBCURL})
            endif()
            target_compile_definitions(exiv2lib PUBLIC ${CONAN_COMPILE_DEFINITIONS_LIBCURL})
        else()
            target_link_libraries( exiv2lib PUBLIC ${CURL_LIBRARIES})
        endif()

    endif()

endif()

if ( MSVC )
    target_compile_definitions(exiv2lib
        PRIVATE
            PSAPI_VERSION=1 # to be compatible with <= WinVista (#905)
    )

    source_group("Header Files" FILES ${LIBEXIV2_HDR} )
    source_group("Header Files" FILES ${LIBCURL_HDR} )
    source_group("Header Files" FILES ${SSH_HDR} )
    target_link_libraries( exiv2lib PRIVATE ${CURL_LIBRARIES} ${SSH_LIBRARIES})
else()
    # TODO: Check if this is really needed.
    if ( UNIX AND NOT FREEBSD )
        target_link_libraries( exiv2lib PRIVATE ${CMAKE_DL_LIBS}) # -ldl = dynamic loader used by src/version.cpp
    endif()

    if (CYGWIN OR MINGW)
        target_link_libraries( exiv2lib PRIVATE  psapi ws2_32 )
    endif()

    target_link_libraries( exiv2lib PRIVATE Threads::Threads)
endif()

if( EXIV2_ENABLE_PNG )
    target_link_libraries( exiv2lib PRIVATE ${ZLIB_LIBRARIES} )
endif()

if( EXIV2_ENABLE_NLS )
    target_link_libraries( exiv2lib PRIVATE  ${LIBINTL_LIBRARIES} )
endif()

if( ICONV_FOUND )
    target_include_directories( exiv2lib SYSTEM PRIVATE ${ICONV_INCLUDE_DIR})
    target_link_libraries( exiv2lib PRIVATE  ${ICONV_LIBRARIES} )
endif()


install(TARGETS exiv2lib EXPORT exiv2Config
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES ${LIBEXIV2_HDR}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/exiv2
)

install(EXPORT exiv2Config DESTINATION "share/exiv2/cmake")

# ******************************************************************************
# exiv2 application

set( EXIV2_SRC
     exiv2.cpp
     exiv2app.hpp
     actions.cpp
     actions.hpp
     utils.cpp
     utils.hpp
)

if(EXIV2_BUILD_EXIV2_COMMAND)
    add_executable( exiv2 ${EXIV2_SRC} )
    set_target_properties( exiv2 PROPERTIES
        COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
    )

    target_compile_definitions(exiv2 PRIVATE EXV_LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}" )
    target_link_libraries( exiv2 PRIVATE exiv2lib )
    if ( BUILD_SHARED_LIBS )
        target_compile_definitions(exiv2 PRIVATE EXV_HAVE_DLL )
    endif()
    # modify source lists to suit environment

    # TODO This should not be needed here! we need to fix the previous TODO
    target_include_directories(exiv2 PRIVATE ${CMAKE_SOURCE_DIR}/include/)

    if(NOT EXV_HAVE_TIMEGM )
        target_sources(exiv2    PRIVATE localtime.c)
    endif()

    if (MSVC)
        target_sources(exiv2    PRIVATE getopt_win32.c)
    endif()
    install(TARGETS exiv2 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# modify source lists to suit environment
if(NOT EXV_HAVE_TIMEGM )
    target_sources(exiv2lib PRIVATE localtime.c)
endif()

if (MSVC)
    target_sources(exiv2lib PRIVATE getopt_win32.c)
endif()

# ******************************************************************************
# Man page
install( FILES exiv2.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

# That's all Folks!
##
