// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef HELPER_FUNCTIONS_HPP
#define HELPER_FUNCTIONS_HPP

#include <string>
#include "types.hpp"
/*!
  @brief Convert a (potentially not null terminated) array into a
  std::string.

  Convert a C style string that may or may not be null terminated safely
  into a std::string. The string's termination is either set at the first \0
  or after data_length characters.

  @param[in] data  A c-string from which the std::string shall be
      constructed. Does not need to be null terminated.
  @param[in] data_length  An upper bound for the string length (must be at most
      the allocated length of `buffer`). If no null terminator is found in data,
      then the resulting std::string will be null terminated at `data_length`.

 */
std::string string_from_unterminated(const char* data, size_t data_length);

namespace Util {
/*!
  @brief Function used to convert a decimal number to its Hexadecimal
      equivalent, then parsed into a character
  @param n Integer which is to be parsed as Hexadecimal character
  @return Return a Hexadecimal number, in character
 */
char returnHEX(int n);

/*!
  @brief Function used to read data from data buffer, reads 16-bit character
      array and stores it in std::string object.
  @param buf Exiv2 data buffer, which stores the information
  @return Returns std::string object .
 */
std::string toString16(Exiv2::DataBuf& buf);

//! Function used to convert buffer data into 64-bit Integer, information stored in littleEndian format
uint64_t getUint64_t(Exiv2::DataBuf& buf);
}  // namespace Util
#endif  // HELPER_FUNCTIONS_HPP
