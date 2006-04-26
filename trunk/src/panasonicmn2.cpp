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
  File:      panasonicmn2.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Apr-06, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// Define DEBUG to output debug information to std::cerr, e.g, by calling make
// like this: make DEFS=-DDEBUG makernote2.o
//#define DEBUG

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "panasonicmn2.hpp"
#include "tiffcomposite.hpp"
#include "types.hpp"

// + standard includes

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const byte PanasonicMnHeader::signature_[] = {
        'P', 'a', 'n', 'a', 's', 'o', 'n', 'i', 'c', 0x00, 0x00, 0x00
    };
    const uint32_t PanasonicMnHeader::size_ = 12;

    PanasonicMnHeader::PanasonicMnHeader()
    {
        read(signature_, size_, invalidByteOrder);
    }

    bool PanasonicMnHeader::read(const byte* pData,
                              uint32_t    size, 
                              ByteOrder   /*byteOrder*/)
    {
        assert (pData != 0);

        if (size < size_) return false;
        if (0 != memcmp(pData, signature_, 9)) return false;
        buf_.alloc(size_);
        memcpy(buf_.pData_, pData, buf_.size_);
        start_ = size_;
        return true;

    } // PanasonicMnHeader::read

    bool TiffPanasonicMn::doReadHeader(const byte* pData,
                                    uint32_t size,
                                    ByteOrder byteOrder)
    {
        return header_.read(pData, size, byteOrder);
    }

    uint32_t TiffPanasonicMn::doIfdOffset() const
    {
        return header_.ifdOffset();
    }

    // *************************************************************************
    // free functions

    TiffComponent* newPanasonicMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    mnGroup,
                              const byte* /*pData*/,
                              uint32_t    /*size*/, 
                              ByteOrder   /*byteOrder*/)
    {
        return new TiffPanasonicMn(tag, group, mnGroup);
    }

}                                       // namespace Exiv2
