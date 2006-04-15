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

// Define DEBUG to output debug information to std::cerr, e.g, by calling make
// like this: make DEFS=-DDEBUG tiffparser.o
//#define DEBUG

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "tiffparser.hpp"
#include "tiffcomposite.hpp"

// + standard includes
#include <cassert>

/* --------------------------------------------------------------------------

   Todo:

   + Add further child mgmt stuff to TIFF composite: remove
   + Review boundary checking, is it better to check the offsets?
   + Define and implement consistent error handling for recursive hierarchy
   + Add Makernote support
   + Make TiffImage a template StandardImage, which can be parametrized with
     a parser and the necessary checking functions to cover all types of 
     images which need to be loaded completely.
   + Decide what tag and group should be assigned to TiffMnEntry and
     concrete Makernotes and which of them should derive from base-entry
     - TiffMnEntry tag 0x927c, group exif, derives from tiffentry: because 
       create needs the entry
     - ConcreteMn tag 0, group Mn, derives from component so that the plain entry
       is only kept in one place,
       if it contains an Ifd, that has a different group (create fct knows which)
   + Implementation of concrete makernotes: Base class TiffIfdMakernote? 
     Why is the hierarchy MnHeader needed?
   + TiffComponent: should it have end() and setEnd() or pData and size??

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
        // ext. tag        group  create function   new group
        //---------  -----------  ----------------  -----------
        { Tag::root, Group::none, newTiffDirectory, Group::ifd0 },
        {    0x8769, Group::ifd0, newTiffSubIfd,    Group::exif },
        {    0x8825, Group::ifd0, newTiffSubIfd,    Group::gps  },
        {    0xa005, Group::exif, newTiffSubIfd,    Group::iop  },
        {    0x927c, Group::exif, newTiffMnEntry,   Group::mn   },
        { Tag::next, Group::ifd0, newTiffDirectory, Group::ifd0 }
    };

    bool TiffStructure::operator==(const TiffStructure::Key& key) const
    {
        return key.e_ == extendedTag_ && key.g_ == group_; 
    }

    TiffComponent::AutoPtr TiffCreator::create(uint32_t extendedTag,
                                               uint16_t group)
    {
        const TiffStructure* ts = find(tiffStructure_,
                                       TiffStructure::Key(extendedTag, group));
        TiffComponent::AutoPtr tc(0);
        if (ts && ts->newTiffCompFct_) {
            tc = ts->newTiffCompFct_(ts);
        }
        if (!ts) {
            uint16_t tag = static_cast<uint16_t>(extendedTag & 0xffff);
            tc = TiffComponent::AutoPtr(new TiffEntry(tag, group));
        }
        return tc;
    } // TiffCreator::create

    // *************************************************************************
    // free functions

    TiffComponent::AutoPtr newTiffDirectory(const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffDirectory(ts->tag(), ts->newGroup_));
    }

    TiffComponent::AutoPtr newTiffSubIfd(const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffSubIfd(ts->tag(),
                                                     ts->group_,
                                                     ts->newGroup_));
    }

    TiffComponent::AutoPtr newTiffMnEntry(const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffMnEntry(ts->tag(),
                                                      ts->group_,
                                                      ts->newGroup_));
    }

}                                       // namespace Exiv2
