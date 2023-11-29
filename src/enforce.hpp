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

#include <string>

#include "error.hpp"

namespace Exiv2::Internal {
/*!
 * @brief Ensure that condition is true, otherwise throw an exception of the
 * type exception_t
 *
 * @tparam exception_t  Exception type that is thrown, must provide a
 * constructor that accepts a single argument to which args are forwarded.
 */
template <typename exception_t, typename... T>
constexpr void enforce(bool condition, T&&... args) {
  if (!condition) {
    throw exception_t(std::forward<T>(args)...);
  }
}

/*!
 * @brief Ensure that condition is true, otherwise throw an Exiv2::Error with
 * the given error_code & arguments.
 */
template <typename... T>
constexpr void enforce(bool condition, Exiv2::ErrorCode err_code, T&&... args) {
  enforce<Exiv2::Error>(condition, err_code, std::forward<T>(args)...);
}
}  // namespace Exiv2::Internal
