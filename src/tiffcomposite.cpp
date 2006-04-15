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
  File:      tiffcomposite.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   11-Apr-06, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// Define DEBUG to output debug information to std::cerr, e.g, by calling make
// like this: make DEFS=-DDEBUG tiffcomposite.o
//#define DEBUG

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "tiffcomposite.hpp"
#include "tiffvisitor.hpp"
#include "value.hpp"

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

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

    TiffMakernote::~TiffMakernote()
    {
        delete mn_;
    } // TiffMakernote::~TiffMakernote

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
        case   1: group = "Image"; break;
        case   2: group = "Thumbnail"; break;
        case   3: group = "Photo"; break;
        case   4: group = "GPSInfo"; break;
        case   5: group = "Iop"; break;
        case 257: group = "Olympus"; break;
        default:  group = "Unknown"; break;
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

    void TiffMakernote::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        if (mn_) mn_->addChild(tiffComponent);
    } // TiffMakernote::doAddChild

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

    void TiffMakernote::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        if (mn_) mn_->addNext(tiffComponent);
    } // TiffMakernote::doAddNext

    void TiffComponent::accept(TiffVisitor& visitor)
    {
        if (visitor.go()) doAccept(visitor);    // one for NVI :)
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
        for (Components::const_iterator i = b; visitor.go() && i != e; ++i) {
            (*i)->accept(visitor);
        }
        if (visitor.go()) visitor.visitDirectoryNext(this);
        if (pNext_) {
            pNext_->accept(visitor);
        }
        if (visitor.go()) visitor.visitDirectoryEnd(this);

    } // TiffDirectory::doAccept

    void TiffSubIfd::doAccept(TiffVisitor& visitor)
    {
        visitor.visitSubIfd(this);
        ifd_.accept(visitor);
    } // TiffSubIfd::doAccept

    void TiffMakernote::doAccept(TiffVisitor& visitor)
    {
        visitor.visitMakernote(this);
        if (mn_) mn_->accept(visitor);
    } // TiffMakernote::doAccept

    // *************************************************************************
    // free functions

}                                       // namespace Exiv2
