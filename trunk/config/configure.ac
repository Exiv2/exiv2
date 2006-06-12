#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Exiv2, 0.10, ahuggel@gmx.net)
AC_CONFIG_SRCDIR([src/exif.cpp])
AC_CONFIG_HEADER([./config/config.h])
AC_CONFIG_AUX_DIR([./config])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Libtool support
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB(z, zlibVersion, , AC_ERROR([zlib not installed]))

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h malloc.h stdint.h stdlib.h string.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRERROR_R
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gmtime_r memset strchr strerror strtol])
AC_CHECK_FUNCS([timegm], HAVE_TIMEGM=1)
AC_SUBST(HAVE_TIMEGM,$HAVE_TIMEGM)

AC_SUBST(GXX,$GXX)
AC_SUBST(GCC,$GCC)

AC_ARG_ENABLE(dependency-tracking,
[  --disable-dependency-tracking  speeds up one-time build
  --enable-dependency-tracking   do not reject slow dependency extractors],
[if test $enableval = yes; then DEP_TRACKING=1; fi],DEP_TRACKING=1)

AC_SUBST(DEP_TRACKING,$DEP_TRACKING)

AC_CONFIG_FILES([./config/config.mk])
AC_CONFIG_FILES([./config/exiv2-config], [chmod +x ./config/exiv2-config])
AC_CONFIG_FILES([./config/exiv2.pc])
AC_OUTPUT
