#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Exiv2, 0.14, ahuggel@gmx.net)
EXIV2_LTVERSION=0:0:0
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_SUBST(EXIV2_LTVERSION)
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)
AC_CONFIG_SRCDIR([src/exif.cpp])
AC_CONFIG_HEADER([./config/config.h])
AC_CONFIG_AUX_DIR([./config])

# ---------------------------------------------------------------------------
# Checks for programs.
# ---------------------------------------------------------------------------
# Don't use -g to compile C++ code
ac_cv_prog_cxx_g=no
AC_PROG_CXX
AC_PROG_CXXCPP
# Don't use -g to compile C code
ac_cv_prog_cc_g=no
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# ---------------------------------------------------------------------------
# i18n support
# ---------------------------------------------------------------------------
AM_GNU_GETTEXT([external])

# ---------------------------------------------------------------------------
# Libtool support
# ---------------------------------------------------------------------------
AC_PROG_LIBTOOL

# ---------------------------------------------------------------------------
# Checks for libraries.
# ---------------------------------------------------------------------------
AC_CHECK_LIBM()
LIBS="$LIBS $LIBM"
CHECK_ZLIB()
AC_SUBST(HAVE_LIBZ,$HAVE_LIBZ)

AC_MSG_CHECKING([whether to use libiconv])
AC_ARG_ENABLE(libiconv,
    [  --disable-libiconv      do not use libiconv to translate Windows XP tags ],
    USE_LIBICONV=$enableval, USE_LIBICONV=yes)
AC_MSG_RESULT($USE_LIBICONV)
if test "$USE_LIBICONV" = "yes"; then
    AM_ICONV
fi

# ---------------------------------------------------------------------------
# Checks for header files.
# ---------------------------------------------------------------------------
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h locale.h malloc.h stdint.h stdlib.h string.h unistd.h sys/mman.h utime.h])

# ---------------------------------------------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
# ---------------------------------------------------------------------------
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM
#AC_TYPE_UINT8_T
#AC_TYPE_UINT16_T
#AC_TYPE_UINT32_T
#AC_TYPE_INT16_T
#AC_TYPE_INT32_T
AC_SUBST(GXX,$GXX)
AC_SUBST(GCC,$GCC)

# ---------------------------------------------------------------------------
# Checks for library functions.
# ---------------------------------------------------------------------------
AC_FUNC_ALLOCA
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_STAT
AC_FUNC_STRERROR_R
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gmtime_r memset strchr strerror strtol mmap munmap])
AC_CHECK_FUNCS([timegm], HAVE_TIMEGM=1)
AC_SUBST(HAVE_TIMEGM,$HAVE_TIMEGM)

# ---------------------------------------------------------------------------
# Miscellaneous
# ---------------------------------------------------------------------------
AC_MSG_CHECKING([whether to enable dependency tracking])
AC_ARG_ENABLE(dependency-tracking,
    [  --disable-dependency-tracking  speeds up one-time build
  --enable-dependency-tracking   do not reject slow dependency extractors],
    USE_TRACKING=$enableval, USE_TRACKING=yes)
AC_MSG_RESULT($USE_TRACKING)
DEP_TRACKING=
if test "$USE_TRACKING" = "yes"; then
    DEP_TRACKING=1
fi
AC_SUBST(DEP_TRACKING,$DEP_TRACKING)

AC_MSG_CHECKING([whether to use the lens database for Exif.Nikon3.LensData])
AC_ARG_ENABLE(lensdata,
    [  --disable-lensdata      do not use the lens database for Exif.Nikon3.LensData ],
    USE_LENSDATA=$enableval, USE_LENSDATA=yes)
AC_MSG_RESULT($USE_LENSDATA)
if test "$USE_LENSDATA" = "yes"; then
    AC_DEFINE(HAVE_LENSDATA,1)
fi

if test "$enable_shared" = "no"; then
    EXV_LIB_STATIC=$LTLIBICONV
else
    EXV_LIB_STATIC=
fi
AC_SUBST(EXV_LIB_STATIC)

# ---------------------------------------------------------------------------
# Create output files.
# ---------------------------------------------------------------------------
AC_CONFIG_FILES([./config/config.mk])
AC_CONFIG_FILES([./config/exiv2.pc])
AC_CONFIG_FILES([./po/Makefile.in])
AC_CONFIG_FILES([./src/doxygen.hpp])
AC_OUTPUT
