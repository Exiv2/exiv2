set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_CONTACT "Luis DÃ­az MÃ¡s <piponazo@gmail.com>")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

set(CPACK_SOURCE_GENERATOR TGZ)
# https://libwebsockets.org/git/libwebsockets/commit/minimal-examples?id=3e25edf1ee7ea8127e941fd7b664e0e962cfeb85
set(CPACK_SOURCE_IGNORE_FILES $(CPACK_SOURCE_IGNORE_FILES) "/.git/" "/build/" "\\\\.tgz$" "\\\\.tar\\\\.gz$" "\\\\.zip$" "/test/tmp/" )

if ( MSVC )
    set(CPACK_GENERATOR ZIP)  # use .zip - less likely to damage bin/exiv2.dll permissions
else()
    set(CPACK_GENERATOR TGZ)  # MinGW/Cygwin/Linux/macOS etc use .tar.gz
endif()

set (BS "") # Bit Size
if ( NOT APPLE )
  if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set (BS 64)
  else()
    set (BS 32)
  endif()
endif()

set (LT "") # Library Type
if ( NOT BUILD_SHARED_LIBS )
	set (LT Static)
endif()

set (BT "") # Build Type
if ( NOT ${CMAKE_BUILD_TYPE} STREQUAL Release )
	set (BT ${CMAKE_BUILD_TYPE})
endif()

if ( MINGW OR MSYS )
    set (PACKDIR MinGW)
elseif ( MSVC )
    set (PACKDIR msvc)
elseif ( CYGWIN )
    set (PACKDIR CYGWIN)
elseif ( APPLE )
    set (PACKDIR Darwin)
elseif ( LINUX )
    set (PACKDIR Linux)
elseif ( CMAKE_SYSTEM_NAME STREQUAL "NetBSD" OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_HOST_SOLARIS)
    set (PACKDIR Unix)
else()
    set (PACKDIR Linux) # Linux and unsupported systems
endif()

set (BUNDLE_NAME ${PACKDIR})
if ( CMAKE_SYSTEM_NAME STREQUAL "NetBSD" OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_HOST_SOLARIS )
    set (BUNDLE_NAME ${CMAKE_SYSTEM_NAME})
endif()

set (CC "") # Compiler
if ( NOT APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" )
  if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set (CC Clang)
  endif()
endif()

set (VI "") # Video
if ( EXIV2_ENABLE_VIDEO )
    set (VI Video)
endif()

set (WR "") # WebReady
if ( EXIV2_ENABLE_WEBREADY )
    set (WR Webready)
endif()

set (VS "") # VisualStudio
if ( MSVC )
    # VS2015 >= 1900, VS2017 >= 1910, VS2019 >= 1920
    if     ( MSVC_VERSION GREATER  1919 )
       set(VS 2019)
    elseif ( MSVC_VERSION GREATER  1909 )
       set(VS 2017)
    elseif ( MSVC_VERSION GREATER  1899 )
       set(VS 2015)
    elseif ( MSVC_VERSION STREQUAL 1800 )
       set(VS 2013)
    elseif ( MSVC_VERSION STREQUAL 1700 )
       set(VS 2012)
    elseif ( MSVC_VERSION STREQUAL 1600 )
       set(VS 2010)
    elseif ( MSVC_VERSION STREQUAL 1500 )
       set(VS 2008)
    endif()
endif()

# Set RC = Release Candidate
if ( PROJECT_VERSION_TWEAK STREQUAL "9" )
    set (RC "Not for release")
elseif ( (PROJECT_VERSION_TWEAK STREQUAL "0") OR (PROJECT_VERSION_TWEAK STREQUAL "")  )
    set(RC "GM Release")
else()
     set ( RC "Release Candidate ${PROJECT_VERSION_TWEAK}" )
endif()

# Set RV = Release Version
set(RV "Exiv2 v${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${VS}${BUNDLE_NAME}${BS}${CC}${LT}${BT}${VI}${WR})

# https://stackoverflow.com/questions/17495906/copying-files-and-including-them-in-a-cpack-archive
install(FILES     "${PROJECT_SOURCE_DIR}/samples/exifprint.cpp" DESTINATION "samples")

# Copy top level documents (eg README.md)
# https://stackoverflow.com/questions/21541707/cpack-embed-text-files
set( DOCS
     README.md
     README-CONAN.md
     README-SAMPLES.md
     COPYING
     exiv2.png
)
foreach(doc ${DOCS})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${doc} DESTINATION .)
endforeach()

# copy build/log which which is present if built by build.sh
if(EXISTS ${PROJECT_SOURCE_DIR}/build/logs/build.txt)
    install(FILES ${PROJECT_SOURCE_DIR}/build/logs/build.txt DESTINATION "logs")
endif()

# Copy releasenotes.txt and appropriate ReadMe.txt (eg releasenotes/${PACKDIR}/ReadMe.txt)
set(VM   ${PROJECT_VERSION_MAJOR})           # Version Major  0
set(VN   ${PROJECT_VERSION_MINOR})           # Version Minor 27
set(VD   ${PROJECT_VERSION_PATCH})           # Version Dot    3
set(VR  .${PROJECT_VERSION_TWEAK})           # Version RC    .1
if ( (PROJECT_VERSION_TWEAK STREQUAL "0") OR (PROJECT_VERSION_TWEAK STREQUAL "")  )
    set(VR "")
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/releasenotes/${PACKDIR}/ReadMe.txt ReadMe.txt       @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/releasenotes/releasenotes.txt      releasenotes.txt @ONLY)
install       (FILES  ${CMAKE_CURRENT_BINARY_DIR}/ReadMe.txt ${CMAKE_CURRENT_BINARY_DIR}/releasenotes.txt DESTINATION .)

include (CPack)
