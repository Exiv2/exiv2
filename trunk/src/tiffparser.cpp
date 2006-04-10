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
#include "image.hpp"
#include "exif.hpp"
#include "tags.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <iostream>
#include <iomanip>
#include <cassert>

/* --------------------------------------------------------------------------

   Todo:

   + Add further child mgmt stuff to TIFF composite: remove, find
   + Review boundary checking, is it better to check the offsets?
   + Define and implement consistent error handling for recursive hierarchy
   + Add Makernote support
   + Make TiffImage a template StandardImage, which can be parametrized with
     a parser and the necessary checking functions to cover all types of 
     images which need to be loaded completely.

   in crwimage.* :

   + Fix CiffHeader according to TiffHeade2
   + Combine Error(15) and Error(33), add format argument %1
   + Search crwimage for todos, fix writeMetadata comment
   + rename all Ciff stuff to Crw for easier reference

   -------------------------------------------------------------------------- */


// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const TiffStructure TiffCreator::tiffStructure_[] = {
        { Tag::root, Group::none, newTiffDirectory, Group::ifd0 },
        {    0x8769, Group::ifd0, newTiffSubIfd,    Group::exif },
        {    0x8825, Group::ifd0, newTiffSubIfd,    Group::gps  },
        {    0xa005, Group::exif, newTiffSubIfd,    Group::iop  },
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

    TiffDirectory::~TiffDirectory()
    {
        Components::iterator b = components_.begin();
        Components::iterator e = components_.end();
        for (Components::iterator i = b; i != e; ++i) {
            delete *i;
        }
        delete pNext_;
    } // TiffDirectory::~TiffDirectory

    TiffEntryBase::~TiffEntryBase()
    {
        if (isAllocated_) {
            delete[] pData_;
        }
        delete pValue_;
    } // TiffEntryBase::~TiffEntryBase

    const uint16_t TiffHeade2::tag_ = 42;

    bool TiffHeade2::read(const byte* pData, uint32_t size)
    {
        if (size < 8) return false;

        if (pData[0] == 0x49 && pData[1] == 0x49) {
            byteOrder_ = littleEndian;
        }
        else if (pData[0] == 0x4d && pData[1] == 0x4d) {
            byteOrder_ = bigEndian;
        }
        else {
            return false;
        }
        if (tag_ != getUShort(pData + 2, byteOrder_)) return false;
        offset_ = getULong(pData + 4, byteOrder_);

        return true;
    } // TiffHeade2::read

    std::string TiffComponent::groupName() const
    {
        // Todo: This mapping should be a table and it belongs somewhere else
        //       Possibly the whole function shouldn't be in this class...
        std::string group;
        switch (group_) {
        case 1: group = "Image"; break;
        case 2: group = "Thumbnail"; break;
        case 3: group = "Photo"; break;
        case 4: group = "GPSInfo"; break;
        case 5: group = "Iop"; break;
        default: group = "Unknown"; break;
        }

        return group;
    }

    void TiffComponent::addChild(TiffComponent::AutoPtr tiffComponent)
    {
        doAddChild(tiffComponent);
    } // TiffComponent::addChild

    void TiffDirectory::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        components_.push_back(tiffComponent.release());
    } // TiffDirectory::doAddChild

    void TiffSubIfd::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        ifd_.addChild(tiffComponent);
    } // TiffSubIfd::doAddChild

    void TiffComponent::addNext(TiffComponent::AutoPtr tiffComponent)
    {
        doAddNext(tiffComponent);
    } // TiffComponent::addNext

    void TiffDirectory::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        pNext_ = tiffComponent.release();
    } // TiffDirectory::doAddNext

    void TiffSubIfd::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        ifd_.addNext(tiffComponent);
    } // TiffSubIfd::doAddNext

    void TiffHeade2::print(std::ostream& os, const std::string& prefix) const
    {
        os << prefix
           << "Header, offset = 0x" << std::setw(8) << std::setfill('0')
           << std::hex << std::right << offset_;

        switch (byteOrder_) {
        case littleEndian:     os << ", little endian encoded"; break;
        case bigEndian:        os << ", big endian encoded"; break;
        case invalidByteOrder: break;
        }
        os << "\n";

    } // TiffHeade2::print

    void TiffComponent::print(std::ostream&      os,
                              ByteOrder          byteOrder,
                              const std::string& prefix) const
    {
        doPrint(os, byteOrder, prefix);
    } // TiffComponent::print

    void TiffEntryBase::printEntry(std::ostream&      os,
                                   ByteOrder          byteOrder,
                                   const std::string& prefix) const
    {
        os << prefix
           << "tag 0x" << std::setw(4) << std::setfill('0')
           << std::hex << std::right << tag()
           << ", type " << TypeInfo::typeName(typeId())
           << ", " << std::dec << count() << " component";
        if (count() > 1) os << "s";
        os <<" in " << size() << " bytes";
        if (size() > 4) os << ", offset " << offset();
        os << "\n";
        if (pValue_ && pValue_->count() < 100) os << prefix << *pValue_;
        else os << prefix << "...";
        os << "\n";

    } // TiffEntryBase::printEntry

    void TiffEntry::doPrint(std::ostream&          os,
                                ByteOrder          byteOrder,
                                const std::string& prefix) const
    {
        TiffEntryBase::printEntry(os, byteOrder, prefix);
    } // TiffEntry::doPrint

    void TiffDirectory::doPrint(std::ostream&      os,
                                ByteOrder          byteOrder,
                                const std::string& prefix) const
    {
        os << prefix << groupName() << " directory with " 
        // cast to make MSVC happy
           << std::dec << static_cast<unsigned int>(components_.size());
        if (components_.size() == 1) os << " entry:\n";
        else os << " entries:\n";
        Components::const_iterator b = components_.begin();
        Components::const_iterator e = components_.end();
        for (Components::const_iterator i = b; i != e; ++i) {
            (*i)->print(os, byteOrder, prefix + "   ");
        }
        if (pNext_) {
            os << prefix << "Next directory:\n";
            pNext_->print(os, byteOrder, prefix);
        }
        else {
            os << prefix << "No next directory.\n";
        }

    } // TiffDirectory::doPrint

    void TiffSubIfd::doPrint(std::ostream&      os,
                             ByteOrder          byteOrder,
                             const std::string& prefix) const
    {
        TiffEntryBase::printEntry(os, byteOrder, prefix);
        ifd_.print(os, byteOrder, prefix);
    } // TiffSubIfd::doPrint

    void TiffComponent::accept(TiffVisitor& visitor)
    {
        doAccept(visitor);
    } // TiffComponent::accept

    void TiffEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitEntry(this);
    } // TiffEntry::doAccept

    void TiffDirectory::doAccept(TiffVisitor& visitor)
    {
        visitor.visitDirectory(this);

        Components::const_iterator b = components_.begin();
        Components::const_iterator e = components_.end();
        for (Components::const_iterator i = b; i != e; ++i) {
            (*i)->accept(visitor);
        }
        if (pNext_) {
            pNext_->accept(visitor);
        }

    } // TiffDirectory::doAccept

    void TiffSubIfd::doAccept(TiffVisitor& visitor)
    {
        visitor.visitSubIfd(this);
        ifd_.accept(visitor);
    } // TiffSubIfd::doAccept

    void TiffMetadataDecoder::visitEntry(TiffEntry* object)
    {
        decodeTiffEntry(object);
    }

    void TiffMetadataDecoder::visitDirectory(TiffDirectory* object)
    {
        // Nothing to do
    }

    void TiffMetadataDecoder::visitSubIfd(TiffSubIfd* object)
    {
        decodeTiffEntry(object);
    }

    void TiffMetadataDecoder::decodeTiffEntry(const TiffEntryBase* object)
    {
        assert(object != 0);

        // Todo: ExifKey should have an appropriate c'tor, it should not be 
        //       necessary to use groupName here
        ExifKey k(object->tag(), object->groupName());
        assert(pImage_ != 0);
        pImage_->exifData().add(k, object->pValue());
    } // TiffMetadataDecoder::decodeTiffEntry

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

}                                       // namespace Exiv2
