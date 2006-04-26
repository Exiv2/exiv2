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
  File:      canonmn2.cpp
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

#include "canonmn2.hpp"
#include "tiffcomposite.hpp"
#include "types.hpp"

// + standard includes

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const TiffStructure TiffCanonCreator::tiffStructure_[] = {
        // ext. tag           group  create function      new group
        //---------  --------------  -------------------  --------------
        {    0x0001, Group::canonmn, newTiffArrayEntry,   Group::canoncs },
        {    0x0004, Group::canonmn, newTiffArrayEntry,   Group::canonsi },
        {    0x000f, Group::canonmn, newTiffArrayEntry,   Group::canoncf },
        {  Tag::all, Group::canoncs, newTiffArrayElement, Group::canoncs },
        {  Tag::all, Group::canonsi, newTiffArrayElement, Group::canonsi },
        {  Tag::all, Group::canoncf, newTiffArrayElement, Group::canoncf }
    };

    TiffComponent::AutoPtr TiffCanonCreator::create(uint32_t extendedTag,
                                                    uint16_t group)
    {
        TiffComponent::AutoPtr tc(0);
        uint16_t tag = static_cast<uint16_t>(extendedTag & 0xffff);
        const TiffStructure* ts = find(tiffStructure_,
                                       TiffStructure::Key(extendedTag, group));
        if (ts && ts->newTiffCompFct_) {
            tc = ts->newTiffCompFct_(tag, ts);
        }
        if (!ts && extendedTag != Tag::next) {
            tc = TiffComponent::AutoPtr(new TiffEntry(tag, group));
        }
        return tc;
    } // TiffCanonCreator::create

    TiffRwState::AutoPtr TiffCanonMn::doGetState(uint32_t /*mnOffset*/,
                                                 ByteOrder byteOrder) const
    {
        // Byteorder: No change
        // Offsets  : No change (relative to the start of the TIFF header)
        // Creator  : Canon TIFF component factory
        return TiffRwState::AutoPtr(
            new TiffRwState(byteOrder, 0, TiffCanonCreator::create));
    }

    // *************************************************************************
    // free functions

    TiffComponent* newCanonMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    mnGroup,
                              const byte* /*pData*/,
                              uint32_t    /*size*/, 
                              ByteOrder   /*byteOrder*/)
    {
        return new TiffCanonMn(tag, group, mnGroup);
    }

}                                       // namespace Exiv2
