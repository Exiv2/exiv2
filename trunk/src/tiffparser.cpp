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
   + Better handling of TiffStructure
   + Add Makernote support

   in crwimage.* :

   + Fix CiffHeader according to TiffHeade2
   + Combine Error(15) and Error(33), add format argument %1
   + Search crwimage for todos, fix writeMetadata comment
   + rename all Ciff stuff to Crw for easier reference

   -------------------------------------------------------------------------- */


// *****************************************************************************
// class member definitions
namespace Exiv2 {

    void TiffParser::decode(const byte* pData,
                            uint32_t size,
                            const TiffStructure* pTiffStructure,
                            TiffVisitor& decoder)
    {
        assert(pData != 0);

        TiffHeade2 tiffHeader;
        if (!tiffHeader.read(pData, size) || tiffHeader.offset() >= size) {
            throw Error(3, "TIFF");
        }

        TiffComponent::AutoPtr rootDir
            = TiffParser::create(Tag::root, Group::none, pTiffStructure);
        if (0 == rootDir.get()) return;

        TiffReader reader(pData,
                          size,
                          tiffHeader.byteOrder(),
                          pTiffStructure);
        rootDir->setStart(pData + tiffHeader.offset());
        rootDir->accept(reader);

#ifdef DEBUG
        tiffHeader.print(std::cerr);
        rootDir->print(std::cerr, tiffHeader.byteOrder());
#endif

        rootDir->accept(decoder);

    } // TiffParser::decode

    TiffComponent::AutoPtr TiffParser::create(uint32_t extendedTag,
                                              uint16_t group,
                                              const TiffStructure* pTiffStructure)
    {
        const TiffStructure* ts = 0;
        int idx = 0;
        for (; pTiffStructure[idx].extendedTag_ != Tag::none; ++idx) {
            if (   extendedTag == pTiffStructure[idx].extendedTag_
                && group == pTiffStructure[idx].group_) {
                ts = &pTiffStructure[idx];
                break;
            }
        }

        TiffComponent::AutoPtr tc(0);
        if (ts && ts->newTiffCompFct_) {
            tc = ts->newTiffCompFct_(ts);
        }
        if (!ts) {
            uint16_t tag = static_cast<uint16_t>(extendedTag & 0xffff);
            tc = TiffComponent::AutoPtr(new TiffEntry(tag, group));
        }
        return tc;
    } // TiffParser::create

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
        os << prefix << "Directory " << group()
           << " with " << components_.size() << " entries.\n";
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

        // Todo: ExifKey should have an appropriate c'tor, this mapping should
        //       be a table and it belongs somewhere else
        std::string group;
        switch (object->group()) {
        case 1: group = "Image"; break;
        case 2: group = "Thumbnail"; break;
        case 3: group = "Photo"; break;
        case 4: group = "GPSInfo"; break;
        case 5: group = "Iop"; break;
        }

        ExifKey k(object->tag(), group);
        assert(pImage_ != 0);
        pImage_->exifData().add(k, object->pValue());
    } // TiffMetadataDecoder::decodeTiffEntry

    TiffReader::TiffReader(const byte* pData,
                           uint32_t    size,
                           ByteOrder   byteOrder,
                           const TiffStructure* pTiffStructure)
        : pData_(pData),
          size_(size),
          pLast_(pData + size - 1),
          byteOrder_(byteOrder),
          pTiffStructure_(pTiffStructure)
    {
        assert(pData);
        assert(size > 0);
        assert(pTiffStructure);
    } // TiffReader::TiffReader

    void TiffReader::visitEntry(TiffEntry* object)
    {
        readTiffEntry(object);
    }

    void TiffReader::visitDirectory(TiffDirectory* object)
    {
        assert(object != 0);

        byte* p = const_cast<byte*>(object->start());
        assert(p >= pData_);

        if (p + 2 > pLast_) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: "
                      << "Directory " << object->group() << ": " // todo: ExifTags::ifdName(ifdId_)
                      << " IFD exceeds data buffer, cannot read entry count.\n";
#endif
            return;
        }
        const uint16_t n = getUShort(p, byteOrder_);
        p += 2;

        for (uint16_t i = 0; i < n; ++i) {
            if (p + 12 > pLast_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Directory " << object->group() << ": " // todo: ExifTags::ifdName(ifdId_)
                          << " IFD entry " << i
                          << " lies outside of the data buffer.\n";
#endif
                return;
            }
            uint16_t tag = getUShort(p, byteOrder_);
            TiffComponent::AutoPtr tc
                = TiffParser::create(tag, object->group(), pTiffStructure_);
            tc->setStart(p);
            object->addChild(tc);

            p += 12;
        }

        if (p + 4 > pLast_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Directory " << object->group() << ": " // todo: ExifTags::ifdName(ifdId_)
                          << " IFD exceeds data buffer, cannot read next pointer.\n";
#endif
                return;
        }
        uint32_t next = getLong(p, byteOrder_);
        if (next) {
            TiffComponent::AutoPtr tc
                = TiffParser::create(Tag::next, object->group(), pTiffStructure_);
            tc->setStart(p);
            object->addNext(tc);
        }

    } // TiffReader::visitDirectory

    void TiffReader::visitSubIfd(TiffSubIfd* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        if (object->typeId() == unsignedLong && object->count() >= 1) {
            uint32_t offset = getULong(object->pData(), byteOrder_);
            object->ifd_.setStart(pData_ + offset);
        }
#ifndef SUPPRESS_WARNINGS
        else {
            std::cerr << "Warning: "
                      << "Directory " << object->group() << ", " // todo: ExifTags::ifdName(ifdId_)
                      << " entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << object->tag()
                      << " doesn't look like a sub-IFD.";
        }
#endif

    } // TiffReader::visitSubIfd

    void TiffReader::readTiffEntry(TiffEntryBase* object)
    {
        assert(object != 0);

        byte* p = const_cast<byte*>(object->start());
        assert(p >= pData_);

        if (p + 12 > pLast_) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Entry in directory " << object->group() // todo: ExifTags::ifdName(ifdId_)
                      << "requests access to memory beyond the data buffer. "
                      << "Skipping entry.\n";
#endif
            return;
        }
        // Component already has tag
        p += 2;
        object->type_ = getUShort(p, byteOrder_);
        // todo: check type
        p += 2;
        object->count_ = getULong(p, byteOrder_);
        p += 4;
        object->offset_ = getULong(p, byteOrder_);
        object->size_ = TypeInfo::typeSize(object->typeId()) * object->count();
        object->pData_ = p;
        if (object->size() > 4) {
            object->pData_ = pData_ + object->offset();
            if (object->pData() + object->size() > pLast_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Warning: Upper boundary of data for "
                          << "directory " << object->group() << ", " // todo: ExifTags::ifdName(ifdId_)
                          << " entry 0x" << std::setw(4)
                          << std::setfill('0') << std::hex << object->tag()
                          << " is out of bounds:\n"
                          << " Offset = 0x" << std::setw(8)
                          << std::setfill('0') << std::hex << object->offset()
                          << ", size = " << std::dec << object->size()
                          << ", exceeds buffer size by "
                          << object->pData() + object->size() - pLast_
                          << " Bytes; adjusting the size\n";
#endif
                object->size_ = size_ - object->offset();
                // todo: adjust count_, make size_ a multiple of typeSize
            }
        }
        Value::AutoPtr v = Value::create(object->typeId());
        if (v.get()) {
            v->read(object->pData(), object->size(), byteOrder_);
            object->pValue_ = v.release();
        }

    } // TiffReader::readTiffEntry

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
