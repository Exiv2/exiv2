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

/*!
  @brief This raises the default stack size limit so that deeply nested
         files won't cause a crash. On Linux, the default stack size is
         often only 8192KB, which is very easy to hit.
 */
void increase_stack_limit();
}  // namespace Util

#endif  // APP_UTILS_HPP_
