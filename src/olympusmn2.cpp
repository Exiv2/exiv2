// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006 Andreas Huggel <ahuggel@gmx.net>
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
  File:      olympusmn2.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   15-Apr-06, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "olympusmn2.hpp"
#include "tiffcomposite.hpp"
#include "types.hpp"

// + standard includes
#include <cstring>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const byte OlympusMnHeader::signature_[] = {
        'O', 'L', 'Y', 'M', 'P', 0x00, 0x01, 0x00
    };
    const uint32_t OlympusMnHeader::size_ = 8;

    OlympusMnHeader::OlympusMnHeader()
    {
        read(signature_, size_, invalidByteOrder);
    }

    bool OlympusMnHeader::read(const byte* pData, 
                               uint32_t size, 
                               ByteOrder /*byteOrder*/)
    {
        assert (pData != 0);

        if (size < size_) return false;

        header_.alloc(size_);
        memcpy(header_.pData_, pData, header_.size_);
        if (   static_cast<uint32_t>(header_.size_) < size_ 
            || 0 != memcmp(header_.pData_, signature_, 5)) {
            return false;
        }
        return true;
    } // OlympusMnHeader::read

    bool TiffOlympusMn::doReadHeader(const byte* pData, 
                                     uint32_t    size, 
                                     ByteOrder   byteOrder)
    {
        return header_.read(pData, size, byteOrder);
    }
    
    uint32_t TiffOlympusMn::doIfdOffset() const
    {
        return header_.ifdOffset();
    }

    // *************************************************************************
    // free functions

    TiffComponent* newOlympusMn(uint16_t    tag,
                                uint16_t    group,
                                uint16_t    mnGroup,
                                const byte* /*pData*/,
                                uint32_t    /*size*/, 
                                ByteOrder   /*byteOrder*/)
    {
        return new TiffOlympusMn(tag, group, mnGroup);
    }

}                                       // namespace Exiv2
