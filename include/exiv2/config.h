// config.h

#ifndef _CONFIG_H_
#define _CONFIG_H_

///// Start of Visual Studio Support /////
#ifdef  _MSC_VER

#define _MSC_VER_2010 1600
#define _MSC_VER_2008 1500
#define _MSC_VER_2005 1400

#if  _MSC_VER >= _MSC_VER_2005
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define NOMINMAX
#include <windows.h>

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

#endif // _CONFIG_H_
