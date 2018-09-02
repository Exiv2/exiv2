/*!
  @file    timegm.h
  @brief   Declaration of timegm(). The implementation is in localtime.c
*/
#pragma once
#include <time.h>

// The UTC version of mktime
/* timegm is replaced with _mkgmtime on Windows (msvc && mingw) */
#if defined(__MINGW__) || defined(_MSC_VER)
#define timegm _mkgmtime
#endif
