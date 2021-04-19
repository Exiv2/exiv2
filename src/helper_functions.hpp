// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
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
#ifndef HELPER_FUNCTIONS_HPP
#define HELPER_FUNCTIONS_HPP

#include <string>

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

#endif  // HELPER_FUNCTIONS_HPP
