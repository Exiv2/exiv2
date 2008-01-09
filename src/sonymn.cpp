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
/*
  File:      sonymn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Apr-05, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "sonymn.hpp"
#include "makernote.hpp"
#include "value.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! @cond IGNORE
    SonyMakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("SONY", "*", createSonyMakerNote);
        MakerNoteFactory::registerMakerNote(
            sonyIfdId, MakerNote::AutoPtr(new SonyMakerNote));

        ExifTags::registerMakerTagInfo(sonyIfdId, tagInfo_);
    }
    //! @endcond

    // Sony MakerNote Tag Info
    const TagInfo SonyMakerNote::tagInfo_[] = {
        TagInfo(0x2000, "0x2000", "0x2000",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9001, "0x9001", "0x9001",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9002, "0x9002", "0x9002",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9003, "0x9003", "0x9003",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9004, "0x9004", "0x9004",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9005, "0x9005", "0x9005",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9006, "0x9006", "0x9006",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9007, "0x9007", "0x9007",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9008, "0x9008", "0x9008",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownSonyMakerNoteTag)", "(UnknownSonyMakerNoteTag)",
                N_("Unknown SonyMakerNote tag"),
                sonyIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* SonyMakerNote::tagList()
    {
        return tagInfo_;
    }

    SonyMakerNote::SonyMakerNote(bool alloc)
        : IfdMakerNote(sonyIfdId, alloc, false)
    {
        byte buf[] = {
            'S', 'O', 'N', 'Y', ' ', 'D', 'S', 'C', ' ', '\0', '\0', '\0'
        };
        readHeader(buf, 12, byteOrder_);
    }

    SonyMakerNote::SonyMakerNote(const SonyMakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    int SonyMakerNote::readHeader(const byte* buf,
                                  long        len,
                                  ByteOrder   /*byteOrder*/)
    {
        if (len < 12) return 1;
        header_.alloc(12);
        std::memcpy(header_.pData_, buf, header_.size_);
        // Adjust the offset of the IFD for the prefix
        start_ = 12;
        return 0;
    }

    int SonyMakerNote::checkHeader() const
    {
        int rc = 0;
        // Check the SONY prefix
        if (   header_.size_ < 12
            || std::string(reinterpret_cast<char*>(header_.pData_), 12)
               != std::string("SONY DSC \0\0\0", 12)) {
            rc = 2;
        }
        return rc;
    }

    SonyMakerNote::AutoPtr SonyMakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    SonyMakerNote* SonyMakerNote::create_(bool alloc) const
    {
        AutoPtr makerNote(new SonyMakerNote(alloc));
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    SonyMakerNote::AutoPtr SonyMakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    SonyMakerNote* SonyMakerNote::clone_() const
    {
        return new SonyMakerNote(*this);
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createSonyMakerNote(bool        alloc,
                                           const byte* /*buf*/,
                                           long        /*len*/,
                                           ByteOrder   /*byteOrder*/,
                                           long        /*offset*/)
    {
        return MakerNote::AutoPtr(new SonyMakerNote(alloc));
    }

}                                       // namespace Exiv2
