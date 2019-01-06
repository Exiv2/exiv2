include(CheckIncludeFile)
include(CheckCXXSourceCompiles)

# Note that the scope of the EXV_ variables in local
if (${EXIV2_ENABLE_WEBREADY})
    set(EXV_USE_SSH   ${EXIV2_ENABLE_SSH})
    set(EXV_USE_CURL  ${EXIV2_ENABLE_CURL})
endif()
set(EXV_ENABLE_VIDEO     ${EXIV2_ENABLE_VIDEO})
set(EXV_ENABLE_WEBREADY  ${EXIV2_ENABLE_WEBREADY})
set(EXV_HAVE_LENSDATA    ${EXIV2_ENABLE_LENSDATA})
set(EXV_HAVE_PRINTUCS2   ${EXIV2_ENABLE_PRINTUCS2})

set(EXV_PACKAGE_NAME     ${PROJECT_NAME})
set(EXV_PACKAGE_VERSION  ${PROJECT_VERSION})
set(EXV_PACKAGE_STRING   "${PROJECT_NAME} ${PROJECT_VERSION}")
if (${EXIV2_ENABLE_XMP} OR ${EXIV2_ENABLE_EXTERNAL_XMP})
    set(EXV_HAVE_XMP_TOOLKIT ON)
else()
    set(EXV_HAVE_XMP_TOOLKIT OFF)
endif()
set(EXV_HAVE_ICONV       ${ICONV_FOUND})
set(EXV_HAVE_LIBZ        ${ZLIB_FOUND})
set(EXV_UNICODE_PATH     ${EXIV2_ENABLE_WIN_UNICODE})

# TODO: Try to use the cmake function check_symbol_exists which is more robust
check_function_exists( gmtime_r EXV_HAVE_GMTIME_R )
check_function_exists( mmap     EXV_HAVE_MMAP )
check_function_exists( munmap   EXV_HAVE_MUNMAP )
check_function_exists( strerror_r   EXV_HAVE_STRERROR_R )

check_cxx_source_compiles( "
#include <string.h>
int main() {
    char buff[100];
    const char* c = strerror_r(0,buff,100);
    return 0;
}" EXV_STRERROR_R_CHAR_P )

check_include_file( "unistd.h"  EXV_HAVE_UNISTD_H )
check_include_file( "sys/mman.h"    EXV_HAVE_SYS_MMAN_H )
if ( NOT MINGW AND NOT MSYS AND NOT MSVC )
check_include_file( "regex.h"       EXV_HAVE_REGEX_H )
endif()

set(EXV_ENABLE_NLS ${EXIV2_ENABLE_NLS})

configure_file(cmake/config.h.cmake ${CMAKE_BINARY_DIR}/exv_conf.h @ONLY)
