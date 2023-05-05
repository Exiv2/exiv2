// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IMAGE_INT_HPP_
#define IMAGE_INT_HPP_

// *****************************************************************************
// included header files
#include "slice.hpp"  // for Slice

#include <cstddef>  // for size_t
#include <cstdint>  // for int32_t
#include <ostream>  // for ostream, basic_ostream::put
#include <string>

#if defined(__MINGW32__)
#define ATTRIBUTE_FORMAT_PRINTF __attribute__((format(__MINGW_PRINTF_FORMAT, 1, 2)))
#elif defined(__GNUC__)
#define ATTRIBUTE_FORMAT_PRINTF __attribute__((format(printf, 1, 2)))
#else
#define ATTRIBUTE_FORMAT_PRINTF
#endif

// *****************************************************************************
// namespace extensions
namespace Exiv2::Internal {
// *****************************************************************************
// class definitions

/*!
  @brief format a string in the pattern of \em sprintf \em .
 */
std::string stringFormat(const char* format, ...) ATTRIBUTE_FORMAT_PRINTF;

/*!
 * @brief Helper struct for binary data output via @ref binaryToString.
 *
 * The only purpose of this struct is to provide a custom
 * `operator<<(std::ostream&)` for the output of binary data, that is not
 * used for all Slices by default.
 */
template <typename T>
struct binaryToStringHelper;

/*!
 * @brief Actual implementation of the output algorithm described in @ref
 * binaryToString
 *
 * @throws nothing
 */
template <typename T>
std::ostream& operator<<(std::ostream& stream, const binaryToStringHelper<T>& binToStr) {
  for (size_t i = 0; i < binToStr.buf_.size(); ++i) {
    auto c = static_cast<int>(binToStr.buf_.at(i));
    const bool bTrailingNull = c == 0 && i == binToStr.buf_.size() - 1;
    if (!bTrailingNull) {
      if (c < ' ' || c >= 127) {
        c = '.';
      }
      stream.put(static_cast<char>(c));
    }
  }
  return stream;
}

template <typename T>
struct binaryToStringHelper {
  constexpr binaryToStringHelper(Slice<T>&& myBuf) noexcept : buf_(std::move(myBuf)) {
  }

  // the Slice is stored by value to avoid dangling references, in case we
  // invoke:
  // binaryToString(makeSlice(buf, 0, n));
  // <- buf_ would be now dangling, were it a reference
  Slice<T> buf_;
};

/*!
 * @brief format binary data for display in @ref Image::printStructure()
 *
 * This function creates a new helper class that can be passed to a
 * `std::ostream` for output. It creates a printable version of the binary
 * data in the slice sl according to the following rules:
 * - characters with numeric values larger than 0x20 (= space) and smaller
 *   or equal to 0x7F (Delete) are printed as ordinary characters
 * - characters outside of that range are printed as '.'
 * - if the last element of the slice is 0, then it is omitted
 *
 * @param[in] sl  Slice containing binary data buffer that should be
 *     printed.
 *
 * @return Helper object, that can be passed into a std::ostream and
 *     produces an output according to the aforementioned rules.
 *
 * @throw This function does not throw. The output of the helper object to
 *     the stream throws neither.
 */
template <typename T>
constexpr binaryToStringHelper<T> binaryToString(Slice<T>&& sl) noexcept {
  return binaryToStringHelper<T>(std::move(sl));
}

/// @brief indent output for kpsRecursive in \em printStructure() \em .
std::string indent(size_t i);

}  // namespace Exiv2::Internal

#endif  // #ifndef IMAGE_INT_HPP_
