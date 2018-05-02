include(CheckIncludeFile)

# Note that the scope of the EXV_ variables in local
if (${EXIV2_ENABLE_WEBREADY})
    set(EXV_USE_SSH   ${EXIV2_ENABLE_SSH})
    set(EXV_USE_CURL  ${EXIV2_ENABLE_CURL})
endif()
set(EXV_ENABLE_VIDEO     ${EXIV2_ENABLE_VIDEO})
set(EXV_ENABLE_WEBREADY  ${EXIV2_ENABLE_WEBREADY})
set(EXV_HAVE_LENSDATA    ${EXIV2_ENABLE_LENSDATA})
set(EXV_HAVE_PRINTUCS2   ${EXIV2_ENABLE_PRINTUCS2})

set(EXV_PACKAGE          ${PACKAGE})
set(EXV_PACKAGE_VERSION  ${PACKAGE_VERSION})
set(EXV_PACKAGE_STRING   ${PACKAGE_STRING})
set(EXV_HAVE_XMP_TOOLKIT ${EXIV2_ENABLE_XMP})
set(EXV_HAVE_ICONV       ${ICONV_FOUND})
set(EXV_HAVE_LIBZ        ${ZLIB_FOUND})
set(EXV_UNICODE_PATH     ${EXIV2_ENABLE_WIN_UNICODE})

# TODO: Try to use the cmake function check_symbol_exists which is more robust
check_function_exists( gmtime_r EXV_HAVE_GMTIME_R )
check_function_exists( mmap     EXV_HAVE_MMAP )
check_function_exists( munmap   EXV_HAVE_MUNMAP )
check_function_exists( strerror     EXV_HAVE_STRERROR )
check_function_exists( strerror_r   EXV_HAVE_STRERROR_R )
check_function_exists( strtol       EXV_HAVE_STRTOL )
check_function_exists( timegm       EXV_HAVE_TIMEGM )

# TODO : Do something about EXV_STRERROR_R_CHAR_P

# TODO: This check should be removed and rely on the check done in findDependencies.cmake
check_include_file( "libintl.h" EXV_HAVE_LIBINTL_H )
check_include_file( "unistd.h"  EXV_HAVE_UNISTD_H )
check_include_file( "memory.h"  EXV_HAVE_MEMORY_H )
check_include_file( "process.h" EXV_HAVE_PROCESS_H )
check_include_file( "stdbool.h" EXV_HAVE_STDBOOL_H )
check_include_file( "stdint.h"  EXV_HAVE_STDINT_H )
check_include_file( "strings.h" EXV_HAVE_STRINGS_H )
check_include_file( "sys/mman.h"    EXV_HAVE_SYS_MMAN_H )
check_include_file( "sys/stat.h"    EXV_HAVE_SYS_STAT_H )
check_include_file( "sys/types.h"   EXV_HAVE_SYS_TYPES_H )
check_include_file( "regex.h"       EXV_HAVE_REGEX_H )
check_include_file( "inttypes.h"    EXV_HAVE_INTTYPES_H )

if (NOT EXV_HAVE_LIBINTL_H)
    set(EXV_ENABLE_NLS 0)
endif()


include(CheckCSourceCompiles)
#####################################################################################
# strerror_r returns char*

# NOTE : reverting commit #2041, which break compilation under linux and windows

CHECK_C_SOURCE_COMPILES( "#include <string.h>
int main() {
char * c;
c = strerror_r(0,c,0);
return 0;
}" EXV_HAVE_DECL_STRERROR_R )

#####################################################################################

configure_file( config/config.h.cmake ${CMAKE_BINARY_DIR}/exv_conf.h @ONLY)
