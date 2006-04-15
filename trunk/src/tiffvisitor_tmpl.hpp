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
/*!
  @file    tiffvisitor_tmpl.hpp
  @brief   
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    11-Apr-06, ahu: created
 */
#ifndef TIFFVISITOR_TMPL_HPP_
#define TIFFVISITOR_TMPL_HPP_

// *****************************************************************************
// included header files
#include "tiffvisitor.hpp"
#include "tiffcomposite.hpp"
#include "makernote2.hpp"
#include "value.hpp"
#include "types.hpp"

// + standard includes
#include <iostream>
#include <iomanip>
#include <cassert>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// template, inline and free functions

    template<typename CreationPolicy>
    TiffReader<CreationPolicy>::TiffReader(const byte*    pData,
                                           uint32_t       size,
                                           ByteOrder      byteOrder,
                                           TiffComponent* pRoot)
        : pData_(pData),
          size_(size),
          pLast_(pData + size - 1),
          byteOrder_(byteOrder),
          pRoot_(pRoot)
    {
        assert(pData);
        assert(size > 0);
    } // TiffReader::TiffReader

    template<typename CreationPolicy>
    void TiffReader<CreationPolicy>::visitEntry(TiffEntry* object)
    {
        readTiffEntry(object);
    }

    template<typename CreationPolicy>
    void TiffReader<CreationPolicy>::visitDirectory(TiffDirectory* object)
    {
        assert(object != 0);

        const byte* p = object->start();
        assert(p >= pData_);

        if (p + 2 > pLast_) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: "
                      << "Directory " << object->groupName() << ": "
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
                          << "Directory " << object->groupName() << ": "
                          << " IFD entry " << i
                          << " lies outside of the data buffer.\n";
#endif
                return;
            }
            uint16_t tag = getUShort(p, byteOrder_);
            TiffComponent::AutoPtr tc = CreationPolicy::create(tag, object->group());
            tc->setStart(p);
            object->addChild(tc);
            p += 12;
        }

        if (p + 4 > pLast_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Directory " << object->groupName() << ": "
                          << " IFD exceeds data buffer, cannot read next pointer.\n";
#endif
                return;
        }
        uint32_t next = getLong(p, byteOrder_);
        if (next) {
            TiffComponent::AutoPtr tc = CreationPolicy::create(Tag::next, object->group());
            if (next > size_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Directory " << object->groupName() << ": "
                          << " Next pointer is out of bounds.\n";
#endif
                return;
            }
            tc->setStart(pData_ + next);
            object->addNext(tc);
        }

    } // TiffReader::visitDirectory

    template<typename CreationPolicy>
    void TiffReader<CreationPolicy>::visitSubIfd(TiffSubIfd* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        if (object->typeId() == unsignedLong && object->count() >= 1) {
            uint32_t offset = getULong(object->pData(), byteOrder_);
            if (offset > size_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Directory " << object->groupName()
                          << ", entry 0x" << std::setw(4)
                          << std::setfill('0') << std::hex << object->tag()
                          << " Sub-IFD pointer is out of bounds; ignoring it.\n";
#endif
                return;
            }
            object->ifd_.setStart(pData_ + offset);
        }
#ifndef SUPPRESS_WARNINGS
        else {
            std::cerr << "Warning: "
                      << "Directory " << object->groupName()
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << object->tag()
                      << " doesn't look like a sub-IFD.";
        }
#endif

    } // TiffReader::visitSubIfd

    template<typename CreationPolicy>
    void TiffReader<CreationPolicy>::visitMakernote(TiffMakernote* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        // Find the camera model
        TiffFinder finder(0x010f, Group::ifd0);
        pRoot_->accept(finder);
        TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
        std::string make;
        if (te && te->pValue()) {
            make = te->pValue()->toString();
            // create concrete makernote, based on model and makernote contents
            object->mn_ = TiffMnCreator::create(object->tag(), 
                                                object->newGroup_,
                                                make,
                                                object->pData(),
                                                object->size(),
                                                byteOrder_);
        }
        if (object->mn_) object->mn_->setStart(object->pData());

    } // TiffReader::visitMakernote

    template<typename CreationPolicy>
    void TiffReader<CreationPolicy>::visitOlympusMn(TiffOlympusMn* object)
    {
        object->header_.read(object->start(), pLast_ - object->start());
        if (!object->header_.check()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Olympus Makernote header check failed.\n";
#endif
            return;   // todo: signal error to parent, delete object
        }
        object->ifd_.setStart(object->start() + object->header_.offset());
        
    } // TiffReader::visitOlympusMn

    template<typename CreationPolicy>
    void TiffReader<CreationPolicy>::readTiffEntry(TiffEntryBase* object)
    {
        assert(object != 0);

        byte* p = const_cast<byte*>(object->start());
        assert(p >= pData_);

        if (p + 12 > pLast_) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Entry in directory " << object->groupName()
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
        object->size_ = TypeInfo::typeSize(object->typeId()) * object->count();
        object->offset_ = getULong(p, byteOrder_);
        object->pData_ = p;
        if (object->size() > 4) {
            if (object->offset() >= size_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: Offset of "
                          << "directory " << object->groupName() << ", "
                          << " entry 0x" << std::setw(4)
                          << std::setfill('0') << std::hex << object->tag()
                          << " is out of bounds:\n"
                          << "Offset = 0x" << std::setw(8)
                          << std::setfill('0') << std::hex << object->offset()
                          << "; truncating the entry\n";
#endif
                object->size_ = 0;
                object->count_ = 0;
                object->offset_ = 0;
                return;
            }
            object->pData_ = pData_ + object->offset();
            if (object->pData() + object->size() > pLast_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Warning: Upper boundary of data for "
                          << "directory " << object->groupName() << ", "
                          << " entry 0x" << std::setw(4)
                          << std::setfill('0') << std::hex << object->tag()
                          << " is out of bounds:\n"
                          << "Offset = 0x" << std::setw(8)
                          << std::setfill('0') << std::hex << object->offset()
                          << ", size = " << std::dec << object->size()
                          << ", exceeds buffer size by "
                // cast to make MSVC happy
                          << static_cast<uint32_t>(object->pData() + object->size() - pLast_)
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

}                                       // namespace Exiv2

#endif                                  // #ifndef TIFFVISITOR_TMPL_HPP_
