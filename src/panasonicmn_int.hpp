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
#ifndef PANASONICMN_INT_HPP_
#define PANASONICMN_INT_HPP_

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
    namespace Internal {

// *****************************************************************************
// class definitions

    //! MakerNote for Panasonic cameras
    class PanasonicMakerNote {
    public:
        //! Return read-only list of built-in Panasonic tags
        static const TagInfo* tagList();
        //! Return read-only list of built-in Panasonic RAW image tags (IFD0)
        static const TagInfo* tagListRaw();

        //! @name Print functions for Panasonic %MakerNote tags
        //@{
        //! Print SpotMode
        static std::ostream& print0x000f(std::ostream& os, const Value& value, const ExifData*);
        //! Print WhiteBalanceBias
        static std::ostream& print0x0023(std::ostream& os, const Value& value, const ExifData*);
        //! Print TimeSincePowerOn
        static std::ostream& print0x0029(std::ostream& os, const Value& value, const ExifData*);
        //! Print Baby age
        static std::ostream& print0x0033(std::ostream& os, const Value& value, const ExifData*);
        //! Print Travel days
        static std::ostream& print0x0036(std::ostream& os, const Value& value, const ExifData*);
        //! Print ISO
        static std::ostream& print0x003c(std::ostream& os, const Value& value, const ExifData*);
        //! Print Manometer Pressure
        static std::ostream& printPressure(std::ostream& os, const Value& value, const ExifData*);
        //! Print special text values: title, landmark, county and so on
        static std::ostream& printPanasonicText(std::ostream& os, const Value& value, const ExifData*);
        //! Print accerometer readings
        static std::ostream& printAccelerometer(std::ostream& os, const Value& value, const ExifData*);
        //! Print roll angle
        static std::ostream& printRollAngle(std::ostream& os, const Value& value, const ExifData*);
        //! Print pitch angle
        static std::ostream& printPitchAngle(std::ostream& os, const Value& value, const ExifData*);
        //@}

    private:
        //! Makernote tag list
        static const TagInfo tagInfo_[];
        //! Taglist for IFD0 of Panasonic RAW images
        static const TagInfo tagInfoRaw_[];

    }; // class PanasonicMakerNote

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef PANASONICMN_INT_HPP_
