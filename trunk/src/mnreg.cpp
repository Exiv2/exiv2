// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
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
  File:      mnreg.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   15-Apr-06, ahu: created

 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#include "makernote2.hpp"
#include "olympusmn2.hpp"

// + standard includes

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const TiffMnRegistry TiffMnCreator::registry_[] = {
        { "OLYMPUS", newOlympusMn, Group::olympmn }
    };


    // The find template needs to be in the same compilation unit as the array
    TiffComponent* TiffMnCreator::create(uint16_t    tag,
                                         uint16_t    group,
                                         std::string make,
                                         const byte* pData,
                                         uint32_t    size,
                                         ByteOrder   byteOrder)
    {
        TiffComponent* tc = 0;
        const TiffMnRegistry* tmr = find(registry_, TiffMnRegistry::Key(make));
        if (tmr) tc = tmr->newMnFct_(tag, 
                                     group, 
                                     tmr->mnGroup_,
                                     pData,
                                     size,
                                     byteOrder);
        return tc;
    } // TiffMnCreator::create


}                                       // namespace Exiv2
