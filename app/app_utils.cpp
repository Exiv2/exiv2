// SPDX-License-Identifier: GPL-2.0-or-later

#include "app_utils.hpp"
#include <climits>
#include <cstdlib>
#include <limits>

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

}  // namespace Util
