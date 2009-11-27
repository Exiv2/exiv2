/* ***************************************************************** -*- C -*- */
/*!
  @file    exv_msvc.h
  @brief   Configuration settings for MSVC
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    07-Feb-04, ahu: created
           26-Feb-05, ahu: renamed and moved to src directory
 */

/* Todo: The PACKAGE_* defines should be generated */

#ifdef _MSC_VER

/* Define to 1 if you have the <process.h> header file. */
#define EXV_HAVE_PROCESS_H 1

/* Define to the address where bug reports for this package should be sent. */
#define EXV_PACKAGE_BUGREPORT "ahuggel@gmx.net"

/* Define to the full name of this package. */
#define EXV_PACKAGE_NAME "exiv2"

/* Define to the full name and version of this package. */
#define EXV_PACKAGE_STRING "exiv2 0.18.2"

/* Define to the one symbol short name of this package. */
#define EXV_PACKAGE_TARNAME "exiv2"

/* Define to the version of this package. */
#define EXV_PACKAGE_VERSION "0.18.2"

/* Define to `int' if <sys/types.h> does not define pid_t. */
typedef int pid_t;

#ifndef EXV_COMMERCIAL_VERSION

/* Define to 1 to enable translation of Exif.Nikon3.LensData. */
# define EXV_HAVE_LENSDATA 1

/* Define to 1 if translation of program messages to the user's
   native language is requested. */
# undef EXV_ENABLE_NLS

/* Define to 1 if you have the `iconv' function. */
# undef EXV_HAVE_ICONV

/* Define to 1 to enable conversion of UCS2 encoded Windows tags to UTF-8. */
# undef EXV_HAVE_PRINTUCS2

#endif /* !EXV_COMMERCIAL_VERSION */

/* Define as 1 if you have the `zlib' library. (0 to omit zlib) [png support] */
#define HAVE_LIBZ 1

#if HAVE_LIBZ
  #define EXV_HAVE_LIBZ
  // assist VC7.1 to compile vsnprintf
  #if (_MSC_VER < 1400) && !defined(vsnprintf)
  #define vsnprintf _vsnprintf
  #endif
#endif

/* Define to 1 if you have the Adobe XMP Toolkit. */
#define EXV_HAVE_XMP_TOOLKIT 1

/* File path seperator */
#define EXV_SEPERATOR_STR "\\"
#define EXV_SEPERATOR_CHR '\\'

/* Windows unicode path support */
#define EXV_UNICODE_PATH

/* Define to 1 if you have the "sys/mman.h header file (and supporting code of course) */
/* At this time (between 0.18.1 and 0.19) this is used by TIFF files to avoid reading  */
/* the total file into memory returning in a 6x improvement in exiv2 on 2mb tiff files */
#define EXV_HAVE_SYS_MMAN_H 1
#ifdef  EXV_HAVE_SYS_MMAN_H
#if     EXV_HAVE_SYS_MMAN_H
#define EXV_HAVE_MMAP       1
#define EXV_HAVE_MUNMAP	    1
#endif
#endif

/* Shared library support */
#ifdef  EXV_HAVE_DLL
#define EXV_IMPORT __declspec(dllimport)
#define EXV_EXPORT __declspec(dllexport)
#define EXV_DLLLOCAL
#define EXV_DLLPUBLIC
#else
#define EXV_IMPORT
#define EXV_EXPORT
#define EXV_DLLLOCAL
#define EXV_DLLPUBLIC
#define EXIV2API
#endif

/* Define EXIV2API for DLL builds */
#ifdef   EXV_HAVE_DLL
#  ifdef EXV_BUILDING_LIB
#    define EXIV2API EXV_EXPORT
#  else
#    define EXIV2API EXV_IMPORT
#  endif /* ! EXV_BUILDING_LIB */
#else
#  define EXIV2API
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
#if _MSC_VER >= 1400
# pragma warning(disable : 4996)
#endif

#endif /* _MSC_VER */
