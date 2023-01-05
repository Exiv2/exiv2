// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef APP_UTILS_HPP_
#define APP_UTILS_HPP_

#include <cstdint>

namespace Util {
/*!
  @brief Convert a C string to an int64_t value, which is returned in n.
         Returns true if the conversion is successful, else false.
         n is not modified if the conversion is unsuccessful. See strtol(2).
 */
bool strtol(const char* nptr, int64_t& n);
}  // namespace Util

#endif  // APP_UTILS_HPP_
