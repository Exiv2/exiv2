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
#define EXV_PACKAGE_NAME "Exiv2"

/* Define to the full name and version of this package. */
#define EXV_PACKAGE_STRING "Exiv2 0.12"

/* Define to the one symbol short name of this package. */
#define EXV_PACKAGE_TARNAME "exiv2"

/* Define to the version of this package. */
#define EXV_PACKAGE_VERSION "0.12"

/* Define to `int' if <sys/types.h> does not define pid_t. */
#define pid_t int

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
