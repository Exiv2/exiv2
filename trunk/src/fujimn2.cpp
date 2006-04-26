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
  File:      fujimn2.cpp
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

#include "fujimn2.hpp"
#include "tiffcomposite.hpp"
#include "tiffparser.hpp"
#include "types.hpp"

// + standard includes
#include <cstring>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const byte FujiMnHeader::signature_[] = {
        'F', 'U', 'J', 'I', 'F', 'I', 'L', 'M', 0x0c, 0x00, 0x00, 0x00
    };
    const uint32_t FujiMnHeader::size_ = 12;
    const ByteOrder FujiMnHeader::byteOrder_ = littleEndian;

    FujiMnHeader::FujiMnHeader()
    {
        read(signature_, size_, byteOrder_);
    }

    bool FujiMnHeader::read(const byte* pData,
                            uint32_t size, 
                            ByteOrder /*byteOrder*/)
    {
        assert (pData != 0);

        if (size < size_) return false;

        header_.alloc(size_);
        memcpy(header_.pData_, pData, header_.size_);

        // Read offset to the IFD relative to the start of the makernote
        // from the header. Note that we ignore the byteOrder argument
        start_ = getUShort(header_.pData_ + 8, byteOrder_);

        if (   static_cast<uint32_t>(header_.size_) < size_ 
            || 0 != memcmp(header_.pData_, signature_, 8)) {
            return false;
        }
        return true;
    } // FujiMnHeader::read

    bool TiffFujiMn::doReadHeader(const byte* pData,
                                  uint32_t size,
                                  ByteOrder byteOrder)
    {
        return header_.read(pData, size, byteOrder);
    }
    
    uint32_t TiffFujiMn::doIfdOffset() const
    {
        return header_.ifdOffset();
    }

    TiffRwState::AutoPtr TiffFujiMn::doGetState(uint32_t mnOffset,
                                                ByteOrder /*byteOrder*/) const
    {
        // Byteorder: from the header (little endian) 
        // Offsets  : relative to the start of the makernote
        // Creator  : no change
        return TiffRwState::AutoPtr(
            new TiffRwState(header_.byteOrder(), mnOffset, 0));
    }

    // *************************************************************************
    // free functions

    TiffComponent* newFujiMn(uint16_t    tag,
                             uint16_t    group,
                             uint16_t    mnGroup,
                             const byte* /*pData*/,
                             uint32_t    /*size*/, 
                             ByteOrder   /*byteOrder*/)
    {
        return new TiffFujiMn(tag, group, mnGroup);
    }

}                                       // namespace Exiv2
