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
  File:      tiffvisitor.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   11-Apr-06, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// Define DEBUG to output debug information to std::cerr, e.g, by calling make
// like this: make DEFS=-DDEBUG tiffvisitor.o
//#define DEBUG

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "tiffvisitor.hpp"
#include "tiffcomposite.hpp"
#include "makernote2.hpp"
#include "exif.hpp"
#include "value.hpp"
#include "image.hpp"

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    void TiffFinder::init(uint16_t tag, uint16_t group)
    {
        tag_ = tag;
        group_ = group;
        tiffComponent_ = 0;
    }

    void TiffFinder::findObject(TiffComponent* object)
    {
        if (object->tag() == tag_ && object->group() == group_) {
            tiffComponent_ = object;
            setGo(false);
        }
    }

    void TiffFinder::visitEntry(TiffEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitDirectory(TiffDirectory* object)
    {
        findObject(object);
    }

    void TiffFinder::visitSubIfd(TiffSubIfd* object)
    {
        findObject(object);
    }

    void TiffFinder::visitMnEntry(TiffMnEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitIfdMakernote(TiffIfdMakernote* object)
    {
        findObject(object);
    }

    void TiffFinder::visitArrayEntry(TiffArrayEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitArrayElement(TiffArrayElement* object)
    {
        findObject(object);
    }

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

    void TiffMetadataDecoder::visitMnEntry(TiffMnEntry* object)
    {
        if (!object->mn_) decodeTiffEntry(object);
    }

    void TiffMetadataDecoder::visitIfdMakernote(TiffIfdMakernote* object)
    {
        // Nothing to do
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

    void TiffMetadataDecoder::visitArrayEntry(TiffArrayEntry* object)
    {
        assert(object != 0);

        // Array entry degenerates to a normal entry if type is not unsignedShort
        if (object->typeId() != unsignedShort) {
            decodeTiffEntry(object);
        }
    }

    void TiffMetadataDecoder::visitArrayElement(TiffArrayElement* object)
    {
        decodeTiffEntry(object);
    }

    const std::string TiffPrinter::indent_("   ");

    void TiffPrinter::incIndent()
    {
        prefix_ += indent_;
    } // TiffPrinter::incIndent

    void TiffPrinter::decIndent()
    {
        if (prefix_.length() >= indent_.length()) {
            prefix_.erase(prefix_.length() - indent_.length(), indent_.length());
        }
    } // TiffPrinter::decIndent

    void TiffPrinter::visitEntry(TiffEntry* object)
    {
        printTiffEntry(object, prefix());
    } // TiffPrinter::visitEntry

    void TiffPrinter::visitDirectory(TiffDirectory* object)
    {
        assert(object != 0);

        os_ << prefix() << object->groupName() << " directory with " 
        // cast to make MSVC happy
           << std::dec << static_cast<unsigned int>(object->components_.size());
        if (object->components_.size() == 1) os_ << " entry:\n";
        else os_ << " entries:\n";
        incIndent();

    } // TiffPrinter::visitDirectory

    void TiffPrinter::visitDirectoryNext(TiffDirectory* object)
    {
        decIndent();
        if (object->pNext_) os_ << prefix() << "Next directory:\n";
        else os_ << prefix() << "No next directory\n";        
    } // TiffPrinter::visitDirectoryNext

    void TiffPrinter::visitDirectoryEnd(TiffDirectory* object)
    {
        // Nothing to do
    } // TiffPrinter::visitDirectoryEnd

    void TiffPrinter::visitSubIfd(TiffSubIfd* object)
    {
        os_ << prefix() << "Sub-IFD ";
        printTiffEntry(object);
    } // TiffPrinter::visitSubIfd

    void TiffPrinter::visitMnEntry(TiffMnEntry* object)
    {
        if (!object->mn_) printTiffEntry(object, prefix());
        else os_ << prefix() << "Makernote ";
    } // TiffPrinter::visitMnEntry

    void TiffPrinter::visitIfdMakernote(TiffIfdMakernote* object)
    {
        os_ << prefix() << "Todo: Print IFD makernote header\n";
    } // TiffPrinter::visitIfdMakernote

    void TiffPrinter::printTiffEntry(TiffEntryBase* object,
                                     const std::string& px) const
    {
        assert(object != 0);

        os_ << px << object->groupName()
            << " tag 0x" << std::setw(4) << std::setfill('0')
            << std::hex << std::right << object->tag()
            << ", type " << TypeInfo::typeName(object->typeId())
            << ", " << std::dec << object->count() << " component";
        if (object->count() > 1) os_ << "s";
        os_ << " in " << object->size() << " bytes";
        if (object->size() > 4) os_ << ", offset " << object->offset();
        os_ << "\n";
        const Value* vp = object->pValue();
        if (vp && vp->count() < 100) os_ << prefix() << *vp;
        else os_ << prefix() << "...";
        os_ << "\n";

    } // TiffPrinter::printTiffEntry

    void TiffPrinter::visitArrayEntry(TiffArrayEntry* object)
    {
        // Array entry degenerates to a normal entry if type is not unsignedShort
        if (object->typeId() != unsignedShort) {
            printTiffEntry(object, prefix());
        }
        else {
            os_ << prefix() << "Array Entry " << object->groupName()
                << " tag 0x" << std::setw(4) << std::setfill('0')
                << std::hex << std::right << object->tag() << "\n";
        }
    } // TiffPrinter::visitArrayEntry

    void TiffPrinter::visitArrayElement(TiffArrayElement* object)
    {
        printTiffEntry(object, prefix());        
    } // TiffPrinter::visitArrayElement

    TiffReader::TiffReader(const byte*    pData,
                           uint32_t       size,
                           TiffComponent* pRoot,
                           TiffRwState::AutoPtr state)
        : pData_(pData),
          size_(size),
          pLast_(pData + size - 1),
          pRoot_(pRoot),
          pState_(state.release()),
          pOrigState_(pState_)
    {
        assert(pData_);
        assert(size_ > 0);

    } // TiffReader::TiffReader

    TiffReader::~TiffReader()
    {
        if (pOrigState_ != pState_) delete pOrigState_;
        delete pState_;
    }

    void TiffReader::resetState() {
        if (pOrigState_ != pState_) delete pState_;
        pState_ = pOrigState_;
    }

    void TiffReader::changeState(TiffRwState::AutoPtr state)
    {
        if (state.get() != 0) {
            if (pOrigState_ != pState_) delete pState_;
            pState_ = state.release();
        }
    }

    ByteOrder TiffReader::byteOrder() const
    {
        assert(pState_);
        return pState_->byteOrder_;
    }
     
    uint32_t TiffReader::baseOffset() const
    {
        assert(pState_);
        return pState_->baseOffset_;
    }

    TiffComponent::AutoPtr TiffReader::create(uint32_t extendedTag, 
                                              uint16_t group) const
    {
        assert(pState_);
        assert(pState_->createFct_);
        return pState_->createFct_(extendedTag, group);
    }

    void TiffReader::visitEntry(TiffEntry* object)
    {
        readTiffEntry(object);
    }

    void TiffReader::visitDirectory(TiffDirectory* object)
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
        const uint16_t n = getUShort(p, byteOrder());
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
            uint16_t tag = getUShort(p, byteOrder());
            TiffComponent::AutoPtr tc = create(tag, object->group());
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
        uint32_t next = getLong(p, byteOrder());
        if (next) {
            TiffComponent::AutoPtr tc = create(Tag::next, object->group());
            if (baseOffset() + next > size_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Directory " << object->groupName() << ": "
                          << " Next pointer is out of bounds.\n";
#endif
                return;
            }
            tc->setStart(pData_ + baseOffset() + next);
            object->addNext(tc);
        }

    } // TiffReader::visitDirectory

    void TiffReader::visitSubIfd(TiffSubIfd* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        if (object->typeId() == unsignedLong && object->count() >= 1) {
            uint32_t offset = getULong(object->pData(), byteOrder());
            if (baseOffset() + offset > size_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Directory " << object->groupName()
                          << ", entry 0x" << std::setw(4)
                          << std::setfill('0') << std::hex << object->tag()
                          << " Sub-IFD pointer is out of bounds; ignoring it.\n";
#endif
                return;
            }
            object->ifd_.setStart(pData_ + baseOffset() + offset);
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

    void TiffReader::visitMnEntry(TiffMnEntry* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        // Find camera make
        TiffFinder finder(0x010f, Group::ifd0);
        pRoot_->accept(finder);
        TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
        std::string make;
        if (te && te->pValue()) {
            make = te->pValue()->toString();
            // create concrete makernote, based on make and makernote contents
            object->mn_ = TiffMnCreator::create(object->tag(), 
                                                object->mnGroup_,
                                                make,
                                                object->pData(),
                                                object->size(),
                                                byteOrder());
        }
        if (object->mn_) object->mn_->setStart(object->pData());

    } // TiffReader::visitMnEntry

    void TiffReader::visitIfdMakernote(TiffIfdMakernote* object)
    {
        assert(object != 0);

        object->readHeader(object->start(), pLast_ - object->start(), byteOrder());
        if (!object->checkHeader()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: IFD Makernote header check failed.\n";
#endif
            return;   // todo: signal error to parent, delete object
        }
        // Modify reader for Makernote peculiarities, byte order, offset,
        // component factory
        changeState(object->getState(object->start() - pData_, byteOrder()));
        object->ifd_.setStart(object->start() + object->ifdOffset());

    } // TiffReader::visitIfdMakernote

    void TiffReader::visitIfdMakernoteEnd(TiffIfdMakernote* object)
    {
        // Reset state (byte order, create function, offset) back to that
        // for the image
        resetState();        
    } // TiffReader::visitIfdMakernoteEnd

    void TiffReader::readTiffEntry(TiffEntryBase* object)
    {
        assert(object != 0);

        const byte* p = object->start();
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
        object->type_ = getUShort(p, byteOrder());
        // todo: check type
        p += 2;
        object->count_ = getULong(p, byteOrder());
        p += 4;
        object->size_ = TypeInfo::typeSize(object->typeId()) * object->count();
        object->offset_ = getULong(p, byteOrder());
        object->pData_ = p;
        if (object->size() > 4) {
            if (baseOffset() + object->offset() >= size_) {
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
            object->pData_ = pData_ + baseOffset() + object->offset();
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
                object->size_ = pLast_ - object->pData() + 1;
                // todo: adjust count_, make size_ a multiple of typeSize
            }
        }
        Value::AutoPtr v = Value::create(object->typeId());
        if (v.get()) {
            v->read(object->pData(), object->size(), byteOrder());
            object->pValue_ = v.release();
        }

    } // TiffReader::readTiffEntry

    void TiffReader::visitArrayEntry(TiffArrayEntry* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        if (object->typeId() == unsignedShort) {
            for (uint16_t i = 0; i < static_cast<uint16_t>(object->count()); ++i) {
                uint16_t tag = i;
                TiffComponent::AutoPtr tc = create(tag, object->elGroup());
                tc->setStart(object->pData() + i * 2);
                object->addChild(tc);
            }
        }

    } // TiffReader::visitArrayEntry

    void TiffReader::visitArrayElement(TiffArrayElement* object)
    {
        assert(object != 0);

        const byte* p = object->start();
        assert(p >= pData_);

        if (p + 2 > pLast_) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Array element in group " << object->groupName()
                      << "requests access to memory beyond the data buffer. "
                      << "Skipping element.\n";
#endif
            return;
        }
        object->type_ = unsignedShort;
        object->count_ = 1;
        object->size_ = TypeInfo::typeSize(object->typeId()) * object->count();
        object->offset_ = 0;
        object->pData_ = p;
        Value::AutoPtr v = Value::create(object->typeId());
        if (v.get()) {
            v->read(object->pData(), object->size(), byteOrder());
            object->pValue_ = v.release();
        }

    } // TiffReader::visitArrayElement

}                                       // namespace Exiv2
