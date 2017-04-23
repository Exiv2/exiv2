// config.h

#pragma once
#ifndef _CONFIG_H_
#define _CONFIG_H_

///// Start of Visual Studio Support /////
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

// Constants required by Microsoft SDKs to define SHGetFolderPathA and others

#ifndef _WIN32_WINNT
// Visual Studio 2012 and earlier
# if _MSC_VER < 1800
#  define _WIN32_WINNT 0x0501
# else
#  define _WIN32_WINNT 0x0600
# endif
#endif

#if _MSC_VER >= _MSC_VER_2010
# define EXV_HAVE_STDINT_H 1
#else
# ifdef  EXV_HAVE_STDINT_H
#  undef EXV_HAVE_STDINT_H
# endif
#endif

#if _MSC_VER_ == _MSC_VER_2012
#define HAVE_NTOHLL 1
#endif

/* Disable warning 4251 from std templates about exporting interfaces */
#ifdef  EXV_HAVE_DLL
#pragma warning( disable : 4251 )
#endif

/* Disable warnings about 'deprecated' standard functions */
#if _MSC_VER >= _MSC_VER_2005
# pragma warning(disable : 4996)
#endif

/* On Microsoft compilers pid_t has to be set to int. */
#ifndef HAVE_PID_T
typedef int pid_t;
#endif

/* Help out our buddy curl */
#if !defined(EXV_HAVE_DLL)
# define CURL_STATICLIB
#endif

#endif // _MSC_VER
///// End of Visual Studio Support /////

///// Include exv_platform.h file  /////
#ifdef _MSC_VER
# ifdef EXV_MSVC_CONFIGURE
#  include "exv_msvc_configure.h"
# else
#  include "exv_msvc.h"
# endif
#else
# include "exv_conf.h"
#endif
////////////////////////////////////////

///// End symbol visibility /////////
#if defined(__CYGWIN32__) && !defined(__CYGWIN__)
   /* For backwards compatibility with Cygwin b19 and
      earlier, we define __CYGWIN__ here, so that
      we can rely on checking just for that macro. */
# define __CYGWIN__  __CYGWIN32__
# define EXV_HAVE_GXXCLASSVISIBILITY
#endif
#ifdef WIN32
# define EXV_IMPORT __declspec(dllimport)
# define EXV_EXPORT __declspec(dllexport)
# define EXV_DLLLOCAL
# define EXV_DLLPUBLIC
#else
# ifdef EXV_WANT_VISIBILITY_SUPPORT
#  if defined(__GNUC__) && (__GNUC__ >= 4)
#   define EXV_IMPORT __attribute__ ((visibility("default")))
#   define EXV_EXPORT __attribute__ ((visibility("default")))
#   define EXV_DLLLOCAL __attribute__ ((visibility("hidden")))
#   define EXV_DLLPUBLIC __attribute__ ((visibility("default")))
#  elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#   define EXV_IMPORT __global
#   define EXV_EXPORT __global
#   define EXV_DLLLOCAL __hidden
#   define EXV_DLLPUBLIC __global
#  else
#   define EXV_IMPORT
#   define EXV_EXPORT
#   define EXV_DLLLOCAL
#   define EXV_DLLPUBLIC
#  endif
# else /* ! EXV_WANT_VISIBILITY_SUPPORT */
#  define EXV_IMPORT
#  define EXV_EXPORT
#  define EXV_DLLLOCAL
#  define EXV_DLLPUBLIC
# endif /* ! EXV_WANT_VISIBILITY_SUPPORT */
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

///// End symbol visibility /////////

///// Start of platform marcos /////////
// Linux GCC 4.8 appears to be confused about strerror_r
#if !defined(EXV_STRERROR_R_CHAR_P) &&  defined( __gnu_linux__) && defined(__GLIBC__)
#define EXV_STRERROR_R_CHAR_P
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
# ifndef  __MING__
#  define __MING__  1
# endif
# ifndef  __MINGW__
#  define __MINGW__ 1
# endif
// Don't know why MinGW refuses to link libregex
# ifdef  EXV_HAVE_REGEX
#  undef EXV_HAVE_REGEX
# endif
#ifdef EXV_UNICODE_PATH
#error EXV_UNICODE_PATH is not supported for MinGW builds
#endif
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


# include <stdio.h>
#ifdef   EXV_HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef __cplusplus
#include <string>
#include <iostream>
#endif

//
// That's all Folks!
#endif // _CONFIG_H_
