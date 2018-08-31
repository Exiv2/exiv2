/*!
  @file    timegm.h
  @brief   Declaration of timegm(). The implementation is in localtime.c
*/
#pragma once

#include <time.h>

#ifdef  __cplusplus
extern "C" {
#endif

// The UTC version of mktime
/* rmills - timegm is replaced with _mkgmtime on VC 2005 and up */
/*        - see localtime.c                                     */
#if !defined(_MSC_VER) || (_MSC_VER < 1400)
time_t timegm(struct tm * const tmp);
#else
#define timegm _mkgmtime
#endif

#ifdef  __cplusplus
}
#endif
