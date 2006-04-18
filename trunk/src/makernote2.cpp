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

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    bool TiffMnRegistry::operator==(const TiffMnRegistry::Key& key) const
    {
        std::string make(make_);
        return make == key.make_.substr(0, make.length());
    }

    bool TiffIfdMakernote::readHeader(const byte* pData, 
                                      uint32_t    size,
                                      ByteOrder   byteOrder)
    {
        return doReadHeader(pData, size, byteOrder);
    }

    bool TiffIfdMakernote::checkHeader() const
    {
        return doCheckHeader();
    }

    uint32_t TiffIfdMakernote::ifdOffset() const
    {
        return doIfdOffset();
    }

    TiffRwState::AutoPtr TiffIfdMakernote::getState(uint32_t mnOffset) const
    {
        return doGetState(mnOffset);
    }

    TiffRwState::AutoPtr TiffIfdMakernote::doGetState(uint32_t mnOffset) const
    {
        return TiffRwState::AutoPtr(0);
    }

    void TiffIfdMakernote::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        ifd_.addChild(tiffComponent);
    }

    void TiffIfdMakernote::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        ifd_.addNext(tiffComponent);
    }

    void TiffIfdMakernote::doAccept(TiffVisitor& visitor)
    {
        visitor.visitIfdMakernote(this);
        ifd_.accept(visitor);
        visitor.visitIfdMakernoteEnd(this);
    }

}                                       // namespace Exiv2
