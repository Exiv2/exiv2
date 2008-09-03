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
  @file    fujimn.hpp
  @brief   Fujifilm MakerNote implemented according to the specification
           in Appendix 4: Makernote of Fujifilm of the document
           <a href="http://park2.wakwak.com/%7Etsuruzoh/Computer/Digicams/exif-e.html">
           Exif file format</a> by TsuruZoh Tachibanaya<br>
           <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/Fuji.html">Fuji Makernote list</a> by Phil Harvey<br>
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Gilles Caulier (gc)
           <a href="mailto:caulier dot gilles at kdemail dot net">caulier dot gilles at kdemail dot net</a>
  @date    11-Feb-04, ahu: created
 */
#ifndef FUJIMN_HPP_
#define FUJIMN_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "types.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    //! MakerNote for Fujifilm cameras
    class EXIV2API FujiMakerNote {
    public:
        //! Return read-only list of built-in Fujifilm tags
        static const TagInfo* tagList();

    private:
        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class FujiMakerNote

}                                       // namespace Exiv2

#endif                                  // #ifndef FUJIMN_HPP_
