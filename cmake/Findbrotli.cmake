#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) 1998 - 2020, Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at https://curl.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
###########################################################################
include(FindPackageHandleStandardArgs)

find_path(brotli_INCLUDE_DIR "brotli/decode.h")

find_library(brotlicommon_LIBRARY NAMES brotlicommon)
find_library(brotlidec_LIBRARY NAMES brotlidec)

find_package_handle_standard_args(brotli
    FOUND_VAR
      brotli_FOUND
    REQUIRED_VARS
      brotlidec_LIBRARY
      brotlicommon_LIBRARY
      brotli_INCLUDE_DIR
    FAIL_MESSAGE
      "Could NOT find brotli"
)

set(brotli_INCLUDE_DIRS ${brotli_INCLUDE_DIR})
set(brotli_LIBRARIES ${brotlidec_LIBRARY} ${brotlicommon_LIBRARY})

mark_as_advanced(brotli_INCLUDE_DIR)
mark_as_advanced(brotlicommon_LIBRARY)
mark_as_advanced(brotlidec_LIBRARY)
