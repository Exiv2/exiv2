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
  @file    nikonmn.hpp
  @brief   Nikon makernote tags.<BR>References:<BR>
  [1] <a href="http://www.tawbaware.com/990exif.htm">MakerNote EXIF Tag of the Nikon 990</a> by Max Lyons<BR>
  [2] <a href="http://park2.wakwak.com/%7Etsuruzoh/Computer/Digicams/exif-e.html">Exif file format</a> by TsuruZoh Tachibanaya<BR>
  [3] "EXIFutils Field Reference Guide"<BR>
  [3] <a href="http://www.ozhiker.com/electronics/pjmt/jpeg_info/nikon_mn.html#Nikon_Type_3_Tags">Nikon Type 3 Makernote Tags Definition</a> of the PHP JPEG Metadata Toolkit by Evan Hunter<BR>
  [4] <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/">ExifTool</a> by Phil Harvey<BR>
  [5] Email communication with <a href="http://www.rottmerhusen.com">Robert Rottmerhusen</a><BR>
  [6] Email communication with Roger Larsson<BR>
  [7] <a href="http://www.cybercom.net/~dcoffin/dcraw/">Decoding raw digital photos in Linux</a> by Dave Coffin<br>

  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Gilles Caulier (gc)
           <a href="mailto:caulier dot gilles at kdemail dot net">caulier dot gilles at kdemail dot net</a>
  @date    17-May-04, ahu: created<BR>
           25-May-04, ahu: combined all Nikon formats in one component
 */
#ifndef NIKONMN_HPP_
#define NIKONMN_HPP_

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

    //! A MakerNote format used by Nikon cameras, such as the E990 and D1.
    class EXIV2API Nikon1MakerNote {
    public:
        //! Return read-only list of built-in Nikon1 tags
        static const TagInfo* tagList();

        //! @name Print functions for Nikon1 %MakerNote tags
        //@{
        //! Print ISO setting
        static std::ostream& print0x0002(std::ostream& os, const Value& value, const ExifData*);
        //! Print autofocus mode
        static std::ostream& print0x0007(std::ostream& os, const Value& value, const ExifData*);
        //! Print manual focus distance
        static std::ostream& print0x0085(std::ostream& os, const Value& value, const ExifData*);
        //! Print digital zoom setting
        static std::ostream& print0x0086(std::ostream& os, const Value& value, const ExifData*);
        //! Print AF focus position
        static std::ostream& print0x0088(std::ostream& os, const Value& value, const ExifData*);
        //@}

    private:
        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class Nikon1MakerNote

    /*!
      @brief A second MakerNote format used by Nikon cameras, including the
             E700, E800, E900, E900S, E910, E950
     */
    class EXIV2API Nikon2MakerNote {
    public:
        //! Return read-only list of built-in Nikon2 tags
        static const TagInfo* tagList();

        //! @name Print functions for Nikon2 %MakerNote tags
        //@{
        //! Print digital zoom setting
        static std::ostream& print0x000a(std::ostream& os, const Value& value, const ExifData*);
        //@}

    private:
        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class Nikon2MakerNote

    //! A third MakerNote format used by Nikon cameras, e.g., E5400, SQ, D2H, D70
    class EXIV2API Nikon3MakerNote {
    public:
        //! Return read-only list of built-in Nikon3 tags
        static const TagInfo* tagList();

        //! @name Print functions for Nikon3 %MakerNote tags
        //@{
        //! Print ISO setting
        static std::ostream& print0x0002(std::ostream& os, const Value& value, const ExifData*);
        //! Print autofocus mode
        static std::ostream& print0x0007(std::ostream& os, const Value& value, const ExifData*);
        //! Print lens type
        static std::ostream& print0x0083(std::ostream& os, const Value& value, const ExifData*);
        //! Print lens information
        static std::ostream& print0x0084(std::ostream& os, const Value& value, const ExifData*);
        //! Print manual focus distance
        static std::ostream& print0x0085(std::ostream& os, const Value& value, const ExifData*);
        //! Print digital zoom setting
        static std::ostream& print0x0086(std::ostream& os, const Value& value, const ExifData*);
        //! Print AF point
        static std::ostream& print0x0088(std::ostream& os, const Value& value, const ExifData*);
        //! Print number of lens stops
        static std::ostream& print0x008b(std::ostream& os, const Value& value, const ExifData*);
        //! Print number of lens data
        static std::ostream& print0x0098(std::ostream& os, const Value& value, const ExifData*);
        //@}

    private:
        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class Nikon3MakerNote

}                                       // namespace Exiv2

#endif                                  // #ifndef NIKONMN_HPP_
