// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*!
  @file    image_int.hpp
  @brief   Internal image helpers
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    10-May-15, ahu: created
 */
#ifndef IMAGE_INT_HPP_
#define IMAGE_INT_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <string>

#if (defined(__GNUG__) || defined(__GNUC__)) || defined(__clang__)
#define ATTRIBUTE_FORMAT_PRINTF __attribute__((format(printf, 1, 0)))
#else
#define ATTRIBUTE_FORMAT_PRINTF
#endif

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

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
    std::ostream& operator<<(std::ostream& stream, const binaryToStringHelper<T>& binToStr) throw()
    {
        for (size_t i = 0; i < binToStr.buf_.size(); ++i) {
            int c = static_cast<int>(binToStr.buf_.at(i));
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
    struct binaryToStringHelper
    {
        explicit binaryToStringHelper(const Slice<T> buf) throw() : buf_(buf)
        {
        }

        friend std::ostream& operator<<<T>(std::ostream& stream, const binaryToStringHelper<T>& binToStr) throw();

        // the Slice is stored by value to avoid dangling references, in case we
        // invoke:
        // binaryToString(makeSlice(buf, 0, n));
        // <- buf_ would be now dangling, were it a reference
        const Slice<T> buf_;
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
    inline binaryToStringHelper<T> binaryToString(const Slice<T> sl) throw()
    {
        return binaryToStringHelper<T>(sl);
    }

    /*!
      @brief format binary for display of raw data .
     */
    std::string binaryToHex(const byte *data, size_t size);

    /*!
      @brief indent output for kpsRecursive in \em printStructure() \em .
     */
    std::string indent(int32_t depth);

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef IMAGE_INT_HPP_
