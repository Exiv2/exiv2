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
EXIV2_RCSID("@(#) $Id$")

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

    //! Structure for group and group name info
    struct TiffGroupInfo {
        //! comparison operator
        bool operator==(uint16_t group) const;

        uint16_t group_;   //!< group
        const char* name_; //!< group name
    };

    // Todo: This mapping table probably still belongs somewhere else
    //! List of groups and their names
    extern const TiffGroupInfo tiffGroupInfo[] = {
        {   1, "Image"        },
        {   2, "Thumbnail"    },
        {   3, "Photo"        },
        {   4, "GPSInfo"      },
        {   5, "Iop"          },
        {   6, "ImageSubIfd0" },
        {   7, "ImageSubIfd1" },
        {   8, "ImageSubIfd2" },
        {   9, "ImageSubIfd3" },
        { 257, "Olympus"      },
        { 258, "Fujifilm"     },
        { 259, "Canon"        },
        { 260, "CanonCs"     },
        { 261, "CanonSi"     },
        { 262, "CanonCf"      },
        // 263 not needed (nikonmn)
        { 264, "Nikon1"       },
        { 265, "Nikon2"       },
        { 266, "Nikon3"       },
        { 267, "Panasonic"    },
        { 268, "Sigma"        },
        // 269 not needed (sonymn)
        { 270, "Sony"         },
        { 271, "Sony"         },
        { 272, "Minolta"      },
        { 273, "MinoltaCsOld" },
        { 274, "MinoltaCsNew" },
        { 275, "MinoltaCs5D"  },
        { 276, "MinoltaCs7D"  }
    };

    bool TiffGroupInfo::operator==(uint16_t group) const
    {
        return group_ == group;
    }

    const char* tiffGroupName(uint16_t group)
    {
        const TiffGroupInfo* gi = find(tiffGroupInfo, group);
        if (!gi) return "Unknown";
        return gi->name_;
    }

    bool TiffStructure::operator==(const TiffStructure::Key& key) const
    {
        return    (Tag::all == extendedTag_ || key.e_ == extendedTag_)
               && key.g_ == group_;
    }

    bool TiffDecoderInfo::operator==(const TiffDecoderInfo::Key& key) const
    {
        std::string make(make_);
        return    ("*" == make || make == key.m_.substr(0, make.length()))
               && (Tag::all == extendedTag_ || key.e_ == extendedTag_)
               && key.g_ == group_;
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

    TiffSubIfd::~TiffSubIfd()
    {
        for (Ifds::iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            delete *i;
        }
    } // TiffSubIfd::~TiffSubIfd

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

    // Possibly this function shouldn't be in this class...
    std::string TiffComponent::groupName() const
    {
        return tiffGroupName(group_);
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
        TiffDirectory* d = dynamic_cast<TiffDirectory*>(tiffComponent.release());
        assert(d);
        ifds_.push_back(d);
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
        for (Ifds::iterator i = ifds_.begin(); visitor.go() && i != ifds_.end(); ++i) {
            (*i)->accept(visitor);
        }
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

}                                       // namespace Exiv2
