// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
 *
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
  @file    sonymn.hpp
  @brief   Basic Sony MakerNote implementation
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    18-Apr-05, ahu: created
 */
#ifndef SONYMN_HPP_
#define SONYMN_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "types.hpp"

// + standard includes
#include <string>
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    //! MakerNote for Sony cameras
    class EXIV2API SonyMakerNote {
    public:
        //! Return read-only list of built-in Sony tags
        static const TagInfo* tagList();

    private:
        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class SonyMakerNote

}                                       // namespace Exiv2

#endif                                  // #ifndef SONYMN_HPP_
