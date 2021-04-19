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
#ifndef SIGMAMN_INT_HPP_
#define SIGMAMN_INT_HPP_

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

    //! MakerNote for Sigma (Foveon) cameras
    class SigmaMakerNote {
    public:
        //! Return read-only list of built-in Sigma tags
        static const TagInfo* tagList();

        //! @name Print functions for Sigma (Foveon) %MakerNote tags
        //@{
        //! Strip the label from the value and print the remainder
        static std::ostream& printStripLabel(std::ostream& os, const Value& value, const ExifData*);
        //! Print exposure mode
        static std::ostream& print0x0008(std::ostream& os, const Value& value, const ExifData*);
        //! Print metering mode
        static std::ostream& print0x0009(std::ostream& os, const Value& value, const ExifData*);
        //@}

    private:
        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class SigmaMakerNote

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef SIGMAMN_INT_HPP_
