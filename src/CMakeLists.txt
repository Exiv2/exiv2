# CMakeLists.txt for exiv2 library and command-line program

# list of public C++ include files
file(GLOB   LIBEXIV2_HDR ../include/exiv2/*.hpp)
list(APPEND LIBEXIV2_HDR ${CMAKE_BINARY_DIR}/exv_conf.h)

# list of internal include files only required to build the library
file(GLOB LIBEXIV2_PRIVATE_HDR *_int.hpp)

# list of library C++ source files
file(GLOB        LIBEXIV2_SRC *.cpp)
# remove files which are not part of the library
file(GLOB        LIBEXIV2_SRC_NOT actions.cpp exiv2.cpp *test.cpp crwparse.cpp mrwthumb.cpp xmpdump.cpp) 
foreach(entry  ${LIBEXIV2_SRC_NOT})
    list(REMOVE_ITEM LIBEXIV2_SRC ${entry})
endforeach()

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

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# ******************************************************************************
# exiv2lib library

if( EXIV2_ENABLE_XMP AND NOT EXIV2_ENABLE_LIBXMP )
    add_library( exiv2lib ${LIBEXIV2_SRC} ${LIBEXIV2_HDR} ${LIBEXIV2_PRIVATE_HDR} $<TARGET_OBJECTS:xmp_object>)
else()
    add_library( exiv2lib ${LIBEXIV2_SRC} ${LIBEXIV2_HDR} ${LIBEXIV2_PRIVATE_HDR})
endif()
if ( BUILD_SHARED_LIBS )
    target_link_libraries(exiv2lib PUBLIC ${EXPAT_LIBRARIES})
endif()

set_target_properties( exiv2lib PROPERTIES
    VERSION       ${GENERIC_LIB_VERSION}
    SOVERSION     ${GENERIC_LIB_SOVERSION}
    OUTPUT_NAME   exiv2
)

target_compile_definitions(exiv2lib PRIVATE EXV_LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}" EXV_BUILDING_LIB )
if ( BUILD_SHARED_LIBS )
    target_compile_definitions(exiv2lib PRIVATE EXV_HAVE_DLL )
endif()

if ( UNIX AND CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" )
    set (FREEBSD 1)
endif()

if ( EXIV2_ENABLE_XMP )
    target_include_directories(exiv2lib PRIVATE ${CMAKE_SOURCE_DIR}/xmpsdk/include)
endif()

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

    if( EXIV2_ENABLE_PNG )
        target_link_libraries( exiv2lib PRIVATE ${ZLIB_LIBRARIES} )
    endif()
    
    source_group("Header Files" FILES ${LIBEXIV2_HDR} )
    source_group("Header Files" FILES ${LIBCURL_HDR} )
    source_group("Header Files" FILES ${SSH_HDR} )
    target_link_libraries( exiv2lib PRIVATE ${ZLIB_LIBRARIES} ${CURL_LIBRARIES} ${SSH_LIBRARIES})
else()
    # TODO: Check if this is really needed.
    if ( UNIX AND NOT FREEBSD )
        target_link_libraries( exiv2lib PRIVATE dl) # -ldl = dynamic loader used by src/version.cpp
    endif()
    target_link_libraries( exiv2lib PRIVATE Threads::Threads)
    target_link_libraries( exiv2lib PUBLIC ${CURL_LIBRARIES} ${SSH_LIBRARIES})
endif()

if( EXIV2_ENABLE_XMP AND EXIV2_ENABLE_LIBXMP )
    target_link_libraries( exiv2lib PUBLIC xmp )
endif()

if( EXIV2_ENABLE_PNG )
    target_link_libraries( exiv2lib PRIVATE ${ZLIB_LIBRARIES} )
endif()

if( EXIV2_ENABLE_NLS )
    target_link_libraries( exiv2lib PRIVATE  ${LIBINTL_LIBRARIES} )
endif()

if( ICONV_FOUND )
    target_link_libraries( exiv2lib PRIVATE  ${ICONV_LIBRARIES} )
endif()

if (CYGWIN OR MINGW)
    target_link_libraries( exiv2lib PRIVATE  psapi ws2_32 )
endif()

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
    target_compile_definitions(exiv2 PRIVATE EXV_LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}" )
    target_link_libraries( exiv2 PRIVATE exiv2lib )
    if ( BUILD_SHARED_LIBS )
        target_compile_definitions(exiv2 PRIVATE EXV_HAVE_DLL )
    endif()    
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
install( FILES exiv2.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

# That's all Folks!
##
