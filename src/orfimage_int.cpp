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

#include "orfimage_int.hpp"

namespace Exiv2 {
    namespace Internal {

    OrfHeader::OrfHeader(ByteOrder byteOrder)
        : TiffHeaderBase(0x4f52, 8, byteOrder, 0x00000008),
          sig_(0x4f52)
    {
    }

    bool OrfHeader::read(const byte* pData, uint32_t size)
    {
        if (size < 8) return false;

        if (pData[0] == 'I' && pData[0] == pData[1]) {
            setByteOrder(littleEndian);
        }
        else if (pData[0] == 'M' && pData[0] == pData[1]) {
            setByteOrder(bigEndian);
        }
        else {
            return false;
        }

        uint16_t sig = getUShort(pData + 2, byteOrder());
        if (tag() != sig && 0x5352 != sig) return false; // #658: Added 0x5352 "SR" for SP-560UZ
        sig_ = sig;
        setOffset(getULong(pData + 4, byteOrder()));
        return true;
    } // OrfHeader::read

    DataBuf OrfHeader::write() const
    {
        DataBuf buf(8);
        switch (byteOrder()) {
        case littleEndian:
            buf.write_uint8(0, 'I');
            break;
        case bigEndian:
            buf.write_uint8(0, 'M');
            break;
        case invalidByteOrder:
            assert(false);
            break;
        }
        buf.write_uint8(1, buf.read_uint8(0));

        buf.write_uint16(2, sig_, byteOrder());
        buf.write_uint32(4, 0x00000008, byteOrder());
        return buf;
    }

    }  // namespace Internal
}  // namespace Exiv2
