#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Exiv2, 0.12, ahuggel@gmx.net)
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)
AC_CONFIG_SRCDIR([src/exif.cpp])
AC_CONFIG_HEADER([./config/config.h])
AC_CONFIG_AUX_DIR([./config])

# ---------------------------------------------------------------------------
# Checks for programs.
# ---------------------------------------------------------------------------
AC_PROG_CXX
AC_PROG_CXXCPP
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

#dnl We cannot use AC_DEFINE_UNQUOTED() for these definitions, as
#dnl we require make to do insert the proper $(datadir) value
#localedir="\$(datadir)/locale"
#AC_SUBST(localedir)
#AM_CPPFLAGS="$CPPFLAGS"
#AM_CPPFLAGS="$AM_CPPFLAGS -DLOCALEDIR=\\\"${localedir}\\\""
#AC_SUBST(AM_CPPFLAGS)

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

# ---------------------------------------------------------------------------
# Checks for header files.
# ---------------------------------------------------------------------------
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h malloc.h stdint.h stdlib.h string.h unistd.h wchar.h sys/mman.h])

# ---------------------------------------------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
# ---------------------------------------------------------------------------
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

# ---------------------------------------------------------------------------
# Checks for library functions.
# ---------------------------------------------------------------------------
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRERROR_R
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gmtime_r memset strchr strerror strtol mmap munmap])
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
AC_CONFIG_FILES([./po/Makefile.in])
AC_OUTPUT
