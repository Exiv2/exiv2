# Note that the scope of the EXV_ variables in local
set(EXV_USE_SSH   ${EXIV2_ENABLE_SSH})
set(EXV_USE_CURL  ${EXIV2_ENABLE_CURL})

# TODO: Try to use check_symbol_exists
check_function_exists( gmtime_r EXV_HAVE_GMTIME_R )

configure_file( config/config.h.cmake ${CMAKE_SOURCE_DIR}/include/exiv2/exv_conf.h @ONLY)
