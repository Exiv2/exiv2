#
# Copyright (c) 2015-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(CheckIncludeFile)

CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(stddef.h HAVE_STDDEF_H)
CHECK_INCLUDE_FILE(strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)

if(HAVE_INTTYPES_H)
    add_definitions(-DHAVE_INTTYPES_H)
endif()

if(HAVE_STDDEF_H)
    add_definitions(-DHAVE_STDDEF_H)
endif()

if(HAVE_STRINGS_H)
    add_definitions(-DHAVE_STRINGS_H)
endif()

if(HAVE_UNISTD_H)
    add_definitions(-DHAVE_UNISTD_H)
endif()

set(NUMERIC_VERSION 0x01050100)
set(PACKAGE_VERSION 1.5.1)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libheif/heif_version.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/libheif/heif_version.h"
)

set(libheif_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libheif/bitstream.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/libheif/box.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/libheif/error.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/libheif/heif.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/libheif/heif_context.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/libheif/heif_file.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/libheif/heif_image.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/libheif/heif_hevc.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/libheif/heif_colorconversion.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/libheif/heif_plugin_registry.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/libheif/heif_plugin.cc
)

foreach(_currentfile ${libheif_SRCS})
    if(NOT MSVC)
        set_source_files_properties(${_currentfile} PROPERTIES COMPILE_FLAGS "-w")
    endif()
endforeach()

