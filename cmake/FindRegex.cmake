# - Try to find the Regex library
#
# Once done this will define
#
#  REGEX_FOUND       - system has libregex
#  REGEX_INCLUDE_DIR - the libregex include directory
#  REGEX_LIBRARIES   - Link these to use libregex
#
# Copyright (c) 2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if ( NOT MSVC AND NOT MINGW AND NOT MSYS )
    find_path(Regex_INCLUDE_DIR
              NAMES regex.h
              DOC "libregex include directory"
    )

    mark_as_advanced(Regex_INCLUDE_DIR)

    find_library(Regex_LIBRARY "regex"
                 DOC "libregex libraries"
    )

    mark_as_advanced(Regex_LIBRARY)

    find_package_handle_standard_args(Regex
                                      FOUND_VAR Regex_FOUND
                                      REQUIRED_VARS Regex_INCLUDE_DIR
                                      FAIL_MESSAGE "Failed to find libregex"
    )

    if(REGEX_FOUND)

        set(REGEX_INCLUDE_DIRS ${Regex_INCLUDE_DIRS})

        if(Regex_LIBRARY)

            set(REGEX_LIBRARIES ${Regex_LIBRARY})

        else()

            unset(REGEX_LIBRARIES)

        endif()

    endif()
endif()

