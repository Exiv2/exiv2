// ******************************************************************* -*- C -*-
/*!
  @file    exv_msvc.hpp
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
#define HAVE_PROCESS_H 1

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "ahuggel@gmx.net"

/* Define to the full name of this package. */
#define PACKAGE_NAME "Exiv2"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "Exiv2 0.6.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "exiv2"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.6.1"

/* Define to `int' if <sys/types.h> does not define pid_t. */
#define pid_t int

/* File path seperator */
#define SEPERATOR_STR "\\"
#define SEPERATOR_CHR '\\'

#endif /* _MSC_VER */
