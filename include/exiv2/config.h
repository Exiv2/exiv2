#pragma once

#ifdef  _MSC_VER
#pragma warning(disable : 4996) // Disable warnings about 'deprecated' standard functions
#pragma warning(disable : 4251) // Disable warnings from std templates about exporting interfaces
#endif // _MSC_VER

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
