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
  File:      makernote2.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   11-Apr-06, ahu: created
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

#include "makernote2.hpp"
#include "tiffcomposite.hpp"
#include "tiffvisitor.hpp"

// + standard includes
#include <string>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const TiffMnRegistry TiffMnCreator::registry_[] = {
        { "OLYMPUS", newOlympusMn }
    };

    bool TiffMnRegistry::operator==(const TiffMnRegistry::Key& key) const
    {
        std::string make(make_);
        return make == key.make_.substr(0, make.length());
    }

    TiffComponent* TiffMnCreator::create(uint16_t    tag,
                                         uint16_t    group,
                                         std::string make,
                                         const byte* pData,
                                         uint32_t    size,
                                         ByteOrder   byteOrder)
    {
        TiffComponent* tc = 0;
        const TiffMnRegistry* tmr = find(registry_, TiffMnRegistry::Key(make));
        if (tmr) tc = tmr->newMnFct_(tag, group, pData, size, byteOrder);
        return tc;
    } // TiffMnCreator::create

    const char* OlympusMnHeader::signature_ = "OLYMP\0\1\0";
    const uint32_t OlympusMnHeader::size_ = 8;

    OlympusMnHeader::OlympusMnHeader()
    {
        read(reinterpret_cast<const byte*>(signature_), size_);
    }

    bool OlympusMnHeader::read(const byte* pData, uint32_t size)
    {
        assert (pData != 0);

        if (size < size_) return false;

        header_.alloc(size_);
        memcpy(header_.pData_, pData, header_.size_);
        return true;
    } // OlympusMnHeader::read

    bool OlympusMnHeader::check() const
    {
        if (   static_cast<uint32_t>(header_.size_) < size_ 
            || 0 != memcmp(header_.pData_, signature_, 5)) {
            return false;
        }
        return true;
    } // OlympusMnHeader::check

    void TiffOlympusMn::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        ifd_.addChild(tiffComponent);
    }

    void TiffOlympusMn::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        ifd_.addNext(tiffComponent);
    }

    void TiffOlympusMn::doAccept(TiffVisitor& visitor)
    {
        visitor.visitOlympusMn(this);
        ifd_.accept(visitor);
    }

    // *************************************************************************
    // free functions

    TiffComponent* newOlympusMn(uint16_t    tag,
                                uint16_t    group,
                                const byte* /*pData*/,
                                uint32_t    /*size*/, 
                                ByteOrder   /*byteOrder*/)
    {
        return new TiffOlympusMn(tag, group);
    }

}                                       // namespace Exiv2
