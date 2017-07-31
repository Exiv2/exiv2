# Note that the scope of the EXV_ variables in local
if (${EXIV2_ENABLE_WEBREADY})
    set(EXV_USE_SSH   ${EXIV2_ENABLE_SSH})
    set(EXV_USE_CURL  ${EXIV2_ENABLE_CURL})
endif()
set(EXV_ENABLE_VIDEO     ${EXIV2_ENABLE_VIDEO})
set(EXV_ENABLE_WEBREADY  ${EXIV2_ENABLE_WEBREADY})
set(EXV_HAVE_LENSDATA    ${EXIV2_ENABLE_LENSDATA})

set(EXV_PACKAGE          ${PACKAGE})
set(EXV_PACKAGE_VERSION  ${PACKAGE_VERSION})
set(EXV_PACKAGE_STRING   ${PACKAGE_STRING})
set(EXV_HAVE_XMP_TOOLKIT ${HAVE_XMP_TOOLKIT})
set(EXV_HAVE_ICONV       ${ICONV_FOUND})
set(EXV_HAVE_LIBZ        ${ZLIB_FOUND})

# TODO: Try to use the cmake function check_symbol_exists which is more robust
check_function_exists( gmtime_r EXV_HAVE_GMTIME_R )

# TODO: This check should be removed and rely on the check done in findDependencies.cmake
check_include_file( "libintl.h" EXV_HAVE_LIBINTL_H )
check_include_file( "unistd.h"  EXV_HAVE_UNISTD_H )

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

configure_file( config/config.h.cmake ${CMAKE_SOURCE_DIR}/include/exiv2/exv_conf.h @ONLY)
#configure_file( config/config.h.cmake ${CMAKE_SOURCE_DIR}/include/exiv2/exv_conf.h)
