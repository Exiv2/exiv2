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
  @file    enforce.hpp
  @brief   Port of D's enforce() to C++ & Exiv2
  @author  Dan Čermák (D4N)
           <a href="mailto:dan.cermak@cgc-instruments.com">dan.cermak@cgc-instruments.com</a>
  @date    11-March-18, D4N: created
 */

#include <string>

#include "error.hpp"

/*!
 * @brief Ensure that condition is true, otherwise throw an exception of the
 * type exception_t
 *
 * @tparam exception_t  Exception type that is thrown, must provide a
 * constructor that accepts a single argument to which arg1 is forwarded.
 *
 * @todo once we have C++>=11 use variadic templates and std::forward to remove
 * all overloads of enforce
 */
template <typename exception_t, typename T>
inline void enforce(bool condition, const T& arg1)
{
    if (!condition) {
        throw exception_t(arg1);
    }
}

/*!
 * @brief Ensure that condition is true, otherwise throw an Exiv2::Error with
 * the given error_code.
 */
inline void enforce(bool condition, Exiv2::ErrorCode err_code)
{
    if (!condition) {
        throw Exiv2::Error(err_code);
    }
}

/*!
 * @brief Ensure that condition is true, otherwise throw an Exiv2::Error with
 * the given error_code & arg1.
 */
template <typename T>
inline void enforce(bool condition, Exiv2::ErrorCode err_code, const T& arg1)
{
    if (!condition) {
        throw Exiv2::Error(err_code, arg1);
    }
}

/*!
 * @brief Ensure that condition is true, otherwise throw an Exiv2::Error with
 * the given error_code, arg1 & arg2.
 */
template <typename T, typename U>
inline void enforce(bool condition, Exiv2::ErrorCode err_code, const T& arg1, const U& arg2)
{
    if (!condition) {
        throw Exiv2::Error(err_code, arg1, arg2);
    }
}

/*!
 * @brief Ensure that condition is true, otherwise throw an Exiv2::Error with
 * the given error_code, arg1, arg2 & arg3.
 */
template <typename T, typename U, typename V>
inline void enforce(bool condition, Exiv2::ErrorCode err_code, const T& arg1, const U& arg2, const V& arg3)
{
    if (!condition) {
        throw Exiv2::Error(err_code, arg1, arg2, arg3);
    }
}
