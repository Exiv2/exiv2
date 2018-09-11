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


namespace Exiv2
{

    /*!
      @brief Class of utility functions used by the video code.
     */
    class UtilsVideo
    {
    public:
        /*!
         @brief compare a buffer and string
         @param buf - binary buffer
         @param str - nul terminated C string
         @return true if match
         */
        static bool compareTagValue(Exiv2::DataBuf &buf, const char *str);

        /*!
         @brief compare a buffer and an array of strings
         @param buf - binary buffer
         @param arr - array of C strings C
         @param arraysize - length of arr
         @return true if match
         */
        static bool compareTagValue(Exiv2::DataBuf& buf,const char arr[][5],int32_t arraysize);

        /*!
         @brief compare a buffer and string
         @param buf - binary buffer
         @param str - C string (not guaranteed nul terminated)
         @param size - number of bytes to compare
         @return true if match
         */
        static bool simpleBytesComparison(Exiv2::DataBuf& buf ,const char* str,int32_t size);
    }; // class UtilsVideo

} // namespace Exiv2
