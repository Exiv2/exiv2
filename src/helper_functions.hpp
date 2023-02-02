// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef HELPER_FUNCTIONS_HPP
#define HELPER_FUNCTIONS_HPP

#include <string>
#include "basicio.hpp"
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

namespace Exiv2 {
/*!
  @brief Function used to convert a decimal number to its Hexadecimal
      equivalent, then parsed into a character
  @param n Integer which is to be parsed as Hexadecimal character
  @return Return a Hexadecimal number, in character
 */
char returnHex(int n);

/*!
  @brief Function used to read data from data buffer, reads 16-bit character
      array and stores it in std::string object.
  @param buf Exiv2 data buffer, which stores the information
  @return Returns std::string object .
 */

static constexpr size_t BYTE = 0x1;
static constexpr size_t WCHAR = 0x2;
static constexpr size_t WORD = 0X2;
static constexpr size_t DWORD = 0x4;
static constexpr size_t QWORD = 0x8;
static constexpr size_t GUID = 0x10;

std::string toString16(Exiv2::DataBuf& buf);

[[nodiscard]] uint64_t readQWORDTag(Exiv2::BasicIo::UniquePtr& io);

[[nodiscard]] uint32_t readDWORDTag(Exiv2::BasicIo::UniquePtr& io);

[[nodiscard]] uint16_t readWORDTag(Exiv2::BasicIo::UniquePtr& io);

[[nodiscard]] std::string readStringWcharTag(Exiv2::BasicIo::UniquePtr& io, uint16_t length);

[[nodiscard]] std::string readStringTag(Exiv2::BasicIo::UniquePtr& io, uint16_t length = DWORD);

}  // namespace Exiv2
#endif  // HELPER_FUNCTIONS_HPP
