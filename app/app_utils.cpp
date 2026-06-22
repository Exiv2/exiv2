// SPDX-License-Identifier: GPL-2.0-or-later

#include "app_utils.hpp"
#include <climits>
#include <cstdlib>
#include <limits>
#include "error.hpp"
#include "futils.hpp"

#if __has_include(<sys/resource.h>)
#include <sys/resource.h>  // for getrlimit()
#define HAS_GETRLIMIT
#endif

namespace Util {
bool strtol(const char* nptr, int64_t& n) {
  if (!nptr || *nptr == '\0')
    return false;
  char* endptr = nullptr;
  long long tmp = std::strtoll(nptr, &endptr, 10);
  if (*endptr != '\0')
    return false;
  // strtoll returns LLONG_MAX or LLONG_MIN if an overflow occurs.
  if (tmp == LLONG_MAX || tmp == LLONG_MIN)
    return false;
  if (tmp < std::numeric_limits<int64_t>::min())
    return false;
  if (tmp > std::numeric_limits<int64_t>::max())
    return false;
  n = static_cast<int64_t>(tmp);
  return true;
}

void increase_stack_limit() {
  using namespace Exiv2;
#ifdef HAS_GETRLIMIT
  {
    struct rlimit rl {};

    if (getrlimit(RLIMIT_STACK, &rl) < 0) {
#ifndef SUPPRESS_WARNINGS
      EXV_WARNING << "getrlimit(RLIMIT_STACK) failed: " << strError() << "\n";
#endif
      return;
    }
    if (rl.rlim_cur < rl.rlim_max) {
      // Increase it to the max allowed.
      rl.rlim_cur = rl.rlim_max;
      if (setrlimit(RLIMIT_STACK, &rl) < 0) {
#ifndef SUPPRESS_WARNINGS
        EXV_WARNING << "setrlimit(RLIMIT_STACK) failed: " << strError() << "\n";
#endif
        return;
      }
    }
    return;
  }
#endif
}

}  // namespace Util
