// ********************************************************* -*- C++ -*-
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
  @file    utils.hpp
  @brief   A collection of utility functions
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    12-Dec-03, ahu: created
 */
#ifndef UTILS_HPP_
#define UTILS_HPP_

// *********************************************************************
// included header files

// + standard includes
#include <string>

// *********************************************************************
// namespace extensions
/*!
  @brief Contains utility classes and functions. Most of these are
         wrappers for common C functions that do not require pointers
         and memory considerations.
*/
namespace Util {

// *********************************************************************
// free functions

    /*!
      @brief Get the directory component from the \em path string.
             See %dirname(3).

      This function can handle Windows paths to some extent: c:\\bar should
      be fine, \\\\bigsrv\\foo also, but \\\\bigsrv alone doesn't work.
     */
    std::string dirname(const std::string& path);

    /*!
      @brief Get the filename component from the \em path string.
             See %basename(3). If the \em delsuffix parameter is true,
             the suffix will be removed.

      This function can handle Windows paths to some extent: c:\\bar should
      be fine, \\\\bigsrv\\foo also, but \\\\bigsrv alone doesn't work.
     */
    std::string basename(const std::string& path, bool delsuffix =false);

    /*!
      @brief Get the suffix from the path string. Normally, the suffix
             is the substring of the basename of path from the last '.'
             to the end of the string.
     */
    std::string suffix(const std::string& path);

    /*!
      @brief Convert a C string to a long value, which is returned in n.
             Returns true if the conversion is successful, else false.
             n is not modified if the conversion is unsuccessful. See strtol(2).
     */
    bool strtol(const char* nptr, long& n);

    /*!
      @brief Replaces all occurrences of \em searchText in the \em text string
             by \em replaceText.
     */
    void replace(std::string& text, const std::string& searchText, const std::string& replaceText);

}                                       // namespace Util

#endif                                  // #ifndef UTILS_HPP_
