// File generated by cmake from cmake/config.h.cmake.

#ifndef _EXV_CONF_H_
#define _EXV_CONF_H_

// Define to 1 if you want to use libcurl in httpIO.
#cmakedefine EXV_USE_CURL

// Define if you require webready support.
#cmakedefine EXV_ENABLE_WEBREADY

// Define if you have the <libintl.h> header file.
#cmakedefine EXV_HAVE_LIBINTL_H

// Define if you want translation of program messages to the user's native language
#cmakedefine EXV_ENABLE_NLS

// Define if you want BMFF support.
#cmakedefine EXV_ENABLE_BMFF

// Define if you have the strerror_r function.
#cmakedefine EXV_HAVE_STRERROR_R

// Define if the strerror_r function returns char*.
#cmakedefine EXV_STRERROR_R_CHAR_P

// Define to enable the Windows unicode path support.
#cmakedefine EXV_UNICODE_PATH

/* Define to `const' or to empty, depending on the second argument of `iconv'. */
#cmakedefine ICONV_ACCEPTS_CONST_INPUT
#if defined(ICONV_ACCEPTS_CONST_INPUT) || defined(__NetBSD__)
#define EXV_ICONV_CONST const
#else
#define EXV_ICONV_CONST
#endif

// Define if you have the mmap function.
#cmakedefine EXV_HAVE_MMAP

// Define if you have the munmap function.
#cmakedefine EXV_HAVE_MUNMAP

/* Define if you have the <libproc.h> header file. */
#cmakedefine EXV_HAVE_LIBPROC_H

/* Define if you have the <unistd.h> header file. */
#cmakedefine EXV_HAVE_UNISTD_H

// Define if you have the <sys/mman.h> header file.
#cmakedefine EXV_HAVE_SYS_MMAN_H

// Define if you have are using the zlib library.
#cmakedefine EXV_HAVE_LIBZ

/* Define if you have (Exiv2/xmpsdk) Adobe XMP Toolkit. */
#cmakedefine EXV_HAVE_XMP_TOOLKIT

/* Define to the full name of this package. */
#cmakedefine EXV_PACKAGE_NAME "@EXV_PACKAGE_NAME@"

/* Define to the full name and version of this package. */
#cmakedefine EXV_PACKAGE_STRING "@EXV_PACKAGE_STRING@"

/* Define to the version of this package. */
#cmakedefine EXV_PACKAGE_VERSION "@PROJECT_VERSION@"

#define EXIV2_MAJOR_VERSION (@PROJECT_VERSION_MAJOR@)
#define EXIV2_MINOR_VERSION (@PROJECT_VERSION_MINOR@)
#define EXIV2_PATCH_VERSION (@PROJECT_VERSION_PATCH@)
#define EXIV2_TWEAK_VERSION (@PROJECT_VERSION_TWEAK@)

// Definition to enable translation of Nikon lens names.
#cmakedefine EXV_HAVE_LENSDATA

// Define if you have the iconv function.
#cmakedefine EXV_HAVE_ICONV

// Definition to enable conversion of UCS2 encoded Windows tags to UTF-8.
#cmakedefine EXV_HAVE_PRINTUCS2

#endif /* !_EXV_CONF_H_ */
