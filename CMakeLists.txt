# CMake build system for exiv2 library and executables
# Copyright 2015-     Daniel Kaneider  <danielkaneider@users.sf.net>
# Copyright 2010-2012 Gilles Caulier   <caulier dot gilles at gmail dot com>
# Copyright 2008      Patrick Spendrin <ps_ml at gmx dot de>

# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

CMAKE_MINIMUM_REQUIRED( VERSION 3.1.0 )
PROJECT( exiv2 )

if( POLICY CMP0042 )
    cmake_policy(SET CMP0042 NEW) # enable MACOSX_RPATH support
else()
    SET(CMAKE_MACOSX_RPATH 1)
endif()

SET( PACKAGE_COPYRIGHT      "Andreas Huggel" )
SET( PACKAGE_BUGREPORT      "http://github.com/exiv2/exiv2" )
SET( PACKAGE                "exiv2" )
SET( PACKAGE_NAME           "exiv2" )
SET( PACKAGE_VERSION        "0.26" )
SET( PACKAGE_URL            "http://www.exiv2.org" )
SET( PACKAGE_STRING         "${PACKAGE_NAME} ${PACKAGE_VERSION}" )
SET( GENERIC_LIB_VERSION    "26.0.0" )
SET( GENERIC_LIB_SOVERSION  "26" )

# options and their default values
OPTION( EXIV2_ENABLE_SHARED        "Build exiv2 as a shared library (dll)"                 ON  )
OPTION( EXIV2_ENABLE_XMP           "Build with XMP metadata support"                       ON  )
OPTION( EXIV2_ENABLE_LIBXMP        "Build a static convenience Library for XMP"            ON  )
OPTION( EXIV2_ENABLE_PNG           "Build with png support (requires libz)"                ON  )
OPTION( EXIV2_ENABLE_NLS           "Build native language support (requires gettext)"      ON  )
OPTION( EXIV2_ENABLE_PRINTUCS2     "Build with Printucs2"                                  ON  )
OPTION( EXIV2_ENABLE_LENSDATA      "Build including lens data"                             ON  )
OPTION( EXIV2_ENABLE_COMMERCIAL    "Build with the EXV_COMMERCIAL_VERSION symbol set"      OFF )
OPTION( EXIV2_ENABLE_BUILD_SAMPLES "Build the unit tests"                                  ON  )
OPTION( EXIV2_ENABLE_BUILD_PO      "Build translations files"                              OFF )
OPTION( EXIV2_ENABLE_VIDEO         "Build video support into library"                      OFF )
OPTION( EXIV2_ENABLE_WEBREADY      "Build webready support into library"                   OFF )
IF (WIN32)
    OPTION( EXIV2_ENABLE_DYNAMIC_RUNTIME  "Use dynamic runtime (used for static libs)"     OFF )
    OPTION( EXIV2_ENABLE_WIN_UNICODE   "Use Unicode paths (wstring) on Windows"            OFF )
    OPTION( EXIV2_ENABLE_CURL          "USE Libcurl for HttpIo"                            OFF )
    OPTION( EXIV2_ENABLE_SSH           "USE Libssh for SshIo"                              OFF )
ELSE()
    OPTION( EXIV2_ENABLE_CURL          "USE Libcurl for HttpIo"                            ON  )
    OPTION( EXIV2_ENABLE_SSH           "USE Libssh for SshIo"                              ON  )
    SET ( EXIV2_ENABLE_DYNAMIC_RUNTIME OFF )
ENDIF()

include(config/findDependencies.cmake)

IF( MINGW OR UNIX )
    IF ( CMAKE_CXX_COMPILER STREQUAL "g++" OR CMAKE_C_COMPILER STREQUAL "gcc" )
        ADD_DEFINITIONS(-Wall
                    -Wcast-align
                    -Wpointer-arith
                    -Wformat-security
                    -Wmissing-format-attribute
                    -Woverloaded-virtual
                    -W
                   )
    ENDIF()

    execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE COMPILER_VERSION)
    string(REGEX MATCHALL "[A-Z|a-z\+]+" GCC_COMPILER_COMPONENTS ${COMPILER_VERSION})
    list(GET GCC_COMPILER_COMPONENTS 0 COMPILER)

    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
    list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)

    IF ( CYGWIN OR ( ${GCC_MAJOR} GREATER 5 ))
        ADD_DEFINITIONS( -std=gnu++98 ) # to support snprintf
    ELSE()
        ADD_DEFINITIONS( -std=c++98 )
    ENDIF()

ENDIF( MINGW OR UNIX )

IF( EXIV2_ENABLE_COMMERCIAL )
    SET (EXIV2_ENABLE_LENSDATA OFF)
    SET (EXIV2_ENABLE_NLS OFF)
ENDIF()

if( MSVC )
    # cmake_policy(SET CMP0008)
    ADD_DEFINITIONS(-DPSAPI_VERSION=1) # to be compatible with <= WinVista (#905)
    set(CMAKE_DEBUG_POSTFIX "d")
endif( MSVC )

IF( EXIV2_ENABLE_XMP )
    SET( HAVE_XMP_TOOLKIT ON )
    SET( XMPLIB "do" )
    IF (NOT MINGW)
        set(THREADS_PREFER_PTHREAD_FLAG ON)
    ENDIF()
ENDIF( EXIV2_ENABLE_XMP )

include( config/generateConfigFile.cmake )
include_directories(${CMAKE_BINARY_DIR})  # Make the exv_conf.h file visible for the full project

include( config/CMakeChecks.txt )

if( EXIV2_ENABLE_XMP )
    ADD_SUBDIRECTORY( xmpsdk )
endif()
ADD_SUBDIRECTORY( src )

IF( EXIV2_ENABLE_BUILD_SAMPLES )
    ADD_SUBDIRECTORY( samples )
ENDIF( EXIV2_ENABLE_BUILD_SAMPLES )

IF( EXIV2_ENABLE_BUILD_PO )
    ADD_SUBDIRECTORY( po )
ENDIF( EXIV2_ENABLE_BUILD_PO )

##
# tests
ADD_CUSTOM_TARGET(tests       COMMAND env EXIV2_BINDIR="${CMAKE_BINARY_DIR}"/bin make tests       WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" )
ADD_CUSTOM_TARGET(geotag-test COMMAND env EXIV2_BINDIR="${CMAKE_BINARY_DIR}"/bin make geotag-test WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" )

##
# http://dev.exiv2.org/boards/3/topics/1364
# effectively does a make doc on the root directory
# has to run 'make config' and './configure'
# and copy bin/taglist to <exiv2dir>/bin/taglist for use by 'make doc'
IF( MINGW OR UNIX OR APPLE)
  ADD_CUSTOM_TARGET(doc
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc"
    COMMAND chmod +x ./cmake_doc.sh
    COMMAND ./cmake_doc.sh "${CMAKE_BINARY_DIR}"
  )
ENDIF()

INCLUDE( config/printSummary.cmake )
# That's all Folks!
##
