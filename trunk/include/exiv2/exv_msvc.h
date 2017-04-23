/**/
/* exv_msvc.h */

#pragma once
#ifndef _EXV_MSVC_H_
#define _EXV_MSVC_H_

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
/* #define EXV_UNICODE_PATH */


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
  #if (_MSC_VER < 1400)// && !defined(vsnprintf)
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
#define EXV_PACKAGE_STRING "exiv2 0.26"

/* Define to the one symbol short name of this package. */
#define EXV_PACKAGE_TARNAME "exiv2"

/* Define to the version of this package. */
#define EXV_PACKAGE_VERSION "0.26"

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

/* On Microsoft compilers pid_t has to be set to int. */
#ifndef HAVE_PID_T
typedef int pid_t;
#endif

/* Define to rpl_realloc if the replacement function should be used. */
/* #undef realloc */

/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */

#endif // _EXV_CONF_H_
