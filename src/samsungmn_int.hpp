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
#ifndef SAMSUNGMN_INT_HPP_
#define SAMSUNGMN_INT_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "types.hpp"

// + standard includes
#include <string>
#include <iosfwd>
#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

// *****************************************************************************
// class definitions

    //! MakerNote for Samsung cameras
    class Samsung2MakerNote {
    public:
        //! Return read-only list of built-in Samsung tags
        static const TagInfo* tagList();
        //! Return read-only list of built-in PictureWizard tags
        static const TagInfo* tagListPw();

    private:
        //! Tag information
        static const TagInfo tagInfo_[];
        //! PictureWizard tag information
        static const TagInfo tagInfoPw_[];

    }; // class Samsung2MakerNote

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef SAMSUNGMN_INT_HPP_
