// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*
  File:      makernote.cpp
  Version:   $Name:  $ $Revision: 1.5 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Feb-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.5 $ $RCSfile: makernote.cpp,v $")

// *****************************************************************************
// included header files
#include "makernote.hpp"
#include "tags.hpp"                         // for ExifTags::ifdItem
#include "error.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>

// Define DEBUG_MAKERNOTE to output debug information to std::cerr
#undef DEBUG_MAKERNOTE

// *****************************************************************************
// class member definitions
namespace Exif {

    std::string MakerNote::makeKey(uint16 tag) const
    {
        return std::string(ExifTags::ifdItem(makerIfd))
            + "." + sectionName(tag) + "." + tagName(tag);
    } // MakerNote::makeKey

    uint16 MakerNote::decomposeKey(const std::string& key) const
    {
        // Get the IFD, section name and tag name parts of the key
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string ifdItem = key.substr(0, pos1);
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string sectionName = key.substr(pos0, pos1 - pos0);
        pos0 = pos1 + 1;
        std::string tagName = key.substr(pos0);
        if (tagName == "") throw Error("Invalid key");

        if (ifdItem != ExifTags::ifdItem(makerIfd)) return 0xffff;
        uint16 tag = this->tag(tagName);
        if (tag == 0xffff) return tag;
        if (sectionName != this->sectionName(tag)) return 0xffff;

        return tag;
    } // MakerNote::decomposeKey

    std::string MakerNote::tagName(uint16 tag) const
    {
        std::string tagName;
        if (mnTagInfo_) {
            for (int i = 0; mnTagInfo_[i].tag_ != 0xffff; ++i) {
                if (mnTagInfo_[i].tag_ == tag) {
                    tagName = mnTagInfo_[i].name_;
                    break;
                }
            }
        }
        if (tagName.empty()) {
            std::ostringstream os;
            os << "0x" << std::setw(4) << std::setfill('0') << std::right
               << std::hex << tag;
            tagName = os.str();
        }
        return tagName;
    } // MakerNote::tagName

    uint16 MakerNote::tag(const std::string& tagName) const
    {
        uint16 tag = 0xffff;
        if (mnTagInfo_) {
            for (int i = 0; mnTagInfo_[i].tag_ != 0xffff; ++i) {
                if (mnTagInfo_[i].name_ == tagName) {
                    tag = mnTagInfo_[i].tag_;
                    break;
                }
            }
        }
        if (tag == 0xffff) {
            std::istringstream is(tagName);
            is >> std::hex >> tag;
        }
        return tag;
    } // MakerNote::tag

    int IfdMakerNote::read(const char* buf,
                           long len, 
                           ByteOrder byteOrder, 
                           long offset)
    {
        int rc = ifd_.read(buf, byteOrder, offset);
        if (rc == 0) {
            // Todo: Make sure the Next field is 0.
            Entries::iterator end = ifd_.end();
            for (Entries::iterator i = ifd_.begin(); i != end; ++i) {
                i->setMakerNote(this);
            }
        }
#ifdef DEBUG_MAKERNOTE
        hexdump(std::cerr, buf, len);
        if (rc == 0) ifd_.print(std::cerr);
#endif
        return rc;
    }

    long IfdMakerNote::copy(char* buf, ByteOrder byteOrder, long offset)
    {
        return ifd_.copy(buf, byteOrder, offset);
    }

    long IfdMakerNote::size() const
    {
         return ifd_.size() + ifd_.dataSize();
    }

}                                       // namespace Exif
