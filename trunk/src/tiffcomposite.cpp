// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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

#include "tiffcomposite_int.hpp"
#include "tiffvisitor_int.hpp"
#include "makernote2_int.hpp"
#include "value.hpp"
#include "error.hpp"

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    //! Structure for group and group name info
    struct TiffGroupInfo {
        //! Comparison operator for group (id)
        bool operator==(const uint16_t& group) const;
        //! Comparison operator for group name
        bool operator==(const std::string& groupName) const;

        uint16_t group_;   //!< group
        const char* name_; //!< group name
    };

    // Todo: This mapping table probably belongs somewhere else - move it
    //! List of groups and their names
    extern const TiffGroupInfo tiffGroupInfo[] = {
        {   1, "Image"        },
        {   2, "Thumbnail"    },
        {   3, "Photo"        },
        {   4, "GPSInfo"      },
        {   5, "Iop"          },
        {   6, "SubImage1"    },
        {   7, "SubImage2"    },
        {   8, "SubImage3"    },
        {   9, "SubImage4"    },
        { 257, "Olympus"      },
        { 258, "Fujifilm"     },
        { 259, "Canon"        },
        { 260, "CanonCs"      },
        { 261, "CanonSi"      },
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
        { 276, "MinoltaCs7D"  },
        { 277, "CanonPi"      },
        { 278, "CanonPa"      },
        { 279, "Pentax"       },
        { 280, "NikonPreview" }
    };

    bool TiffGroupInfo::operator==(const uint16_t& group) const
    {
        return group_ == group;
    }

    bool TiffGroupInfo::operator==(const std::string& groupName) const
    {
        std::string name(name_);
        return name == groupName;
    }

    const char* tiffGroupName(uint16_t group)
    {
        const TiffGroupInfo* gi = find(tiffGroupInfo, group);
        if (!gi) return "Unknown";
        return gi->name_;
    }

    uint16_t tiffGroupId(const std::string& groupName)
    {
        const TiffGroupInfo* gi = find(tiffGroupInfo, groupName);
        if (!gi) return 0;
        return gi->group_;
    }

    bool TiffStructure::operator==(const TiffStructure::Key& key) const
    {
        return    (Tag::all == extendedTag_ || key.e_ == extendedTag_)
               && key.g_ == group_;
    }

    bool TiffMappingInfo::operator==(const TiffMappingInfo::Key& key) const
    {
        std::string make(make_);
        return    ("*" == make || make == key.m_.substr(0, make.length()))
               && (Tag::all == extendedTag_ || key.e_ == extendedTag_)
               && key.g_ == group_;
    }

    TiffComponent::TiffComponent(uint16_t tag, uint16_t group)
        : tag_(tag), group_(group), pStart_(0)
    {
    }

    TiffEntryBase::TiffEntryBase(uint16_t tag, uint16_t group, TiffType tiffType)
        : TiffComponent(tag, group),
          tiffType_(tiffType), count_(0), offset_(0),
          size_(0), pData_(0), isMalloced_(false),
          pValue_(0)
    {
    }

    TiffSubIfd::TiffSubIfd(uint16_t tag, uint16_t group, uint16_t newGroup)
        : TiffEntryBase(tag, group, ttUnsignedLong), newGroup_(newGroup)
    {
    }

    TiffMnEntry::TiffMnEntry(uint16_t tag, uint16_t group, uint16_t mnGroup)
        : TiffEntryBase(tag, group, ttUndefined), mnGroup_(mnGroup), mn_(0)
    {
    }

    TiffArrayEntry::TiffArrayEntry(uint16_t tag,
                                   uint16_t group,
                                   uint16_t elGroup,
                                   TiffType elTiffType,
                                   bool     addSizeElement)
        : TiffEntryBase(tag, group, elTiffType),
          elSize_(static_cast<uint16_t>(TypeInfo::typeSize(toTypeId(elTiffType, 0, elGroup)))),
          elGroup_(elGroup),
          addSizeElement_(addSizeElement)
    {
    }

    TiffDirectory::~TiffDirectory()
    {
        for (Components::iterator i = components_.begin(); i != components_.end(); ++i) {
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
        if (isMalloced_) {
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
        for (Components::iterator i = elements_.begin(); i != elements_.end(); ++i) {
            delete *i;
        }
    } // TiffArrayEntry::~TiffArrayEntry

    void TiffEntryBase::allocData(uint32_t len)
    {
        if (isMalloced_) {
            delete[] pData_;
        }
        pData_ = new byte[len];
        size_ = len;
        isMalloced_ = true;
    } // TiffEntryBase::allocData

    void TiffEntryBase::setData(byte* pData, int32_t size)
    {
        pData_ = pData;
        size_  = size;
        if (pData_ == 0) size_ = 0;
    }

    void TiffEntryBase::updateValue(Value::AutoPtr value, ByteOrder byteOrder)
    {
        if (value.get() == 0) return;
        uint32_t newSize = value->size();
        if (newSize > size_) {
            allocData(newSize);
        }
        memset(pData_, 0x0, size_);
        size_ = value->copy(pData_, byteOrder);
        assert(size_ == newSize);
        setValue(value);
    } // TiffEntryBase::updateValue

    void TiffEntryBase::setValue(Value::AutoPtr value)
    {
        if (value.get() == 0) return;
        tiffType_  = toTiffType(value->typeId());
        count_ = value->count();
        delete pValue_;
        pValue_ = value.release();
    } // TiffEntryBase::setValue

    void TiffDataEntry::setStrips(const Value* pSize,
                                  const byte*  pData,
                                  uint32_t     sizeData,
                                  uint32_t     baseOffset)
    {
        assert(pSize);
        assert(pValue());

        long size = 0;
        for (long i = 0; i < pSize->count(); ++i) {
            size += pSize->toLong(i);
        }
        long offset = pValue()->toLong(0);
        // Todo: Remove limitation of JPEG writer: strips must be contiguous
        // Until then we check: last offset + last size - first offset == size?
        if (  pValue()->toLong(pValue()->count()-1)
            + pSize->toLong(pSize->count()-1)
            - offset != size) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: "
                      << "Directory " << tiffGroupName(group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << tag()
                      << " Data area is not contiguous, ignoring it.\n";
#endif
            return;
        }
        if (baseOffset + offset + size > sizeData) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: "
                      << "Directory " << tiffGroupName(group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << tag()
                      << " Data area exceeds data buffer, ignoring it.\n";
#endif
            return;
        }
        pDataArea_ = const_cast<byte*>(pData) + baseOffset + offset;
        sizeDataArea_ = size;
        const_cast<Value*>(pValue())->setDataArea(pDataArea_, sizeDataArea_);
    } // TiffDataEntry::setStrips

    void TiffImageEntry::setStrips(const Value* pSize,
                                   const byte*  pData,
                                   uint32_t     sizeData,
                                   uint32_t     baseOffset)
    {
        assert(pSize);
        assert(pValue());

        if (pValue()->count() != pSize->count()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: "
                      << "Directory " << tiffGroupName(group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << tag()
                      << ": Size and data offset entries have different"
                      << " number of components, ignoring them.\n";
#endif
            return;
        }
        for (int i = 0; i < pValue()->count(); ++i) {
            const byte* pStrip = pData + baseOffset + pValue()->toLong(i);
            const uint32_t stripSize = static_cast<uint32_t>(pSize->toLong(i));
            if (   stripSize > 0
                && pData + sizeData > pStrip
                && static_cast<uint32_t>(pData + sizeData - pStrip) >= stripSize) {
                strips_.push_back(std::make_pair(pStrip, stripSize));
            }
#ifndef SUPPRESS_WARNINGS
            else {
                std::cerr << "Warning: "
                          << "Directory " << tiffGroupName(group())
                          << ", entry 0x" << std::setw(4)
                          << std::setfill('0') << std::hex << tag()
                          << ": Strip " << std::dec << i
                          << " is outside of the data area; ignored.\n";
            }
#endif
        }
    } // TiffImageEntry::setStrips

    TiffComponent* TiffComponent::addPath(uint16_t tag, TiffPath& tiffPath)
    {
        return doAddPath(tag, tiffPath);
    } // TiffComponent::addPath

    TiffComponent* TiffDirectory::doAddPath(uint16_t tag, TiffPath& tiffPath)
    {
        tiffPath.pop();
        assert(!tiffPath.empty());
        const TiffStructure* ts = tiffPath.top();
        assert(ts != 0);
        // Prevent dangling subIFD tags: Do not add a subIFD tag if it has no child
        if (tiffPath.size() == 1 && ts->newTiffCompFct_ == newTiffSubIfd) return 0;
        TiffComponent* tc = 0;
        // To allow duplicate entries, we only check if the new component already
        // exists if there is still at least one composite tag on the stack
        // Todo: Find a generic way to require subIFDs to be unique tags
        if (tiffPath.size() > 1 || ts->newTiffCompFct_ == newTiffSubIfd) {
            if (ts->extendedTag_ == Tag::next) {
                tc = pNext_;
            }
            else {
                for (Components::iterator i = components_.begin(); i != components_.end(); ++i) {
                    if ((*i)->tag() == ts->tag() && (*i)->group() == ts->group_) {
                        tc = *i;
                        break;
                    }
                }
            }
        }
        if (tc == 0) {
            assert(ts->newTiffCompFct_ != 0);
            uint16_t tg = tiffPath.size() == 1 ? tag : ts->tag();
            TiffComponent::AutoPtr atc(ts->newTiffCompFct_(tg, ts));
            if (ts->extendedTag_ == Tag::next) {
                tc = this->addNext(atc);
            }
            else {
                tc = this->addChild(atc);
            }
        }
        return tc->addPath(tag, tiffPath);
    } // TiffDirectory::doAddPath

    TiffComponent* TiffSubIfd::doAddPath(uint16_t tag, TiffPath& tiffPath)
    {
        const TiffStructure* ts1 = tiffPath.top();
        assert(ts1 != 0);
        tiffPath.pop();
        if (tiffPath.empty()) {
            // If the last element in the path is the sub-IFD tag itself we're done
            return this;
        }
        const TiffStructure* ts2 = tiffPath.top();
        assert(ts2 != 0);
        tiffPath.push(ts1);
        uint16_t dt = ts1->tag();
        TiffComponent* tc = 0;
        for (Ifds::iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            if ((*i)->group() == ts2->group_) {
                tc = *i;
                break;
            }
        }
        if (tc == 0) {
            TiffComponent::AutoPtr atc(new TiffDirectory(dt, ts2->group_));
            tc = addChild(atc);
            setCount(ifds_.size());
        }
        return tc->addPath(tag, tiffPath);
    } // TiffSubIfd::doAddPath

    TiffComponent* TiffMnEntry::doAddPath(uint16_t tag, TiffPath& tiffPath)
    {
        const TiffStructure* ts1 = tiffPath.top();
        assert(ts1 != 0);
        tiffPath.pop();
        if (tiffPath.empty()) {
            // If the last element in the path is the makernote tag itself we're done
            return this;
        }
        const TiffStructure* ts2 = tiffPath.top();
        assert(ts2 != 0);
        tiffPath.push(ts1);
        if (mn_ == 0) {
            mnGroup_ = ts2->group_;
            mn_ = TiffMnCreator::create(ts1->tag(), ts1->group_, mnGroup_);
            assert(mn_);
        }
        return mn_->addPath(tag, tiffPath);
    } // TiffMnEntry::doAddPath

    TiffComponent* TiffArrayEntry::doAddPath(uint16_t tag, TiffPath& tiffPath)
    {
        tiffPath.pop();
        assert(!tiffPath.empty());
        const TiffStructure* ts = tiffPath.top();
        assert(ts != 0);
        TiffComponent* tc = 0;
        // To allow duplicate entries, we only check if the new component already
        // exists if there is still at least one composite tag on the stack
        if (tiffPath.size() > 1) {
            for (Components::iterator i = elements_.begin(); i != elements_.end(); ++i) {
                if ((*i)->tag() == ts->tag() && (*i)->group() == ts->group_) {
                    tc = *i;
                    break;
                }
            }
        }
        if (tc == 0) {
            assert(ts->newTiffCompFct_ != 0);
            uint16_t tg = tiffPath.size() == 1 ? tag : ts->tag();
            TiffComponent::AutoPtr atc(ts->newTiffCompFct_(tg, ts));
            assert(ts->extendedTag_ != Tag::next);
            tc = addChild(atc);
            setCount(elements_.size());
        }
        return tc->addPath(tag, tiffPath);
    } // TiffArrayEntry::doAddPath

    TiffComponent* TiffComponent::addChild(TiffComponent::AutoPtr tiffComponent)
    {
        return doAddChild(tiffComponent);
    } // TiffComponent::addChild

    TiffComponent* TiffDirectory::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = tiffComponent.release();
        components_.push_back(tc);
        return tc;
    } // TiffDirectory::doAddChild

    TiffComponent* TiffSubIfd::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        TiffDirectory* d = dynamic_cast<TiffDirectory*>(tiffComponent.release());
        assert(d);
        ifds_.push_back(d);
        return d;
    } // TiffSubIfd::doAddChild

    TiffComponent* TiffMnEntry::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = 0;
        if (mn_) {
            tc =  mn_->addChild(tiffComponent);
        }
        return tc;
    } // TiffMnEntry::doAddChild

    TiffComponent* TiffArrayEntry::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = tiffComponent.release();
        elements_.push_back(tc);
        return tc;
    } // TiffArrayEntry::doAddChild

    TiffComponent* TiffComponent::addNext(TiffComponent::AutoPtr tiffComponent)
    {
        return doAddNext(tiffComponent);
    } // TiffComponent::addNext

    TiffComponent* TiffDirectory::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = 0;
        if (hasNext_) {
            tc = tiffComponent.release();
            pNext_ = tc;
        }
        return tc;
    } // TiffDirectory::doAddNext

    TiffComponent* TiffMnEntry::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = 0;
        if (mn_) {
            tc = mn_->addNext(tiffComponent);
        }
        return tc;
    } // TiffMnEntry::doAddNext

    void TiffComponent::accept(TiffVisitor& visitor)
    {
        if (visitor.go(TiffVisitor::geTraverse)) doAccept(visitor); // one for NVI :)
    } // TiffComponent::accept

    void TiffEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitEntry(this);
    } // TiffEntry::doAccept

    void TiffDataEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitDataEntry(this);
    } // TiffDataEntry::doAccept

    void TiffImageEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitImageEntry(this);
    } // TiffImageEntry::doAccept

    void TiffSizeEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitSizeEntry(this);
    } // TiffSizeEntry::doAccept

    void TiffDirectory::doAccept(TiffVisitor& visitor)
    {
        visitor.visitDirectory(this);
        for (Components::const_iterator i = components_.begin();
             visitor.go(TiffVisitor::geTraverse) && i != components_.end(); ++i) {
            (*i)->accept(visitor);
        }
        if (visitor.go(TiffVisitor::geTraverse)) visitor.visitDirectoryNext(this);
        if (pNext_) pNext_->accept(visitor);
        if (visitor.go(TiffVisitor::geTraverse)) visitor.visitDirectoryEnd(this);
    } // TiffDirectory::doAccept

    void TiffSubIfd::doAccept(TiffVisitor& visitor)
    {
        visitor.visitSubIfd(this);
        for (Ifds::iterator i = ifds_.begin();
             visitor.go(TiffVisitor::geTraverse) && i != ifds_.end(); ++i) {
            (*i)->accept(visitor);
        }
    } // TiffSubIfd::doAccept

    void TiffMnEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitMnEntry(this);
        if (mn_) mn_->accept(visitor);
        if (!visitor.go(TiffVisitor::geKnownMakernote)) {
            delete mn_;
            mn_ = 0;
        }

    } // TiffMnEntry::doAccept

    void TiffArrayEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitArrayEntry(this);
        for (Components::const_iterator i = elements_.begin();
             visitor.go(TiffVisitor::geTraverse) && i != elements_.end(); ++i) {
            (*i)->accept(visitor);
        }
    } // TiffArrayEntry::doAccept

    void TiffArrayElement::doAccept(TiffVisitor& visitor)
    {
        visitor.visitArrayElement(this);
    } // TiffArrayElement::doAccept

    void TiffEntryBase::encode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        doEncode(encoder, datum);
    } // TiffComponent::encode

    void TiffArrayElement::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeArrayElement(this, datum);
    } // TiffArrayElement::doEncode

    void TiffArrayEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeArrayEntry(this, datum);
    } // TiffArrayEntry::doEncode

    void TiffDataEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeDataEntry(this, datum);
    } // TiffDataEntry::doEncode

    void TiffEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeTiffEntry(this, datum);
    } // TiffEntry::doEncode

    void TiffImageEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeImageEntry(this, datum);
    } // TiffImageEntry::doEncode

    void TiffMnEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeMnEntry(this, datum);
    } // TiffMnEntry::doEncode

    void TiffSizeEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeSizeEntry(this, datum);
    } // TiffSizeEntry::doEncode

    void TiffSubIfd::doEncode(TiffEncoder& encoder, const Exifdatum* datum)
    {
        encoder.encodeSubIfd(this, datum);
    } // TiffSubIfd::doEncode

    uint32_t TiffComponent::count() const
    {
        return doCount();
    }

    uint32_t TiffDirectory::doCount() const
    {
        return components_.size();
    }

    uint32_t TiffEntryBase::doCount() const
    {
        return count_;
    }

    uint32_t TiffMnEntry::doCount() const
    {
        // Count of tag Exif.Photo.MakerNote is the size of the Makernote in bytes
        assert(tiffType() == ttUndefined);
        return size();
    }

    uint32_t TiffArrayEntry::doCount() const
    {
        if (elements_.empty()) return 0;

        uint16_t maxTag = 0;
        for (Components::const_iterator i = elements_.begin(); i != elements_.end(); ++i) {
            uint32_t mt = (*i)->tag();
            if ((*i)->count() > 1) mt += (*i)->count() - 1;
            if (mt > maxTag) maxTag = mt;
        }
        return maxTag + 1;
    }

    uint32_t TiffComponent::write(Blob&     blob,
                                  ByteOrder byteOrder,
                                  int32_t   offset,
                                  uint32_t  valueIdx,
                                  uint32_t  dataIdx,
                                  uint32_t& imageIdx)
    {
        return doWrite(blob, byteOrder, offset, valueIdx, dataIdx, imageIdx);
    } // TiffComponent::write

    uint32_t TiffDirectory::doWrite(Blob&     blob,
                                    ByteOrder byteOrder,
                                    int32_t   offset,
                                    uint32_t  valueIdx,
                                    uint32_t  dataIdx,
                                    uint32_t& imageIdx)
    {
        bool isRootDir = (imageIdx == uint32_t(-1));

        // Number of components to write
        const uint32_t compCount = count();
        if (compCount > 0xffff) throw Error(49, tiffGroupName(group()));

        // Size of next IFD, if any
        uint32_t sizeNext = 0;
        if (pNext_) sizeNext = pNext_->size();

        // Nothing to do if there are no entries and the size of the next IFD is 0
        if (compCount == 0 && sizeNext == 0) return 0;

        // Size of all directory entries, without values and additional data
        const uint32_t sizeDir = 2 + 12 * compCount + (hasNext_ ? 4 : 0);

        // TIFF standard requires IFD entries to be sorted in ascending order by tag
        std::sort(components_.begin(), components_.end(), cmpTagLt);

        // Size of IFD values and additional data
        uint32_t sizeValue = 0;
        uint32_t sizeData = 0;
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            uint32_t sv = (*i)->size();
            if (sv > 4) {
                sv += sv & 1;               // Align value to word boundary
                sizeValue += sv;
            }
            // Also add the size of data, but only if needed
            if (isRootDir) {
                uint32_t sd = (*i)->sizeData();
                sd += sd & 1;               // Align data to word boundary
                sizeData += sd;
            }
        }

        uint32_t idx = 0;                   // Current IFD index / bytes written
        valueIdx = sizeDir;                 // Offset to the current IFD value
        dataIdx  = sizeDir + sizeValue;     // Offset to the entry's data area
        if (isRootDir) {                    // Absolute offset to the image data
            imageIdx = offset + dataIdx + sizeData + sizeNext;
            imageIdx += imageIdx & 1;       // Align image data to word boundary
        }

        // 1st: Write the IFD, a) Number of directory entries
        byte buf[4];
        us2Data(buf, static_cast<uint16_t>(compCount), byteOrder);
        append(blob, buf, 2);
        idx += 2;
        // b) Directory entries - may contain pointers to the value or data
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            idx += writeDirEntry(blob, byteOrder, offset, *i, valueIdx, dataIdx, imageIdx);
            uint32_t sv = (*i)->size();
            if (sv > 4) {
                sv += sv & 1;               // Align value to word boundary
                valueIdx += sv;
            }
            uint32_t sd = (*i)->sizeData();
            sd += sd & 1;                   // Align data to word boundary
            dataIdx += sd;
        }
        // c) Pointer to the next IFD
        if (hasNext_) {
            memset(buf, 0x0, 4);
            if (pNext_ && sizeNext) {
                l2Data(buf, offset + dataIdx, byteOrder);
            }
            append(blob, buf, 4);
            idx += 4;
        }
        assert(idx == sizeDir);

        // 2nd: Write IFD values - may contain pointers to additional data
        valueIdx = sizeDir;
        dataIdx = sizeDir + sizeValue;
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            uint32_t sv = (*i)->size();
            if (sv > 4) {
                uint32_t d = (*i)->write(blob, byteOrder, offset, valueIdx, dataIdx, imageIdx);
                assert(sv == d);
                if ((sv & 1) == 1) {
                    blob.push_back(0x0);    // Align value to word boundary
                    sv += 1;
                }
                idx += sv;
                valueIdx += sv;
            }
            uint32_t sd = (*i)->sizeData();
            sd += sd & 1;                   // Align data to word boundary
            dataIdx += sd;
        }
        assert(idx == sizeDir + sizeValue);

        // 3rd: Write data - may contain offsets too (eg sub-IFD)
        dataIdx = sizeDir + sizeValue;
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            uint32_t sd = (*i)->writeData(blob, byteOrder, offset, dataIdx, imageIdx);
            assert((*i)->sizeData() == sd);
            if ((sd & 1) == 1) {
                blob.push_back(0x0);        // Align data to word boundary
                sd += 1;
            }
            idx += sd;
            dataIdx += sd;
        }
        // No assertion (sizeData may not be available, see above)
        // assert(idx == sizeDir + sizeValue + sizeData);

        // 4th: Write next-IFD
        if (pNext_ && sizeNext) {
            idx += pNext_->write(blob, byteOrder, offset + idx, uint32_t(-1), uint32_t(-1), imageIdx);
        }

        // 5th, at the root directory level only: write image data
        if (isRootDir) {
            idx += writeImage(blob, byteOrder);
        }

        return idx;
    } // TiffDirectory::doWrite

    uint32_t TiffDirectory::writeDirEntry(Blob&          blob,
                                          ByteOrder      byteOrder,
                                          int32_t        offset,
                                          TiffComponent* pTiffComponent,
                                          uint32_t       valueIdx,
                                          uint32_t       dataIdx,
                                          uint32_t&      imageIdx) const
    {
        assert(pTiffComponent);
        TiffEntryBase* pDirEntry = dynamic_cast<TiffEntryBase*>(pTiffComponent);
        assert(pDirEntry);
        byte buf[8];
        us2Data(buf,     pDirEntry->tag(),      byteOrder);
        us2Data(buf + 2, pDirEntry->tiffType(), byteOrder);
        ul2Data(buf + 4, pDirEntry->count(),    byteOrder);
        append(blob, buf, 8);
        if (pDirEntry->size() > 4) {
            pDirEntry->setOffset(offset + static_cast<int32_t>(valueIdx));
            l2Data(buf, pDirEntry->offset(), byteOrder);
            append(blob, buf, 4);
        }
        else {
            const uint32_t len = pDirEntry->write(blob,
                                                  byteOrder,
                                                  offset,
                                                  valueIdx,
                                                  dataIdx,
                                                  imageIdx);
            assert(len <= 4);
            if (len < 4) {
                memset(buf, 0x0, 4);
                append(blob, buf, 4 - len);
            }
        }
        return 12;
    } // TiffDirectory::writeDirEntry

    uint32_t TiffEntryBase::doWrite(Blob&     blob,
                                    ByteOrder byteOrder,
                                    int32_t   /*offset*/,
                                    uint32_t  /*valueIdx*/,
                                    uint32_t  /*dataIdx*/,
                                    uint32_t& /*imageIdx*/)
    {
        if (!pValue_) return 0;

        DataBuf buf(pValue_->size());
        pValue_->copy(buf.pData_, byteOrder);
        append(blob, buf.pData_, buf.size_);
        return buf.size_;
    } // TiffEntryBase::doWrite

    uint32_t TiffEntryBase::writeOffset(byte*     buf,
                                        int32_t   offset,
                                        TiffType  tiffType,
                                        ByteOrder byteOrder)
    {
        uint32_t rc = 0;
        switch(tiffType) {
        case ttUnsignedShort:
        case ttSignedShort:
            if (static_cast<uint32_t>(offset) > 0xffff) throw Error(26);
            rc = s2Data(buf, static_cast<int16_t>(offset), byteOrder);
            break;
        case ttUnsignedLong:
        case ttSignedLong:
            rc = l2Data(buf, static_cast<int32_t>(offset), byteOrder);
            break;
        default:
            throw Error(27);
            break;
        }
        return rc;
    } // TiffEntryBase::writeOffset

    uint32_t TiffDataEntry::doWrite(Blob&     blob,
                                    ByteOrder byteOrder,
                                    int32_t   offset,
                                    uint32_t  /*valueIdx*/,
                                    uint32_t  dataIdx,
                                    uint32_t& /*imageIdx*/)
    {
        if (!pValue()) return 0;

        DataBuf buf(pValue()->size());
        uint32_t idx = 0;
        const long prevOffset = pValue()->toLong(0);
        for (uint32_t i = 0; i < count(); ++i) {
            const long newDataIdx =   pValue()->toLong(i) - prevOffset
                                    + static_cast<long>(dataIdx);
            idx += writeOffset(buf.pData_ + idx,
                               offset + newDataIdx,
                               tiffType(),
                               byteOrder);
        }
        append(blob, buf.pData_, buf.size_);
        return buf.size_;
    } // TiffDataEntry::doWrite

    uint32_t TiffImageEntry::doWrite(Blob&     blob,
                                     ByteOrder byteOrder,
                                     int32_t   /*offset*/,
                                     uint32_t  /*valueIdx*/,
                                     uint32_t  /*dataIdx*/,
                                     uint32_t& imageIdx)
    {
#ifdef DEBUG
        std::cerr << "TiffImageEntry, tag 0x" << std::setw(4) 
                  << std::setfill('0') << std::hex << tag() << std::dec
                  << ": Writing offset " << imageIdx << "\n";
#endif
        DataBuf buf(strips_.size() * 4);
        uint32_t idx = 0;
        for (Strips::const_iterator i = strips_.begin(); i != strips_.end(); ++i) {
            idx += writeOffset(buf.pData_ + idx, imageIdx, tiffType(), byteOrder);
            imageIdx += i->second;
        }
        imageIdx += sizeImage() & 1;                // Align image data to word boundary
        append(blob, buf.pData_, buf.size_);
        return buf.size_;
    } // TiffImageEntry::doWrite

    uint32_t TiffSubIfd::doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  /*valueIdx*/,
                                 uint32_t  dataIdx,
                                 uint32_t& /*imageIdx*/)
    {
        DataBuf buf(ifds_.size() * 4);
        uint32_t idx = 0;
        for (Ifds::const_iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            idx += writeOffset(buf.pData_ + idx, offset + dataIdx, tiffType(), byteOrder);
            dataIdx += (*i)->size();
        }
        append(blob, buf.pData_, buf.size_);
        return buf.size_;
    } // TiffSubIfd::doWrite

    uint32_t TiffMnEntry::doWrite(Blob&     blob,
                                  ByteOrder byteOrder,
                                  int32_t   offset,
                                  uint32_t  valueIdx,
                                  uint32_t  dataIdx,
                                  uint32_t& imageIdx)
    {
        if (!mn_) {
            return TiffEntryBase::doWrite(blob, byteOrder, offset, valueIdx, dataIdx, imageIdx);
        }
        return mn_->write(blob, byteOrder, offset + valueIdx, uint32_t(-1), uint32_t(-1), imageIdx);
    } // TiffMnEntry::doWrite

    uint32_t TiffArrayEntry::doWrite(Blob&     blob,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  valueIdx,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx)
    {
        const uint32_t cnt = count();
        if (cnt == 0) return 0;

        uint32_t idx = 0;
        int32_t nextTag = 0;

        // Some array entries need to have the size in the first element
        if (addSizeElement_) {
            byte buf[4];
            switch (elSize_) {
            case 2:
                idx += us2Data(buf, size(), byteOrder);
                break;
            case 4:
                idx += ul2Data(buf, size(), byteOrder);
                break;
            default:
                assert(false);
            }
            append(blob, buf, elSize_);
            nextTag = 1;
        }

        // Tags must be sorted in ascending order
        std::sort(elements_.begin(), elements_.end(), cmpTagLt);
        uint32_t seq = 0;
        for (Components::const_iterator i = elements_.begin(); i != elements_.end(); ++i) {
            // Skip deleted entries at the end of the array
            if (seq++ > cnt) break;
            // Skip the manufactured tag, if it exists
            if (addSizeElement_ && (*i)->tag() == 0x0000) continue;
            // Fill gaps. Repeated tags will cause an exception
            int32_t gap = ((*i)->tag() - nextTag) * elSize_;
            if (gap < 0) throw Error(50, (*i)->tag());
            if (gap > 0) {
                blob.insert(blob.end(), gap, 0);
                idx += gap;
            }
            idx += (*i)->write(blob, byteOrder, offset + idx, valueIdx, dataIdx, imageIdx);
            nextTag = (*i)->tag() + 1;
            if ((*i)->count() > 1) nextTag += (*i)->count() - 1;
        }
        return idx;
    } // TiffArrayEntry::doWrite

    uint32_t TiffArrayElement::doWrite(Blob&     blob,
                                       ByteOrder byteOrder,
                                       int32_t   /*offset*/,
                                       uint32_t  /*valueIdx*/,
                                       uint32_t  /*dataIdx*/,
                                       uint32_t& /*imageIdx*/)
    {
        Value const* pv = pValue();
        if (!pv || pv->count() == 0) return 0;
        if (toTiffType(pv->typeId()) != elTiffType_) {
            throw Error(51, tag());
        }
        DataBuf buf(pv->size());
        if (elByteOrder_ != invalidByteOrder) byteOrder = elByteOrder_;
        pv->copy(buf.pData_, byteOrder);
        append(blob, buf.pData_, buf.size_);
        return buf.size_;
    } // TiffArrayElement::doWrite

    uint32_t TiffComponent::writeData(Blob&     blob,
                                      ByteOrder byteOrder,
                                      int32_t   offset,
                                      uint32_t  dataIdx,
                                      uint32_t& imageIdx) const
    {
        return doWriteData(blob, byteOrder, offset, dataIdx, imageIdx);
    } // TiffComponent::writeData

    uint32_t TiffDirectory::doWriteData(Blob&     /*blob*/,
                                        ByteOrder /*byteOrder*/,
                                        int32_t   /*offset*/,
                                        uint32_t  /*dataIdx*/,
                                        uint32_t& /*imageIdx*/) const
    {
        // We don't expect this method to be called. This makes it obvious.
        assert(false);
        return 0;
    } // TiffDirectory::doWriteData

    uint32_t TiffEntryBase::doWriteData(Blob&     /*blob*/,
                                        ByteOrder /*byteOrder*/,
                                        int32_t   /*offset*/,
                                        uint32_t  /*dataIdx*/,
                                        uint32_t& /*imageIdx*/) const
    {
        return 0;
    } // TiffEntryBase::doWriteData

    uint32_t TiffDataEntry::doWriteData(Blob&     blob,
                                        ByteOrder /*byteOrder*/,
                                        int32_t   /*offset*/,
                                        uint32_t  /*dataIdx*/,
                                        uint32_t& /*imageIdx*/) const
    {
        if (!pValue()) return 0;

        DataBuf buf = pValue()->dataArea();
        append(blob, buf.pData_, buf.size_);
        return buf.size_;
    } // TiffDataEntry::doWriteData

    uint32_t TiffSubIfd::doWriteData(Blob&     blob,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const
    {
        uint32_t len = 0;
        for (Ifds::const_iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            len  += (*i)->write(blob, byteOrder, offset + dataIdx + len, uint32_t(-1), uint32_t(-1), imageIdx);
        }
        return len;
    } // TiffSubIfd::doWriteData

    uint32_t TiffComponent::writeImage(Blob&     blob,
                                       ByteOrder byteOrder) const
    {
        return doWriteImage(blob, byteOrder);
    } // TiffComponent::writeImage

    uint32_t TiffDirectory::doWriteImage(Blob&     blob,
                                         ByteOrder byteOrder) const
    {
        uint32_t len = 0;
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            len += (*i)->writeImage(blob, byteOrder);
        }
        if (pNext_) {
            len += pNext_->writeImage(blob, byteOrder);
        }
        return len;
    } // TiffDirectory::doWriteImage

    uint32_t TiffEntryBase::doWriteImage(Blob&     /*blob*/,
                                         ByteOrder /*byteOrder*/) const
    {
        return 0;
    } // TiffEntryBase::doWriteImage

    uint32_t TiffSubIfd::doWriteImage(Blob&     blob,
                                      ByteOrder byteOrder) const
    {
        uint32_t len = 0;
        for (Ifds::const_iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            len  += (*i)->writeImage(blob, byteOrder);
        }
        return len;
    } // TiffSubIfd::doWriteData

    uint32_t TiffImageEntry::doWriteImage(Blob&     blob,
                                          ByteOrder /*byteOrder*/) const
    {
        // Adjust blob capacity - speeds up copying of large data areas
        uint32_t sz = sizeImage();
        Blob::size_type size = blob.size();
        if (blob.capacity() - size < sz) {
            blob.reserve(size + sz + 65536);
        }
        // Align image data to word boundary
        uint32_t align = 0;
        if ((blob.size() & 1) == 1) {
            blob.push_back(0x0);
            align = 1;
        }

        uint32_t len = pValue()->sizeDataArea();
        if (len > 0) {
#ifdef DEBUG
            std::cerr << "TiffImageEntry, tag 0x" << std::setw(4) 
                      << std::setfill('0') << std::hex << tag() << std::dec
                      << ": Writing data area, blob-size = " << blob.size();
#endif
            DataBuf buf = pValue()->dataArea();
            append(blob, buf.pData_, buf.size_);
        }
        else {
#ifdef DEBUG
            std::cerr << "TiffImageEntry, tag 0x" << std::setw(4) 
                      << std::setfill('0') << std::hex << tag() << std::dec
                      << ": Writing data area, blob-size = " << blob.size();
#endif
            len = 0;
            for (Strips::const_iterator i = strips_.begin(); i != strips_.end(); ++i) {
                append(blob, i->first, i->second);
                len += i->second;
            }
        }
#ifdef DEBUG
        std::cerr << ", len = " << len << " bytes\n";
#endif
        return len + align;
    } // TiffImageEntry::doWriteImage

    uint32_t TiffComponent::size() const
    {
        return doSize();
    } // TiffComponent::size

    uint32_t TiffDirectory::doSize() const
    {
        uint32_t compCount = count();
        // Size of the directory, without values and additional data
        uint32_t len = 2 + 12 * compCount + (hasNext_ ? 4 : 0);
        // Size of IFD values and data
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            uint32_t sv = (*i)->size();
            if (sv > 4) {
                sv += sv & 1;               // Align value to word boundary
                len += sv;
            }
            uint32_t sd = (*i)->sizeData();
            sd += sd & 1;                   // Align data to word boundary
            len += sd;
        }
        // Size of next-IFD, if any
        uint32_t sizeNext = 0;
        if (pNext_) {
            sizeNext = pNext_->size();
            len += sizeNext;
        }
        // Reset size of IFD if it has no entries and no or empty next IFD.
        if (compCount == 0 && sizeNext == 0) len = 0;
        return len;
    } // TiffDirectory::doSize

    uint32_t TiffEntryBase::doSize() const
    {
        return size_;
    } // TiffEntryBase::doSize

    uint32_t TiffImageEntry::doSize() const
    {
        return strips_.size() * 4;
    } // TiffImageEntry::doSize

    uint32_t TiffSubIfd::doSize() const
    {
        return ifds_.size() * 4;
    } // TiffSubIfd::doSize

    uint32_t TiffMnEntry::doSize() const
    {
        if (!mn_) {
            return TiffEntryBase::doSize();
        }
        return mn_->size();
    } // TiffMnEntry::doSize

    uint32_t TiffArrayEntry::doSize() const
    {
        return count() * elSize_;
    } // TiffArrayEntry::doSize

    uint32_t TiffComponent::sizeData() const
    {
        return doSizeData();
    } // TiffComponent::sizeData

    uint32_t TiffDirectory::doSizeData() const
    {
        assert(false);
        return 0;
    } // TiffDirectory::doSizeData

    uint32_t TiffEntryBase::doSizeData() const
    {
        return 0;
    } // TiffEntryBase::doSizeData

    uint32_t TiffDataEntry::doSizeData() const
    {
        if (!pValue()) return 0;
        return pValue()->sizeDataArea();
    } // TiffDataEntry::doSizeData

    uint32_t TiffSubIfd::doSizeData() const
    {
        uint32_t len = 0;
        for (Ifds::const_iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            len += (*i)->size();
        }
        return len;
    } // TiffSubIfd::doSizeData

    uint32_t TiffComponent::sizeImage() const
    {
        return doSizeImage();
    } // TiffComponent::sizeImage

    uint32_t TiffDirectory::doSizeImage() const
    {
        uint32_t len = 0;
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            len += (*i)->sizeImage();
        }
        if (pNext_) {
            len += pNext_->sizeImage();
        }
        return len;
    } // TiffDirectory::doSizeImage

    uint32_t TiffSubIfd::doSizeImage() const
    {
        uint32_t len = 0;
        for (Ifds::const_iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            len += (*i)->sizeImage();
        }
        return len;
    } // TiffSubIfd::doSizeImage

    uint32_t TiffEntryBase::doSizeImage() const
    {
        return 0;
    } // TiffEntryBase::doSizeImage

    uint32_t TiffImageEntry::doSizeImage() const
    {
        uint32_t len = pValue()->sizeDataArea();
        if (len == 0) {
            for (Strips::const_iterator i = strips_.begin(); i != strips_.end(); ++i) {
                len += i->second;
            }
        }
        return len;
    } // TiffImageEntry::doSizeImage

    // *************************************************************************
    // free functions

    TypeId toTypeId(TiffType tiffType, uint16_t tag, uint16_t group)
    {
        TypeId ti = TypeId(tiffType);
        // On the fly type conversion for Exif.Photo.UserComment        
        if (tag == 0x9286 && group == Group::exif && ti == undefined) {
            ti = comment;
        }
        return ti;
    }

    TiffType toTiffType(TypeId typeId)
    {
        if (static_cast<uint32_t>(typeId) > 0xffff) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: '" << TypeInfo::typeName(typeId)
                      << "' is not a valid Exif (TIFF) type; using type '"
                      << TypeInfo::typeName(undefined) << "'.\n";
#endif
            return undefined;
        }
        return static_cast<uint16_t>(typeId);
    }

    bool cmpTagLt(TiffComponent const* lhs, TiffComponent const* rhs)
    {
        assert(lhs != 0);
        assert(rhs != 0);
        return lhs->tag() < rhs->tag();
    }

    TiffComponent::AutoPtr newTiffDirectory(uint16_t tag,
                                            const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffDirectory(tag, ts->newGroup_));
    }

    TiffComponent::AutoPtr newTiffEntry(uint16_t tag,
                                        const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffEntry(tag, ts->newGroup_));
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

}}                                      // namespace Internal, Exiv2
