/* ***************************************************************** -*- C -*- */
/*!
  @file    exv_msvc.h
  @brief   Configuration settings for MSVC
  @version $Rev: 2320 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    07-Feb-04, ahu: created
 */

#ifdef  _MSC_VER
#ifndef _EXV_MSVC_H_
#define _EXV_MSVC_H_

/* Define to 1 if you have the <regex.h> header file. */
// #define EXV_HAVE_REGEX 1

/* Define to 1 if you have the <process.h> header file. */
#define EXV_HAVE_PROCESS_H 1

/* Define to the address where bug reports for this package should be sent. */
#define EXV_PACKAGE_BUGREPORT "ahuggel@gmx.net"

/* Define to the full name of this package. */
#define EXV_PACKAGE_NAME "exiv2"

/* Define to the full name and version of this package. */
#define EXV_PACKAGE_STRING "exiv2 0.26"

/* Define to the one symbol short name of this package. */
#define EXV_PACKAGE_TARNAME "exiv2"

/* Define to the version of this package. */
#define EXV_PACKAGE_VERSION "0.26"

/* Define to `int' if <sys/types.h> does not define pid_t. */
typedef int pid_t;

#ifndef EXV_COMMERCIAL_VERSION

/* Define to 1 to enable translation of Nikon lens names. */
# define EXV_HAVE_LENSDATA 1

/* Define to 1 if translation of program messages to the user's
   native language is requested. */
# undef EXV_ENABLE_NLS

#endif /* !EXV_COMMERCIAL_VERSION */

/* Define to 1 if you have the `iconv' function. */
# undef EXV_HAVE_ICONV

#ifdef  EXV_HAVE_ICONV
#define EXV_ICONV_CONST const
#endif

/* Define as 1 if you have the `zlib' library. (0 to omit zlib) [png support] */
#define HAVE_LIBZ 1

#if HAVE_LIBZ
  #define EXV_HAVE_LIBZ 1
  // assist VC7.1 to compile vsnprintf
  #if (_MSC_VER < 1400) && !defined(vsnprintf)
  #define vsnprintf _vsnprintf
  #endif
#else
  #define EXV_HAVE_LIBZ 0
#endif

/* Define to 1 if you have the Adobe XMP Toolkit. */
#define EXV_HAVE_XMP_TOOLKIT 1

/* Define to 1 to enable video/audio metadata support. */
# undef EXV_ENABLE_VIDEO

/* Define to 1 to enable webready support. */
#define EXV_ENABLE_WEBREADY 1

/* Define as 1 if you want to use curl to perform http I/O */
#ifndef EXV_USE_CURL
#define EXV_USE_CURL 1
#endif

/* Define as 1 if you want to use libssh (for ssh:// and sftp:// support */
#ifndef EXV_USE_SSH
#define EXV_USE_SSH 1
#endif

/* Help out our buddy curl */
#ifndef EXV_HAVE_DLL
#define CURL_STATICLIB
#endif

/* Windows unicode path support */
// #define EXV_UNICODE_PATH

/* Define to 1 if you have the `mmap' function. */
/* #undef EXV_HAVE_MMAP */

/* Define to 1 if you have the `munmap' function. */
/* #undef EXV_HAVE_MUNMAP */


#endif /* _EXV_MSVC_H_ */
#endif /* _MSC_VER     */
