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
  File:      nikonmn2.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Apr-06, ahu: created
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

#include "nikonmn2.hpp"
#include "tiffcomposite.hpp"
#include "types.hpp"

// + standard includes

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const byte Nikon2MnHeader::signature_[] = {
        'N', 'i', 'k', 'o', 'n', '\0', 0x00, 0x01
    };
    const uint32_t Nikon2MnHeader::size_ = 8;

    Nikon2MnHeader::Nikon2MnHeader()
    {
        read(signature_, size_, invalidByteOrder);
    }

    bool Nikon2MnHeader::read(const byte* pData,
                              uint32_t    size, 
                              ByteOrder   /*byteOrder*/)
    {
        assert (pData != 0);

        if (size < size_) return false;
        if (0 != memcmp(pData, signature_, 6)) return false;
        buf_.alloc(size_);
        memcpy(buf_.pData_, pData, buf_.size_);
        start_ = size_;
        return true;

    } // Nikon2MnHeader::read

    bool TiffNikon2Mn::doReadHeader(const byte* pData,
                                    uint32_t size,
                                    ByteOrder byteOrder)
    {
        return header_.read(pData, size, byteOrder);
    }

    uint32_t TiffNikon2Mn::doIfdOffset() const
    {
        return header_.ifdOffset();
    }

    const byte Nikon3MnHeader::signature_[] = {
        'N', 'i', 'k', 'o', 'n', '\0',
        0x02, 0x10, 0x00, 0x00, 0x4d, 0x4d, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08
    };
    const uint32_t Nikon3MnHeader::size_ = 18;

    Nikon3MnHeader::Nikon3MnHeader()
    {
        read(signature_, size_, invalidByteOrder);
    }

    bool Nikon3MnHeader::read(const byte* pData,
                              uint32_t    size, 
                              ByteOrder   /*byteOrder*/)
    {
        assert (pData != 0);

        if (size < size_) return false;
        if (0 != memcmp(pData, signature_, 6)) return false;
        buf_.alloc(size_);
        memcpy(buf_.pData_, pData, buf_.size_);
        TiffHeade2 th;
        if (!th.read(buf_.pData_ + 10, 8)) return false;
        byteOrder_ = th.byteOrder();
        start_ = 10 + th.ifdOffset();
        return true;

    } // Nikon3MnHeader::read

    bool TiffNikon3Mn::doReadHeader(const byte* pData,
                                    uint32_t size,
                                    ByteOrder byteOrder)
    {
        return header_.read(pData, size, byteOrder);
    }

    uint32_t TiffNikon3Mn::doIfdOffset() const
    {
        return header_.ifdOffset();
    }

    TiffRwState::AutoPtr TiffNikon3Mn::doGetState(uint32_t mnOffset,
                                                  ByteOrder byteOrder) const
    {
        // Byteorder: From header
        // Offsets  : From header
        // Creator  : No change
        return TiffRwState::AutoPtr(
            new TiffRwState(header_.byteOrder(),
                            mnOffset + header_.baseOffset(), 
                            0));
    }

    // *************************************************************************
    // free functions

    TiffComponent* newNikonMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    mnGroup,
                              const byte* pData,
                              uint32_t    size,
                              ByteOrder   /*byteOrder*/)
    {
        // If there is no "Nikon" string it must be Nikon1 format
        if (size < 6 ||    std::string(reinterpret_cast<const char*>(pData), 6)
                        != std::string("Nikon\0", 6)) {
            return new TiffNikon1Mn(tag, group, Group::nikon1mn);
        }
        // If the "Nikon" string is not followed by a TIFF header, we assume
        // Nikon2 format
        TiffHeade2 tiffHeader;
        if (   size < 18
            || !tiffHeader.read(pData + 10, size - 10)
            || tiffHeader.tag() != 0x002a) {
            return new TiffNikon2Mn(tag, group, Group::nikon2mn);
        }
        // Else we have a Nikon3 makernote
        return new TiffNikon3Mn(tag, group, Group::nikon3mn);
    }

}                                       // namespace Exiv2
