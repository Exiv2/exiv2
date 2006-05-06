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

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "tiffcomposite.hpp"
#include "tiffvisitor.hpp"
#include "makernote2.hpp"
#include "value.hpp"

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    bool TiffStructure::operator==(const TiffStructure::Key& key) const
    {
        return    key.e_   == extendedTag_ && key.g_ == group_ 
               || Tag::all == extendedTag_ && key.g_ == group_;
    }

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

    TiffMnEntry::~TiffMnEntry()
    {
        delete mn_;
    } // TiffMnEntry::~TiffMnEntry

    TiffArrayEntry::~TiffArrayEntry()
    {
        Components::iterator b = elements_.begin();
        Components::iterator e = elements_.end();
        for (Components::iterator i = b; i != e; ++i) {
            delete *i;
        }
    } // TiffArrayEntry::~TiffArrayEntry

    std::string TiffComponent::groupName() const
    {
        // Todo: This mapping should be a table and it belongs somewhere else
        //       Possibly the whole function shouldn't be in this class...
        std::string group;
        switch (group_) {
        case   1: group = "Image";     break;
        case   2: group = "Thumbnail"; break;
        case   3: group = "Photo";     break;
        case   4: group = "GPSInfo";   break;
        case   5: group = "Iop";       break;
        case 257: group = "Olympus";   break;
        case 258: group = "Fujifilm";  break;
        case 259: group = "Canon";     break;
        case 260: group = "CanonCs1";  break;
        case 261: group = "CanonCs2";  break;
        case 262: group = "CanonCf";   break;
        // 263 not needed (nikonmn)
        case 264: group = "Nikon1";    break;
        case 265: group = "Nikon2";    break;
        case 266: group = "Nikon3";    break;
        case 267: group = "Panasonic"; break;
        case 268: group = "Sigma";     break;
        // 269 not needed (sonymn)
        case 270: group = "Sony";      break;
        case 271: group = "Sony";      break;
        case 272: group = "Minolta";   break;
        default:  group = "Unknown";   break;
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

    void TiffMnEntry::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        if (mn_) mn_->addChild(tiffComponent);
    } // TiffMnEntry::doAddChild

    void TiffArrayEntry::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        elements_.push_back(tiffComponent.release());
    } // TiffArrayEntry::doAddChild

    void TiffComponent::addNext(TiffComponent::AutoPtr tiffComponent)
    {
        doAddNext(tiffComponent);
    } // TiffComponent::addNext

    void TiffDirectory::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        if (hasNext_) pNext_ = tiffComponent.release();
    } // TiffDirectory::doAddNext

    void TiffSubIfd::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        ifd_.addNext(tiffComponent);
    } // TiffSubIfd::doAddNext

    void TiffMnEntry::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        if (mn_) mn_->addNext(tiffComponent);
    } // TiffMnEntry::doAddNext

    void TiffComponent::accept(TiffVisitor& visitor)
    {
        if (visitor.go()) doAccept(visitor);    // one for NVI :)
    } // TiffComponent::accept

    void TiffEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitEntry(this);
    } // TiffEntry::doAccept

    void TiffDataEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitDataEntry(this);
    } // TiffDataEntry::doAccept

    void TiffSizeEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitSizeEntry(this);
    } // TiffSizeEntry::doAccept

    void TiffDirectory::doAccept(TiffVisitor& visitor)
    {
        visitor.visitDirectory(this);
        Components::const_iterator b = components_.begin();
        Components::const_iterator e = components_.end();
        for (Components::const_iterator i = b; visitor.go() && i != e; ++i) {
            (*i)->accept(visitor);
        }
        if (visitor.go()) visitor.visitDirectoryNext(this);
        if (pNext_) pNext_->accept(visitor);
        if (visitor.go()) visitor.visitDirectoryEnd(this);

    } // TiffDirectory::doAccept

    void TiffSubIfd::doAccept(TiffVisitor& visitor)
    {
        visitor.visitSubIfd(this);
        ifd_.accept(visitor);
    } // TiffSubIfd::doAccept

    void TiffMnEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitMnEntry(this);
        if (mn_) mn_->accept(visitor);
        if (!visitor.go()) {
            delete mn_;
            mn_ = 0;
            visitor.setGo(true);
        }
    } // TiffMnEntry::doAccept

    void TiffArrayEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitArrayEntry(this);
        Components::const_iterator b = elements_.begin();
        Components::const_iterator e = elements_.end();
        for (Components::const_iterator i = b; visitor.go() && i != e; ++i) {
            (*i)->accept(visitor);
        }
    } // TiffArrayEntry::doAccept

    void TiffArrayElement::doAccept(TiffVisitor& visitor)
    {
        visitor.visitArrayElement(this);
    } // TiffArrayElement::doAccept

    // *************************************************************************
    // free functions

    TiffComponent::AutoPtr newTiffDirectory(uint16_t tag,
                                            const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffDirectory(tag, ts->newGroup_));
    }

    TiffComponent::AutoPtr newTiffSubIfd(uint16_t tag,
                                         const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffSubIfd(tag,
                                                     ts->group_,
                                                     ts->newGroup_));
    }

    TiffComponent::AutoPtr newTiffMnEntry(uint16_t tag,
                                          const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffMnEntry(tag,
                                                      ts->group_,
                                                      ts->newGroup_));
    }

    TiffComponent::AutoPtr newTiffArrayEntry(uint16_t tag,
                                             const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffArrayEntry(tag,
                                                         ts->group_,
                                                         ts->newGroup_));
    }

    TiffComponent::AutoPtr newTiffArrayElement(uint16_t tag,
                                               const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffArrayElement(tag,
                                                           ts->group_));
    }

    TiffComponent::AutoPtr newTiffThumbData(uint16_t tag,
                                            const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffDataEntry(tag, 
                                                        ts->group_,
                                                        0x0202,
                                                        Group::ifd1));
    }

    TiffComponent::AutoPtr newTiffThumbSize(uint16_t tag,
                                            const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffSizeEntry(tag, 
                                                        ts->group_,
                                                        0x0201,
                                                        Group::ifd1));
    }

}                                       // namespace Exiv2
