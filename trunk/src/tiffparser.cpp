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
  File:      tiffparser.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   15-Mar-06, ahu: created

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

#include "tiffparser.hpp"
#include "tiffcomposite.hpp"
#include "makernote2.hpp"
#include "tiffvisitor.hpp"
#include "tiffimage.hpp"
#include "error.hpp"

// + standard includes
#include <cassert>

/* --------------------------------------------------------------------------

   Todo:

   + Add further child mgmt stuff to TIFF composite: remove
   + Review boundary checking, is it better to check the offsets?
   + Define and implement consistent error handling for recursive hierarchy
   + Make TiffImage a template StandardImage, which can be parametrized with
     a parser and the necessary checking functions to cover all types of 
     images which need to be loaded completely.
   + TiffComponent: should it have end() and setEnd() or pData and size?
   + Can NewTiffCompFct and TiffCompFactoryFct be combined?
   + Create function is repeated when actually only the table changes. Fix it.
   + Is it easier (for writing) to combine all creation tables into one?
   + CR2 Makernotes don't seem to have a next pointer but Canon Jpeg Makernotes
     do. What a mess. (That'll become an issue when it comes to writing to CR2)
   + Sony makernotes in RAW files do not seem to have header like those in Jpegs.
     And maybe no next pointer either.
   + Filtering of large unknown tags: Should be moved to writing/encoding code
     and done only if really needed (i.e., if writing to a Jpeg segment)

   in crwimage.* :

   + Fix CiffHeader according to TiffHeade2
   + Combine Error(15) and Error(33), add format argument %1
   + Search crwimage for todos, fix writeMetadata comment
   + rename all Ciff stuff to Crw for easier reference

   -------------------------------------------------------------------------- */


// *****************************************************************************
// class member definitions
namespace Exiv2 {

    /*
      This table describes the standard TIFF layout and determines the
      corresponding Exiv2 TIFF components. The key of the table consists of the
      first two attributes, (extended) tag and group. Tag is the TIFF tag or one
      of a few extended tags, group identifies the IFD or any other composite
      TIFF component. Each entry of the table defines for a particular tag and
      group combination, which create function is used and what the group of the
      new component is.
     */
    const TiffStructure TiffCreator::tiffStructure_[] = {
        // ext. tag  group             create function       new group
        //---------  --------------    -------------------   --------------
        { Tag::root, Group::none,      newTiffDirectory,     Group::ifd0    },
        {    0x8769, Group::ifd0,      newTiffSubIfd,        Group::exif    },
        {    0x8825, Group::ifd0,      newTiffSubIfd,        Group::gps     },
        {    0xa005, Group::exif,      newTiffSubIfd,        Group::iop     },
        {    0x927c, Group::exif,      newTiffMnEntry,       Group::mn      },
        {    0x0201, Group::ifd1,      newTiffThumbData,     Group::ifd1    },
        {    0x0202, Group::ifd1,      newTiffThumbSize,     Group::ifd1    },
        { Tag::next, Group::ifd0,      newTiffDirectory,     Group::ifd1    },
        { Tag::next, Group::ifd1,      newTiffDirectory,     Group::ignr    },
        { Tag::next, Group::ignr,      newTiffDirectory,     Group::ignr    },
        // SubIfd found in NEF images
        {    0x014a, Group::ifd0,      newTiffSubIfd,        Group::sub0_0  },
        // Canon makernote structure
        {    0x0001, Group::canonmn,   newTiffArrayEntry<2>, Group::canoncs },
        {    0x0004, Group::canonmn,   newTiffArrayEntry<2>, Group::canonsi },
        {    0x000f, Group::canonmn,   newTiffArrayEntry<2>, Group::canoncf },
        {  Tag::all, Group::canoncs,   newTiffArrayElement<unsignedShort>, Group::canoncs },
        {  Tag::all, Group::canonsi,   newTiffArrayElement<unsignedShort>, Group::canonsi },
        {  Tag::all, Group::canoncf,   newTiffArrayElement<unsignedShort>, Group::canoncf },
        // Minolta makernote structure
        {    0x0001, Group::minoltamn, newTiffArrayEntry<4>, Group::minocso },
        {    0x0003, Group::minoltamn, newTiffArrayEntry<4>, Group::minocsn },
        {    0x0004, Group::minoltamn, newTiffArrayEntry<2>, Group::minocs7 },
        {    0x0114, Group::minoltamn, newTiffArrayEntry<2>, Group::minocs5 },
        {  Tag::all, Group::minocso,   newTiffArrayElement<unsignedLong,  bigEndian>, Group::minocso },
        {  Tag::all, Group::minocsn,   newTiffArrayElement<unsignedLong,  bigEndian>, Group::minocsn },
        {  Tag::all, Group::minocs7,   newTiffArrayElement<unsignedShort, bigEndian>, Group::minocs7 },
        {  Tag::all, Group::minocs5,   newTiffArrayElement<unsignedShort, bigEndian>, Group::minocs5 }

    };

    TiffComponent::AutoPtr TiffCreator::create(uint32_t extendedTag,
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
    } // TiffCreator::create

    void TiffParser::decode(Image* pImage,
                            const byte* pData,
                            uint32_t size,
                            TiffCompFactoryFct createFct)
    {
        assert(pImage != 0);
        assert(pData != 0);

        TiffHeade2 tiffHeader;
        if (!tiffHeader.read(pData, size) || tiffHeader.ifdOffset() >= size) {
            throw Error(3, "TIFF");
        }
        TiffComponent::AutoPtr rootDir = createFct(Tag::root, Group::none);
        if (0 == rootDir.get()) return;
        rootDir->setStart(pData + tiffHeader.ifdOffset());

        TiffRwState::AutoPtr state(
            new TiffRwState(tiffHeader.byteOrder(), 0, createFct));
        TiffReader reader(pData, size, rootDir.get(), state);
        rootDir->accept(reader);

        TiffMetadataDecoder decoder(pImage, rootDir.get(), 4096);
        rootDir->accept(decoder);

    } // TiffParser::decode

}                                       // namespace Exiv2
