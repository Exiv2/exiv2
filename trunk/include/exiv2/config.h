#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

// Linux GCC 4.8 appears to be confused about strerror_r
#ifndef EXV_STRERROR_R_CHAR_P
#ifdef  __gnu_linux__
#define EXV_STRERROR_R_CHAR_P
#endif
#endif

// Don't know why MinGW refuses to link libregex
#if defined(__MINGW32__) || defined(__MINGW64__)
#ifdef EXV_HAVE_REGEX
#undef EXV_HAVE_REGEX
#endif
#endif

