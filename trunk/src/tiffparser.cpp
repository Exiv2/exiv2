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

   + Fix CiffHeader according to TiffHeade2
   + Combine Error(15) and Error(33), add format argument %1
   + Search crwimage for todos, fix writeMetadata comment
   + rename all Ciff stuff to Crw for easier reference

   -------------------------------------------------------------------------- */


// *****************************************************************************
// class member definitions
namespace Exiv2 {

    void TiffParser::decode(Image* pImage, 
                            const TiffStructure* pTiffStructure,
                            const byte* pData, 
                            uint32_t size)
    {
        assert(pImage != 0);
        assert(pData != 0);

        TiffHeade2 tiffHeader;
        if (!tiffHeader.read(pData, size) || tiffHeader.offset() >= size) {
            throw Error(3, "TIFF");
        }

        TiffComponent::AutoPtr rootDir 
            = create(Tag::root, Group::none, pTiffStructure);
        if (0 == rootDir.get()) return;
        rootDir->read(pData, size, tiffHeader.offset(), tiffHeader.byteOrder());

#ifdef DEBUG
        tiffHeader.print(std::cerr);
        rootDir->print(std::cerr, tiffHeader.byteOrder());
#endif

        rootDir->decode(*pImage, tiffHeader.byteOrder());

    } // TiffParser::decode

    TiffComponent::AutoPtr TiffParser::create(int32_t tag,
                                              uint16_t group,
                                              const TiffStructure* pTiffStructure)
    {
        const TiffStructure* ts = 0;
        for (int i = 0; pTiffStructure[i].tag_ != Tag::none; ++i) {
            if (tag == pTiffStructure[i].tag_ && group == pTiffStructure[i].group_) {
                ts = &pTiffStructure[i];
                break;
            }
        }
        TiffComponent::AutoPtr tc(0);
        if (ts && ts->newTiffCompFct_) {
            tc = ts->newTiffCompFct_(ts->newGroup_, pTiffStructure);
        }
        if (!ts) {
            tc = TiffComponent::AutoPtr(new TiffEntry);
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
        if (isAllocated_) delete[] pData_;
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

    void TiffComponent::read(const byte* pData,
                             uint32_t    size,
                             uint32_t    start,
                             ByteOrder   byteOrder)
    {
        doRead(pData, size, start, byteOrder);
    } // TiffComponent::read

    void TiffEntryBase::doRead(const byte* pData,
                               uint32_t    size,
                               uint32_t    start,
                               ByteOrder   byteOrder)
    {
        if (size - start < 12) throw Error(3, "TIFF");
        const byte* p = pData + start;
        tag_    = getUShort(p, byteOrder);
        p += 2;
        type_   = getUShort(p, byteOrder);
        // todo: check type
        p += 2;
        count_  = getULong(p, byteOrder);
        p += 4;
        offset_ = getULong(p, byteOrder);
#ifdef DEBUG
        std::cout << "TiffEntryBase for "
                  << "tag 0x" << std::hex << tag_
                  << ", type " << std::dec << type_ 
                  << ", count " << count_
                  << ", offset 0x" << std::hex << offset_ 
                  << std::dec"\n";
#endif
        size_ = TypeInfo::typeSize(typeId()) * count();
        if (size_ > 4) {
            if (size < offset() + size_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Warning: Upper boundary of data for "
                          << "directory " << group() << ", " // todo: ExifTags::ifdName(ifdId_)
                          << " entry 0x" << std::setw(4)
                          << std::setfill('0') << std::hex << tag()
                          << " is out of bounds:\n"
                          << " Offset = 0x" << std::setw(8)
                          << std::setfill('0') << std::hex << offset()
                          << ", size = " << std::dec << size_
                          << ", exceeds buffer size by "
                          << offset() + size_ - size
                          << " Bytes; adjusting the size\n";
#endif
                size_ = size - offset();
                // todo: adjust count_, make size_ a multiple of typeSize
            }
            pData_ = pData + offset();
        }
        else {
            pData_ = pData + start + 8;
        }

    } // TiffEntryBase::doRead

    void TiffEntry::doRead(const byte* pData,
                           uint32_t    size,
                           uint32_t    start,
                           ByteOrder   byteOrder)
    {
        TiffEntryBase::doRead(pData, size, start, byteOrder);
    } // TiffEntry::doRead

    void TiffDirectory::doRead(const byte* pData,
                               uint32_t    size,
                               uint32_t    start,
                               ByteOrder   byteOrder)
    {
        if (size < start + 2) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: "
                      << "Directory " << group() << ": " // todo: ExifTags::ifdName(ifdId_)
                      << " IFD exceeds data buffer, cannot read entry count.\n";
#endif
            return;
        }
        uint32_t o = start;
        const uint16_t n = getUShort(pData + o, byteOrder);
        o += 2;

        for (uint16_t i = 0; i < n; ++i) {
            if (size < o + 12) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Directory " << group() << ": " // todo: ExifTags::ifdName(ifdId_)
                          << " IFD entry " << i
                          << " lies outside of the data buffer.\n";
#endif
                return;
            }
            uint16_t tag = getUShort(pData + o, byteOrder);
            TiffComponent::AutoPtr tc 
                = TiffParser::create(tag, group(), pTiffStructure());
            tc->read(pData, size, o, byteOrder);
            components_.push_back(tc.release());
            o += 12;
        }
        if (size < o + 4) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Directory " << group() << ": " // todo: ExifTags::ifdName(ifdId_)
                          << " IFD exceeds data buffer, cannot read next pointer.\n";
#endif
                return;
        }
        uint32_t next = getLong(pData + o, byteOrder);
        if (next) {
            pNext_ = TiffParser::create(Tag::next, group(), pTiffStructure()).release();
            pNext_->read(pData, size, next, byteOrder);
        }

    } // TiffDirectory::doRead

    void TiffSubIfd::doRead(const byte* pData,
                            uint32_t    size,
                            uint32_t    start,
                            ByteOrder   byteOrder)
    {
        TiffEntryBase::doRead(pData, size, start, byteOrder);
        if (typeId() == unsignedLong && count() >= 1) {
            uint32_t offset = getULong(this->pData(), byteOrder);
            ifd_.read(pData, size, offset, byteOrder);
        }
#ifndef SUPPRESS_WARNINGS
        else {
            std::cerr << "Warning: "
                      << "Directory " << group() << ", " // todo: ExifTags::ifdName(ifdId_)
                      << " entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << tag()
                      << " doesn't look like a sub-IFD.";
        }
#endif
    } // TiffSubIfd::read

    void TiffComponent::decode(Image& image, ByteOrder byteOrder) const
    {
        doDecode(image, byteOrder);
    } // TiffComponent::decode

    void TiffEntryBase::doDecode(Image& image, ByteOrder byteOrder) const
    {
        ExifKey k(tag(), "Image"); // todo needs ifdItem
        TypeId t = typeId();
        Value::AutoPtr v = Value::create(t);
        v->read(pData_, size_, byteOrder);
        image.exifData().add(k, v.get());
    } // TiffEntryBase::doDecode

    void TiffEntry::doDecode(Image& image, ByteOrder byteOrder) const
    {
        TiffEntryBase::doDecode(image, byteOrder);
    } // TiffEntry::doDecode

    void TiffDirectory::doDecode(Image& image, ByteOrder byteOrder) const
    {
        Components::const_iterator b = components_.begin();
        Components::const_iterator e = components_.end();
        for (Components::const_iterator i = b; i != e; ++i) {
            (*i)->decode(image, byteOrder);
        }
        if (pNext_) pNext_->decode(image, byteOrder);
    } // TiffDirectory::doDecode

    void TiffSubIfd::doDecode(Image& image, ByteOrder byteOrder) const
    {
        ifd_.decode(image, byteOrder);
    } // TiffSubIfd::doDecode

    void TiffHeade2::print(std::ostream& os, const std::string& prefix) const
    {
        os << prefix
           << "Header, offset = 0x" << std::setw(8) << std::setfill('0')
           << std::hex << std::right << offset_ << "\n";
    } // TiffHeade2::print

    void TiffComponent::print(std::ostream&      os,
                              ByteOrder          byteOrder,
                              const std::string& prefix) const
    {
        doPrint(os, byteOrder, prefix);
    } // TiffComponent::print

    void TiffEntryBase::doPrint(std::ostream&      os,
                                ByteOrder          byteOrder,
                                const std::string& prefix) const
    {
        os << prefix
           << "tag = 0x" << std::setw(4) << std::setfill('0')
           << std::hex << std::right << tag()
           << ", type = " << TypeInfo::typeName(typeId())
           << ", count = " << std::dec << count()
           << ", offset = " << offset() << "\n";

        TypeId t = typeId();
        Value::AutoPtr v = Value::create(t);
        v->read(pData_, size_, byteOrder);
        if (v->size() < 100) {
            os << prefix << *v << "\n";
        }
    } // TiffEntryBase::doPrint

    void TiffEntry::doPrint(std::ostream&          os,
                                ByteOrder          byteOrder,
                                const std::string& prefix) const
    {
        TiffEntryBase::doPrint(os, byteOrder, prefix);
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
        TiffEntryBase::doPrint(os, byteOrder, prefix);
        ifd_.print(os, byteOrder, prefix);
    } // TiffSubIfd::doPrint

    // *************************************************************************
    // free functions

    TiffComponent::AutoPtr newTiffDirectory(uint16_t group,
                                            const TiffStructure* pTiffStructure)
    {
        return TiffComponent::AutoPtr(new TiffDirectory(group, pTiffStructure));
    }

    TiffComponent::AutoPtr newTiffSubIfd(uint16_t group,
                                         const TiffStructure* pTiffStructure)
    {
        return TiffComponent::AutoPtr(new TiffSubIfd(group, pTiffStructure));
    }

}                                       // namespace Exiv2
