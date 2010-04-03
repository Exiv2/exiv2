# CMake build system for exiv2 library and executables
# Copyright 2008 by Patrick Spendrin <ps_ml@gmx.de>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    1. Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#    2. Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#    3. The name of the author may not be used to endorse or promote
#       products derived from this software without specific prior
#       written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ConfigureChecks for exiv2

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/config )
include( CheckIncludeFile )
include( CheckFunctionExists )
include( CheckSymbolExists )
include( CheckCSourceCompiles )

set( STDC_HEADERS ON )
set( HAVE_DECL_STRERROR_R 0 )

set( HAVE_PRINTUCS2 ${EXIV2_ENABLE_PRINTUCS2} )
set( HAVE_LENSDATA ${EXIV2_ENABLE_LENSDATA} )

include_directories( ${CMAKE_INCLUDE_PATH} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/xmpsdk/include )
link_directories( ${CMAKE_LIBRARY_PATH} )
set( CMAKE_REQUIRED_INCLUDES ${CMAKE_INCLUDE_PATH} )

if( EXIV2_ENABLE_PNG )
    find_package( ZLIB )
    include_directories( ${ZLIB_INCLUDE_DIR} )
    set (HAVE_LIBZ ${ZLIB_FOUND})
endif( EXIV2_ENABLE_PNG )

if (EXIV2_ENABLE_XMP)
    find_package(EXPAT)
    include_directories(${EXPAT_INCLUDE_DIR})
    # FindEXPAT.cmake doesn't check for REQUIRED flags - so we need to check ourselves
    if (NOT EXPAT_FOUND)
        message(FATAL_ERROR "missing library expat required for XMP")
    endif( NOT EXPAT_FOUND )
endif (EXIV2_ENABLE_XMP)

if( EXIV2_ENABLE_SHARED )
    add_definitions( -DEXV_HAVE_DLL )
    set( STATIC_FLAG SHARED )
else( EXIV2_ENABLE_SHARED )
    set( STATIC_FLAG STATIC )
endif( EXIV2_ENABLE_SHARED )

if( EXIV2_ENABLE_NLS )
    if( NOT LOCALEDIR )
        set( LOCALEDIR "\"${CMAKE_INSTALL_PREFIX}/share/locale\"" )
        if( WIN32 )
            string( REPLACE "/" "\\\\" LOCALEDIR ${LOCALEDIR} )
        endif( WIN32 )
    endif( NOT LOCALEDIR )
    add_definitions( -DEXV_LOCALEDIR=${LOCALEDIR} )
endif( EXIV2_ENABLE_NLS )

if( EXIV2_ENABLE_COMMERCIAL )
    add_definitions( -DEXV_COMMERCIAL_VERSION )
endif( EXIV2_ENABLE_COMMERCIAL )

find_package(Iconv)
if( ICONV_TEST )
    set( HAVE_ICONV 1 )
    message ( "================> ICONV_LIBRARIES : " ${ICONV_LIBRARIES} )
endif( ICONV_TEST )
if( ICONV_ACCEPTS_CONST_INPUT )
    set( ICONV_CONST "const" )
endif( ICONV_ACCEPTS_CONST_INPUT )

# checking for Header files
check_include_file( "inttypes.h" HAVE_INTTYPES_H )
check_include_file( "libintl.h" HAVE_LIBINTL_H )
check_include_file( "malloc.h" HAVE_MALLOC_H )
check_include_file( "memory.h" HAVE_MEMORY_H )
check_include_file( "iconv.h" HAVE_ICONV_H )
check_include_file( "stdbool.h" HAVE_STDBOOL_H )
check_include_file( "stdint.h" HAVE_STDINT_H )
check_include_file( "stdlib.h" HAVE_STDLIB_H )
check_include_file( "string.h" HAVE_STRING_H )
check_include_file( "strings.h" HAVE_STRINGS_H )
check_include_file( "unistd.h" HAVE_UNISTD_H )
check_include_file( "wchar.h" HAVE_WCHAR_H )
check_include_file( "sys/stat.h" HAVE_SYS_STAT_H )
check_include_file( "sys/time.h" HAVE_SYS_TIME_H )
check_include_file( "sys/types.h" HAVE_SYS_TYPES_H )
check_include_file( "sys/mman.h" HAVE_SYS_MMAN_H )
check_include_file( "process.h" HAVE_PROCESS_H )

check_function_exists( alarm HAVE_ALARM )
check_function_exists( gmtime_r HAVE_GMTIME_R )
check_function_exists( malloc HAVE_MALLOC )
check_function_exists( memset HAVE_MEMSET )
check_function_exists( mmap HAVE_MMAP )
check_function_exists( munmap HAVE_MUNMAP )
check_function_exists( realloc HAVE_REALLOC )
check_function_exists( strchr HAVE_STRCHR )
check_function_exists( strchr_r HAVE_STRCHR_R )
check_function_exists( strerror HAVE_STRERROR )
check_function_exists( strerror_r HAVE_STRERROR_R )
check_function_exists( strtol HAVE_STRTOL )
check_function_exists( timegm HAVE_TIMEGM )
check_function_exists( vprintf HAVE_VPRINTF )

message( STATUS "None:              ${CMAKE_CXX_FLAGS}" )
message( STATUS "Debug:             ${CMAKE_CXX_FLAGS_DEBUG}" )
message( STATUS "Release:           ${CMAKE_CXX_FLAGS_RELEASE}" )
message( STATUS "RelWithDebInfo:    ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )
message( STATUS "MinSizeRel:        ${CMAKE_CXX_FLAGS_MINSIZEREL}" )

if( HAVE_STDBOOL_H )
    check_symbol_exists( "_Bool" stdbool.h HAVE__BOOL )
endif( HAVE_STDBOOL_H )

# struct tm in sys/time.h
if( HAVE_SYS_TIME_H )
    check_c_source_compiles( "#include <sys/time.h>
int main() {
struct tm t;
return 0;
}" TM_IN_SYS_TIME )
endif( HAVE_SYS_TIME_H )

# strerror_r returns char*
check_cxx_source_compiles( "
#include <cstring>
int dfunc() { char cb[12]; return *(strerror_r(1, cb, sizeof(cb))) == '\0'; }
" STRERROR_R_CHAR_P )

#function is declared with the above
if( STRERROR_R_CHAR_P )
    set( HAVE_DECL_STRERROR_R 1 )
endif( STRERROR_R_CHAR_P )

# time.h and sys/time.h can be included in the same file
check_c_source_compiles( "#include <time.h>
#include <sys/time.h>
int main() {
return 0;
}" TIME_WITH_SYS_TIME )

# for msvc define to int in exv_conf.h
if( NOT MSVC )
    set( HAVE_PID_T TRUE )
endif( NOT MSVC )

set( EXV_SYMBOLS
ENABLE_NLS
HAVE_ALARM
HAVE_DECL_STRERROR_R
HAVE_GMTIME_R
HAVE_ICONV
HAVE_ICONV_H
HAVE_INTTYPES_H
HAVE_LENSDATA
HAVE_LIBINTL_H
HAVE_LIBZ
HAVE_MALLOC_H
HAVE_MEMORY_H
HAVE_MEMSET
HAVE_MMAP
HAVE_MUNMAP
HAVE_PRINTUCS2
HAVE_PROCESS_H
HAVE_REALLOC
HAVE_STDBOOL_H
HAVE_STDINT_H
HAVE_STDLIB_H
HAVE_STRCHR
HAVE_STRCHR_R
HAVE_STRERROR
HAVE_STRERROR_R
HAVE_STRINGS_H
HAVE_STRING_H
HAVE_STRTOL
HAVE_SYS_MMAN_H
HAVE_SYS_STAT_H
HAVE_SYS_TIME_H
HAVE_SYS_TYPES_H
HAVE_TIMEGM
HAVE_UNISTD_H
HAVE_VPRINTF
HAVE_WCHAR_H
HAVE_XMP_TOOLKIT
HAVE__BOOL
ICONV_CONST
PACKAGE
PACKAGE_BUGREPORT
PACKAGE_NAME
PACKAGE_STRING
PACKAGE_TARNAME
PACKAGE_VERSION
)

foreach( entry ${EXV_SYMBOLS} )
    set( EXV_${entry} ${${entry}} )
endforeach( entry ${EXV_SYMBOLS} )

configure_file( config/config.h.cmake ${CMAKE_BINARY_DIR}/exv_conf.h )
configure_file( config/exv_msvc.h.cmake ${CMAKE_BINARY_DIR}/exv_msvc.h COPYONLY )
install( FILES ${CMAKE_BINARY_DIR}/exv_conf.h DESTINATION include/exiv2 )
install( FILES ${CMAKE_BINARY_DIR}/exv_msvc.h DESTINATION include/exiv2 )

if( NOT MSVC )
    configure_file( config/exiv2.pc.cmake ${CMAKE_BINARY_DIR}/exiv2.pc )
    install( FILES ${CMAKE_BINARY_DIR}/exiv2.pc DESTINATION lib/pkgconfig )
endif( NOT MSVC )

# ******************************************************************************
# output chosen build options
macro( OptionOutput _outputstring )
    if( ${ARGN} )
        set( _var "YES" )
    else( ${ARGN} )
        set( _var "NO" )
    endif( ${ARGN} )
    message( STATUS "${_outputstring}${_var}" )
endmacro( OptionOutput _outputstring )

message( STATUS "-------------------------------------------------------------" )
OptionOutput( "Building PNG support:            " EXIV2_ENABLE_PNG AND ZLIB_FOUND )
OptionOutput( "Building shared library:         " EXIV2_ENABLE_SHARED )
OptionOutput( "XMP metadata support:            " EXIV2_ENABLE_XMP )
OptionOutput( "Building static libxmp:          " ENABLE_LIBXMP )
OptionOutput( "Native language support:         " EXIV2_ENABLE_NLS )
OptionOutput( "Conversion of Windows XP tags:   " EXIV2_ENABLE_PRINTUCS2 )
OptionOutput( "Nikon lens database:             " EXIV2_ENABLE_LENSDATA )
OptionOutput( "commercial build:                " EXIV2_ENABLE_COMMERCIAL )
message( STATUS "-------------------------------------------------------------" )
