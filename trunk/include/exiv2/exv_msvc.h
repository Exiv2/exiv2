/**/
/* exv_msvc.h */

#ifndef _EXV_MSVC_H_
#define _EXV_MSVC_H_

#ifdef  _MSC_VER
#define _MSC_VER_2015 1900
#define _MSC_VER_2013 1800
#define _MSC_VER_2012 1700
#define _MSC_VER_2010 1600
#define _MSC_VER_2008 1500
#define _MSC_VER_2005 1400
#define _MSC_VER_2003 1300

#if    _MSC_VER >= _MSC_VER_2015
#define MSDEV_2015    1
#elif  _MSC_VER >= _MSC_VER_2013
#define MSDEV_2013    1
#elif  _MSC_VER >= _MSC_VER_2012
#define MSDEV_2012    1
#elif  _MSC_VER >= _MSC_VER_2010
#define MSDEV_2010    1
#elif  _MSC_VER >= _MSC_VER_2008
#define MSDEV_2008    1
#elif  _MSC_VER >= _MSC_VER_2005
#define MSDEV_2005    1
#else
#define MSDEV_2003    1
#endif

#endif /* _EXV_MSVC_H_ */
#endif /* _MSC_VER     */

// Visual Studio 2012 supports ntohll
#if defined(_MSC_VER) && _MSC_VER_ == _MSC_VER_2012
#define HAVE_NTOHLL 1
#endif


/* Define to 1 if you want to use `libssh' for SshIO. */
/* #undef EXV_USE_SSH */

/* Define to 1 if you want to use `libcurl' in httpIO. */
/* #undef EXV_USE_CURL */

/* Define to 1 if you have the `alarm' function. */
/* #undef EXV_HAVE_ALARM */

/* Define to 1 if you have the declaration of `strerror_r', and to 0 if you
   don't. */
/* #undef EXV_HAVE_DECL_STRERROR_R */

/* Define to 1 if you have the `gmtime_r' function. */
/* #undef EXV_HAVE_GMTIME_R */

/* Define to 1 if you have the <inttypes.h> header file. */
/* #undef EXV_HAVE_INTTYPES_H */

/* Define to 1 if you have the <libintl.h> header file. */
#define EXV_HAVE_LIBINTL_H 1

/* Define to 1 if you require video support. */
/* #undef EXV_ENABLE_VIDEO */

/* Define to 1 if you require webready support. */
/* #undef EXV_ENABLE_WEBREADY */

#ifndef EXV_COMMERCIAL_VERSION

/* Define to 1 to enable translation of Nikon lens names. */
#define EXV_HAVE_LENSDATA 1

/* Define to 1 if translation of program messages to the user's
   native language is requested. */
/* #undef EXV_ENABLE_NLS */

/* Define to 1 if you have the `iconv' function. */
/* #undef EXV_HAVE_ICONV */

/* Define to 1 to enable conversion of UCS2 encoded Windows tags to UTF-8. */
#define EXV_HAVE_PRINTUCS2 1

#endif /* !EXV_COMMERCIAL_VERSION */

/* Define Windows unicode path support. */
/* #define EXV_UNICODE_PATH 1 */

/* Define to `const' or to empty, depending on the second argument of `iconv'. */
/* #undef ICONV_ACCEPTS_CONST_INPUT */
#if defined ICONV_ACCEPTS_CONST_INPUT
#define EXV_ICONV_CONST const
#else
#define EXV_ICONV_CONST
#endif

/* Define to 1 if you have the <regex.h> header file. */
/* #undef EXV_HAVE_REGEX */

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
/* #undef EXV_HAVE_MALLOC */

/* Define to 1 if you have the <malloc.h> header file. */
/* #undef EXV_HAVE_MALLOC_H */

/* Define to 1 if you have the <memory.h> header file. */
/* #undef EXV_HAVE_MEMORY_H */

/* Define to 1 if you have the `memset' function. */
#define EXV_HAVE_MEMSET 1

/* Define to 1 if your system has a GNU libc compatible `realloc' function,
   and to 0 otherwise. */
#define EXV_HAVE_REALLOC 1

/* Define to 1 if stdbool.h conforms to C99. */
/* #undef EXV_HAVE_STDBOOL_H */

/* Define to 1 if you have the <stdint.h> header file. */
/* #undef EXV_HAVE_STDINT_H */

/* Define to 1 if you have the <stdlib.h> header file. */
/* #undef EXV_HAVE_STDLIB_H */

/* Define to 1 if you have the `strchr' function. */
#define EXV_HAVE_STRCHR 1

/* Define to 1 if you have the `strerror' function. */
#define EXV_HAVE_STRERROR 1

/* Define to 1 if you have the `strerror_r' function. */
/* #undef EXV_HAVE_STRERROR_R */

/* Define to 1 if you have the <strings.h> header file. */
/* #undef EXV_HAVE_STRINGS_H */

/* Define to 1 if you have the <string.h> header file. */
/* #undef EXV_HAVE_STRING_H */

/* Define to 1 if you have the `strtol' function. */
#define EXV_HAVE_STRTOL 1

/* Define to 1 if you have the `mmap' function. */
/* #undef EXV_HAVE_MMAP */

/* Define to 1 if you have the `munmap' function. */
/* #undef EXV_HAVE_MUNMAP */

/* Define to 1 if you have the <sys/stat.h> header file. */
/* #undef EXV_HAVE_SYS_STAT_H */

/* Define to 1 if you have the <sys/time.h> header file. */
/* #undef EXV_HAVE_SYS_TIME_H */

/* Define to 1 if you have the <sys/types.h> header file. */
/* #undef EXV_HAVE_SYS_TYPES_H */

/* Define to 1 if you have the `timegm' function. */
/* #undef EXV_HAVE_TIMEGM */

/* Define to 1 if you have the <unistd.h> header file. */
/* #undef EXV_HAVE_UNISTD_H */

/* Define to 1 if you have the `vprintf' function. */
#define EXV_HAVE_VPRINTF 1

/* Define to 1 if you have the <wchar.h> header file. */
/* #undef EXV_HAVE_WCHAR_H */

/* Define to 1 if you have the <sys/mman.h> header file. */
/* #undef EXV_HAVE_SYS_MMAN_H */

/* Define to 1 if you have the `zlib' library. */
#define EXV_HAVE_LIBZ 1

#if defined(_MSC_VER) && HAVE_LIBZ
  #if (_MSC_VER < 1400) && !defined(vsnprintf)
  #define vsnprintf _vsnprintf
  #endif
#endif

/* Define to 1 if you have the <process.h> header file. */
#define EXV_HAVE_PROCESS_H 1

/* Define to 1 if you have the Adobe XMP Toolkit. */
#define EXV_HAVE_XMP_TOOLKIT 1

/* Define to 1 if the system has the type `_Bool'. */
/* #undef EXV_HAVE__BOOL */

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
/* #undef EXV_LSTAT_FOLLOWS_SLASHED_SYMLINK */

/* Define to the address where bug reports for this package should be sent. */
#define EXV_PACKAGE_BUGREPORT "ahuggel@gmx.net"

/* Define to the name of this package. */
#define EXV_PACKAGE "exiv2"

/* Define to the full name of this package. */
#define EXV_PACKAGE_NAME "exiv2"

/* Define to the full name and version of this package. */
#define EXV_PACKAGE_STRING "exiv2 0.25"

/* Define to the one symbol short name of this package. */
#define EXV_PACKAGE_TARNAME "exiv2"

/* Define to the version of this package. */
#define EXV_PACKAGE_VERSION "0.25"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if strerror_r returns char *. */
/* #undef STRERROR_R_CHAR_P */

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to rpl_malloc if the replacement function should be used. */
/* #undef malloc */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef HAVE_PID_T */

/* On Microsoft compilers pid_t has to be set to int. */
#ifndef HAVE_PID_T
typedef int pid_t;
#endif

/* Define to rpl_realloc if the replacement function should be used. */
/* #undef realloc */

/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */

#if defined(__CYGWIN32__) && !defined(__CYGWIN__)
   /* For backwards compatibility with Cygwin b19 and
      earlier, we define __CYGWIN__ here, so that
      we can rely on checking just for that macro. */
#define __CYGWIN__  __CYGWIN32__
#define EXV_HAVE_GXXCLASSVISIBILITY
#endif

/* Help out our buddy curl */
#if defined(_MSC_VER) && !defined(EXV_HAVE_DLL)
    #define CURL_STATICLIB
#endif

#if defined(_MSC_VER)
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wldap32.lib")
#endif

/* File path separator */
#ifndef EXV_SEPARATOR_STR
#if defined(WIN32) && !defined(__CYGWIN__)
#define EXV_SEPARATOR_STR "\\"
#define EXV_SEPARATOR_CHR '\\'
#else
#define EXV_SEPARATOR_STR "/"
#define EXV_SEPARATOR_CHR '/'
#endif
#endif

/* Shared library support, see http://gcc.gnu.org/wiki/Visibility */
#if defined(WIN32) && !defined(__CYGWIN__) && EXV_HAVE_DLL
# define EXV_IMPORT __declspec(dllimport)
# define EXV_EXPORT __declspec(dllexport)
# define EXV_DLLLOCAL
# define EXV_DLLPUBLIC
#else
# ifdef EXV_HAVE_GXXCLASSVISIBILITY
#  define EXV_IMPORT __attribute__ ((visibility("default")))
#  define EXV_EXPORT __attribute__ ((visibility("default")))
#  define EXV_DLLLOCAL __attribute__ ((visibility("hidden")))
#  define EXV_DLLPUBLIC __attribute__ ((visibility("default")))
# else
#  define EXV_IMPORT
#  define EXV_EXPORT
#  define EXV_DLLLOCAL
#  define EXV_DLLPUBLIC
# endif /* ! EXV_HAVE_GXXCLASSVISIBILITY */
#endif /* ! WIN32 */

/* Define EXIV2API for DLL builds */
#ifdef EXV_HAVE_DLL
# ifdef EXV_BUILDING_LIB
#  define EXIV2API EXV_EXPORT
# else
#  define EXIV2API EXV_IMPORT
# endif /* ! EXV_BUILDING_LIB */
#else
# define EXIV2API
#endif /* ! EXV_HAVE_DLL */

/*
  Disable warning 4251.  This is warning from std templates about exporting interfaces
*/
#ifdef  EXV_HAVE_DLL
#pragma warning( disable : 4251 )
#endif

/*
  Visual Studio C++ 2005 (8.0)
  Disable warnings about 'deprecated' standard functions
  See, eg. http://www.informit.com/guides/content.asp?g=cplusplus&seqNum=259
*/
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
# pragma warning(disable : 4996)
#endif

// Constants required by Microsoft SDKs to define SHGetFolderPathA and others
#ifdef  _MSC_VER
# ifndef _WIN32_WINNT
// Visual Studio 2012 and earlier
#  if _MSC_VER < 1800
#   define _WIN32_WINNT 0x0501
#  else
#   define _WIN32_WINNT 0x0600
#  endif
# endif
# include <windows.h>
# include <shlobj.h>
#endif

/*
  If you're using Solaris and the Solaris Studio compiler
  you must -library=stdcxx4 along with these inclusions below
*/
#if defined(OS_SOLARIS)
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>
#if defined(__cplusplus)
#include <ios>
#include <fstream>
#endif
#endif
