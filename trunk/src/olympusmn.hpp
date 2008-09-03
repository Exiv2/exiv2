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
  @file    olympusmn.hpp
  @brief   Olympus MakerNote implemented using the following references:
           <a href="http://park2.wakwak.com/%7Etsuruzoh/Computer/Digicams/exif-e.html#APP1">Exif file format, Appendix 1: MakerNote of Olympus Digicams</a> by TsuruZoh Tachibanaya,<br>
           Olympus.pm of <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/">ExifTool</a> by Phil Harvey,<br>
           <a href="http://www.ozhiker.com/electronics/pjmt/jpeg_info/olympus_mn.html">Olympus Makernote Format Specification</a> by Evan Hunter,<br>
           email communication with <a href="mailto:wstokes@gmail.com">Will Stokes</a>
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Will Stokes (wuz)
           <a href="mailto:wstokes@gmail.com">wstokes@gmail.com</a>
  @author  Gilles Caulier (gc)
           <a href="mailto:caulier dot gilles at kdemail dot net">caulier dot gilles at kdemail dot net</a>
  @date    10-Mar-05, wuz: created
 */
#ifndef OLYMPUSMN_HPP_
#define OLYMPUSMN_HPP_

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

// *****************************************************************************
// class definitions

    //! MakerNote for Olympus cameras
    class EXIV2API OlympusMakerNote {
    public:
        //! Return read-only list of built-in Olympus tags
        static const TagInfo* tagList();

        //! @name Print functions for Olympus %MakerNote tags
        //@{
        //! Print 'Special Mode'
        static std::ostream& print0x0200(std::ostream& os, const Value& value, const ExifData*);
        //! Print Digital Zoom Factor
        static std::ostream& print0x0204(std::ostream& os, const Value& value, const ExifData*);
        //! Print White Balance Mode
        static std::ostream& print0x1015(std::ostream& os, const Value& value, const ExifData*);
        //@}

    private:
        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class OlympusMakerNote

}                                       // namespace Exiv2

#endif                                  // #ifndef OLYMPUSMN_HPP_
