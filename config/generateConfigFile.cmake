# Note that the scope of the EXV_ variables in local
if (${EXIV2_ENABLE_WEBREADY})
    set(EXV_USE_SSH   ${EXIV2_ENABLE_SSH})
    set(EXV_USE_CURL  ${EXIV2_ENABLE_CURL})
endif()
set(EXV_ENABLE_VIDEO     ${EXIV2_ENABLE_VIDEO})

# TODO: Try to use the cmake function check_symbol_exists which is more robust
check_function_exists( gmtime_r EXV_HAVE_GMTIME_R )

# TODO: This check should be removed and rely on the check done in findDependencies.cmake
check_include_file( "libintl.h" EXV_HAVE_LIBINTL_H )

if (NOT EXV_HAVE_LIBINTL_H)
    set(EXV_ENABLE_NLS 0)
endif()

configure_file( config/config.h.cmake ${CMAKE_SOURCE_DIR}/include/exiv2/exv_conf.h @ONLY)
