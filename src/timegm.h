/*!
  @file    timegm.h
  @brief   Declaration of timegm(). The implementation is in localtime.c
*/
#pragma once
#include <ctime>

#ifdef  __cplusplus	
extern "C" {	
#endif

// The UTC version of mktime
/* timegm is replaced with _mkgmtime on Windows (msvc && mingw) */
#if defined(_MSC_VER)
#define timegm _mkgmtime
#endif

#if defined(__MINGW__)
time_t timegm(struct tm * const tmp);
#endif

#ifdef  __cplusplus	
}	
#endif

