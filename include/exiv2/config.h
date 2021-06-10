// config.h

#ifndef _CONFIG_H_
#define _CONFIG_H_

///// Start of Visual Studio Support /////
#ifdef  _MSC_VER

#define _MSC_VER_2010 1600
#define _MSC_VER_2008 1500

// Constants required by Microsoft SDKs to define SHGetFolderPathA and others

#ifndef _WIN32_WINNT
// Visual Studio 2012 and earlier
# if _MSC_VER < 1800
#  define _WIN32_WINNT 0x0501
# else
#  define _WIN32_WINNT 0x0600
# endif
#endif

#if _MSC_VER >= _MSC_VER_2008
#pragma warning(disable : 4996) // Disable warnings about 'deprecated' standard functions
#pragma warning(disable : 4251) // Disable warnings from std templates about exporting interfaces
#endif

/* On Microsoft compilers pid_t has to be set to int. */
#ifndef HAVE_PID_T
using pid_t = int;
#endif

#endif // _MSC_VER
///// End of Visual Studio Support /////

#include "exv_conf.h"
////////////////////////////////////////

///// Start of platform marcos /////////
#if defined(__MINGW32__) || defined(__MINGW64__)
# ifndef  __MING__
#  define __MING__  1
# endif
# ifndef  __MINGW__
#  define __MINGW__ 1
# endif
#endif

#ifndef __CYGWIN__
# if defined(__CYGWIN32__) || defined(__CYGWIN64__)
#  define __CYGWIN__ 1
# endif
#endif

#ifndef __LITTLE_ENDIAN__
# if    defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
#  if            __BYTE_ORDER__  ==         __ORDER_LITTLE_ENDIAN__
#   define __LITTLE_ENDIAN__ 1
#  endif
# endif
#endif

#ifndef __LITTLE_ENDIAN__
# if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW__)
#  define __LITTLE_ENDIAN__ 1
# endif
#endif

/*
  If you're using Solaris and the Solaris Studio compiler
  you must -library=stdcxx4 along with these inclusions below
*/
#if defined(OS_SOLARIS)
# include <string.h>
# include <strings.h>
# include <math.h>
# if defined(__cplusplus)
#  include <ios>
#  include <fstream>
# endif
#endif
///// End of platform marcos /////////

///// Path separator marcos      /////
#ifndef EXV_SEPARATOR_STR
# if defined(WIN32) && !defined(__CYGWIN__)
#  define EXV_SEPARATOR_STR "\\"
#  define EXV_SEPARATOR_CHR '\\'
# else
#  define EXV_SEPARATOR_STR "/"
#  define EXV_SEPARATOR_CHR '/'
# endif
#endif
//////////////////////////////////////


// https://softwareengineering.stackexchange.com/questions/291141/how-to-handle-design-changes-for-auto-ptr-deprecation-in-c11
#if __cplusplus >= 201103L
  #include <memory>
  #include <sys/types.h>
  #ifndef  _MSC_VER
    #include <unistd.h>
  #endif
  template <typename T>
  using auto_ptr = std::unique_ptr<T>;
#endif

#endif // _CONFIG_H_
