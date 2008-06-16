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
#define EXV_PACKAGE_STRING "exiv2 0.17.1"

/* Define to the one symbol short name of this package. */
#define EXV_PACKAGE_TARNAME "exiv2"

/* Define to the version of this package. */
#define EXV_PACKAGE_VERSION "0.17.1"

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

/* Define to 1 if you have the `zlib' library. */
#undef HAVE_LIBZ

/* Define to 1 if you have the Adobe XMP Toolkit. */
#define EXV_HAVE_XMP_TOOLKIT 1

/* File path seperator */
#define EXV_SEPERATOR_STR "\\"
#define EXV_SEPERATOR_CHR '\\'

/*
  Visual Studio C++ 2005 (8.0)
  Disable warnings about 'deprecated' standard functions
  See, eg. http://www.informit.com/guides/content.asp?g=cplusplus&seqNum=259
*/
#if _MSC_VER >= 1400
# pragma warning(disable : 4996)
#endif

#endif /* _MSC_VER */
