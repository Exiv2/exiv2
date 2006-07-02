// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
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
  File:      ifd.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "ifd.hpp"
#include "types.hpp"
#include "error.hpp"
#include "tags.hpp"                             // for ExifTags::ifdName

// + standard includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    Entry::Entry(bool alloc)
        : alloc_(alloc), ifdId_(ifdIdNotSet), idx_(0),
          tag_(0), type_(0), count_(0), offset_(0), size_(0), pData_(0),
          sizeDataArea_(0), pDataArea_(0), byteOrder_(invalidByteOrder)
    {
    }

    Entry::~Entry()
    {
        if (alloc_) {
            delete[] pData_;
            delete[] pDataArea_;
        }
    }

    Entry::Entry(const Entry& rhs)
        : alloc_(rhs.alloc_), ifdId_(rhs.ifdId_), idx_(rhs.idx_),
          tag_(rhs.tag_), type_(rhs.type_),
          count_(rhs.count_), offset_(rhs.offset_), size_(rhs.size_), pData_(0),
          sizeDataArea_(rhs.sizeDataArea_), pDataArea_(0), byteOrder_(rhs.byteOrder_)
    {
        if (alloc_) {
            if (rhs.pData_) {
                pData_ = new byte[rhs.size()];
                memcpy(pData_, rhs.pData_, rhs.size());
            }
            if (rhs.pDataArea_) {
                pDataArea_ = new byte[rhs.sizeDataArea()];
                memcpy(pDataArea_, rhs.pDataArea_, rhs.sizeDataArea());
            }
        }
        else {
            pData_ = rhs.pData_;
            pDataArea_ = rhs.pDataArea_;
        }
    }

    Entry& Entry::operator=(const Entry& rhs)
    {
        if (this == &rhs) return *this;
        alloc_ = rhs.alloc_;
        ifdId_ = rhs.ifdId_;
        idx_ = rhs.idx_;
        tag_ = rhs.tag_;
        type_ = rhs.type_;
        count_ = rhs.count_;
        offset_ = rhs.offset_;
        size_ = rhs.size_;
        sizeDataArea_ = rhs.sizeDataArea_;
        byteOrder_ = rhs.byteOrder_;
        if (alloc_) {
            delete[] pData_;
            pData_ = 0;
            if (rhs.pData_) {
                pData_ = new byte[rhs.size()];
                memcpy(pData_, rhs.pData_, rhs.size());
            }
            delete[] pDataArea_;
            pDataArea_ = 0;
            if (rhs.pDataArea_) {
                pDataArea_ = new byte[rhs.sizeDataArea()];
                memcpy(pDataArea_, rhs.pDataArea_, rhs.sizeDataArea());
            }
        }
        else {
            pData_ = rhs.pData_;
            pDataArea_ = rhs.pDataArea_;
        }
        return *this;
    } // Entry::operator=

    void Entry::setValue(uint32_t data, ByteOrder byteOrder)
    {
        if (pData_ == 0 || size_ < 4) {
            assert(alloc_);
            size_ = 4;
            delete[] pData_;
            pData_ = new byte[size_];
        }
        ul2Data(pData_, data, byteOrder);
        // do not change size_
        type_ = unsignedLong;
        count_ = 1;
    }

    void Entry::setValue(uint16_t type, uint32_t count, const byte* buf, long len, ByteOrder byteOrder)
    {
        byteOrder_ = byteOrder;
        long dataSize = count * TypeInfo::typeSize(TypeId(type));
        // No minimum size requirement, but make sure the buffer can hold the data
        if (len < dataSize) throw Error(24, tag(), dataSize, len);
        if (alloc_) {
            delete[] pData_;
            pData_ = new byte[len];
            memset(pData_, 0x0, len);
            memcpy(pData_, buf, dataSize);
            size_ = len;
        }
        else {
            if (size_ == 0) {
                // Set the data pointer of a virgin entry
                pData_ = const_cast<byte*>(buf);
                size_ = len;
            }
            else {
                // Overwrite existing data if it fits into the buffer
                if (size_ < dataSize) throw Error(24, tag(), dataSize, size_);
                memset(pData_, 0x0, size_);
                memcpy(pData_, buf, dataSize);
                // do not change size_
            }
        }
        type_ = type;
        count_ = count;
    } // Entry::setValue

    void Entry::setDataArea(const byte* buf, long len)
    {
        if (alloc_) {
            delete[] pDataArea_;
            pDataArea_ = new byte[len];
            memcpy(pDataArea_, buf, len);
            sizeDataArea_ = len;
        }
        else {
            if (sizeDataArea_ == 0) {
                // Set the data area pointer of a virgin entry
                pDataArea_ = const_cast<byte*>(buf);
                sizeDataArea_ = len;
            }
            else {
                // Overwrite existing data if it fits into the buffer
                if (sizeDataArea_ < len) {
                    throw Error(25, tag(), sizeDataArea_, len);
                }
                memset(pDataArea_, 0x0, sizeDataArea_);
                memcpy(pDataArea_, buf, len);
                // do not change sizeDataArea_
            }
        }
    } // Entry::setDataArea

    void Entry::setDataAreaOffsets(uint32_t offset, ByteOrder byteOrder)
    {
        // Hack: Do not require offsets to start from 0, except for rationals
        uint16_t  fusOffset = 0;
        uint32_t  fulOffset = 0;
        int16_t   fsOffset  = 0;
        int32_t   flOffset  = 0;

        for (uint32_t i = 0; i < count(); ++i) {
            byte* buf = pData_ + i * typeSize();
            switch(TypeId(type())) {
            case unsignedShort: {
                if (i == 0) fusOffset = getUShort(buf, byteOrder);
                uint16_t d = getUShort(buf, byteOrder) - fusOffset;
                if (d + offset > 0xffff) throw Error(26);
                us2Data(buf, d + static_cast<uint16_t>(offset), byteOrder);
                break;
            }
            case unsignedLong: {
                if (i == 0) fulOffset = getULong(buf, byteOrder);
                ul2Data(buf, getULong(buf, byteOrder) - fulOffset + offset, byteOrder);
                break;
            }
            case unsignedRational: {
                URational d = getURational(buf, byteOrder);
                d.first = d.first + offset * d.second;
                ur2Data(buf, d, byteOrder);
                break;
            }
            case signedShort: {
                if (i == 0) fsOffset = getShort(buf, byteOrder);
                int16_t d = getShort(buf, byteOrder) - fsOffset;
                if (d + static_cast<int32_t>(offset) > 0xffff) throw Error(26);
                s2Data(buf, d + static_cast<int16_t>(offset), byteOrder);
                break;
            }
            case signedLong: {
                if (i == 0) flOffset = getLong(buf, byteOrder);
                int32_t d = getLong(buf, byteOrder) - flOffset;
                l2Data(buf, d + static_cast<int32_t>(offset), byteOrder);
                break;
            }
            case signedRational: {
                Rational d = getRational(buf, byteOrder);
                d.first = d.first + static_cast<int32_t>(offset) * d.second;
                r2Data(buf, d, byteOrder);
                break;
            }
            default:
                throw Error(27);
                break;
            }
        }
    } // Entry::setDataAreaOffsets

    void Entry::updateBase(byte* pOldBase, byte* pNewBase)
    {
        if (!alloc_) {
            if (pDataArea_) {
                pDataArea_ = pDataArea_ - pOldBase + pNewBase;
            }
            if (pData_) {
                pData_ = pData_ - pOldBase + pNewBase;
            }
        }
    } // Entry::updateBase

    const byte* Entry::component(uint32_t n) const
    {
        if (n >= count()) return 0;
        return data() + n * typeSize();
    } // Entry::component

    Ifd::Ifd(IfdId ifdId)
        : alloc_(true), ifdId_(ifdId), pBase_(0), offset_(0),
          dataOffset_(0), hasNext_(true), pNext_(0), next_(0)
    {
        pNext_ = new byte[4];
        memset(pNext_, 0x0, 4);
    }

    Ifd::Ifd(IfdId ifdId, long offset)
        : alloc_(true), ifdId_(ifdId), pBase_(0), offset_(offset),
          dataOffset_(0), hasNext_(true), pNext_(0), next_(0)
    {
        pNext_ = new byte[4];
        memset(pNext_, 0x0, 4);
    }

    Ifd::Ifd(IfdId ifdId, long offset, bool alloc, bool hasNext)
        : alloc_(alloc), ifdId_(ifdId), pBase_(0), offset_(offset),
          dataOffset_(0), hasNext_(hasNext), pNext_(0), next_(0)
    {
        if (alloc_ && hasNext_) {
            pNext_ = new byte[4];
            memset(pNext_, 0x0, 4);
        }
    }

    Ifd::~Ifd()
    {
        // do not delete pBase_
        if (alloc_ && hasNext_) delete[] pNext_;
    }

    Ifd::Ifd(const Ifd& rhs)
        : alloc_(rhs.alloc_), entries_(rhs.entries_), ifdId_(rhs.ifdId_),
          pBase_(rhs.pBase_), offset_(rhs.offset_), dataOffset_(rhs.dataOffset_),
          hasNext_(rhs.hasNext_), pNext_(rhs.pNext_), next_(rhs.next_)
    {
        if (alloc_ && hasNext_) {
            pNext_ = new byte[4];
            memset(pNext_, 0x0, 4);
            if (rhs.pNext_) memcpy(pNext_, rhs.pNext_, 4);
        }
    }

    int Ifd::read(const byte* buf,
                  long len,
                  long start,
                  ByteOrder byteOrder,
                  long shift)
    {
        int rc = 0;
        long o = start;
        Ifd::PreEntries preEntries;

        if (o < 0 || len < o + 2) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: " << ExifTags::ifdName(ifdId_)
                      << " lies outside of the IFD memory buffer.\n";
#endif
            rc = 6;
        }
        if (rc == 0) {
            offset_ = start - shift;
            int n = getUShort(buf + o, byteOrder);
            o += 2;

            for (int i = 0; i < n; ++i) {
                if (len < o + 12) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Error: " << ExifTags::ifdName(ifdId_)
                              << " entry " << i
                              << " lies outside of the IFD memory buffer.\n";
#endif
                    rc = 6;
                    break;
                }
                Ifd::PreEntry pe;
                pe.tag_ = getUShort(buf + o, byteOrder);
                pe.type_ = getUShort(buf + o + 2, byteOrder);
                pe.count_ = getULong(buf + o + 4, byteOrder);
                pe.size_ = pe.count_ * TypeInfo::typeSize(TypeId(pe.type_));
                pe.offsetLoc_ = o + 8 - shift;
                pe.offset_ = pe.size_ > 4 ? getLong(buf + o + 8, byteOrder) : 0;
                preEntries.push_back(pe);
                o += 12;
            }
        }
        if (rc == 0 && hasNext_) {
            if (len < o + 4) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: " << ExifTags::ifdName(ifdId_)
                          << " memory of the pointer to the next IFD"
                          << " lies outside of the IFD memory buffer.\n";
#endif
                rc = 6;
            }
            else {
                if (alloc_) {
                    memcpy(pNext_, buf + o, 4);
                }
                else {
                    pNext_ = const_cast<byte*>(buf + o);
                }
                next_ = getULong(buf + o, byteOrder);
                if (   static_cast<long>(next_) + shift < 0 
                    || static_cast<long>(next_) + shift >= len) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Warning: " << ExifTags::ifdName(ifdId_)
                              << ": Pointer to next IFD is out of bounds; ignored.\n";
#endif
                    next_ = 0;
                }
            }
        }
        // Set the offset of the first data entry outside of the IFD.
        if (rc == 0 && preEntries.size() > 0) {
            // Find the entry with the smallest offset
            Ifd::PreEntries::const_iterator i = std::min_element(
                preEntries.begin(), preEntries.end(), cmpPreEntriesByOffset);
            // Only do something if there is at least one entry with data
            // outside the IFD directory itself.
            if (i->size_ > 4) {
                // Set the offset of the first data entry outside of the IFD
                if (i->offset_ + shift < 0) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Error: Offset of the 1st data entry of "
                              << ExifTags::ifdName(ifdId_)
                              << " is out of bounds:\n"
                              << " Offset = 0x" << std::setw(8)
                              << std::setfill('0') << std::hex
                              << i->offset_ - offset_ // relative to start of IFD
                              << ", is before start of buffer by "
                              << std::dec << -1 * (i->offset_ + shift)
                              << " Bytes\n";
#endif
                    rc = 6;
                }
                else if (i->offset_ + shift + i->size_ > len) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Error: Upper boundary of the 1st data entry of "
                              << ExifTags::ifdName(ifdId_)
                              << " is out of bounds:\n"
                              << " Offset = 0x" << std::setw(8)
                              << std::setfill('0') << std::hex
                              << i->offset_ - offset_ // relative to start of IFD
                              << ", exceeds buffer size by "
                              << std::dec << i->offset_ + shift + i->size_ - len
                              << " Bytes\n";
#endif
                    rc = 6;
                }
                else {
                    dataOffset_ = i->offset_;
                }
            }
        }
        // Convert the pre-IFD entries to the actual entries, assign the data
        // to each IFD entry and calculate relative offsets, relative to the
        // start of the IFD
        if (rc == 0) {
            entries_.clear();
            int idx = 0;
            const Ifd::PreEntries::iterator begin = preEntries.begin();
            const Ifd::PreEntries::iterator end = preEntries.end();
            for (Ifd::PreEntries::iterator i = begin; i != end; ++i) {
                Entry e(alloc_);
                e.setIfdId(ifdId_);
                e.setIdx(++idx);
                e.setTag(i->tag_);
                long tmpOffset = // still from the start of the TIFF header
                    i->size_ > 4 ? i->offset_ : i->offsetLoc_;
                if (tmpOffset + shift + i->size_ > len) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Warning: Upper boundary of data for "
                              << ExifTags::ifdName(ifdId_)
                              << " entry " << static_cast<int>(i - begin)
                              << " is out of bounds:\n"
                              << " Offset = 0x" << std::setw(8)
                              << std::setfill('0') << std::hex
                              << tmpOffset - offset_ // relative to start of IFD
                              << ", size = " << std::dec << i->size_
                              << ", exceeds buffer size by "
                              << tmpOffset + shift + i->size_ - len
                              << " Bytes; Truncating the data.\n";
#endif
                    // Truncate the entry
                    i->size_ = 0;
                    i->count_ = 0;
                    tmpOffset = i->offsetLoc_;
                }
                // Set the offset to the data, relative to start of IFD
                e.setOffset(tmpOffset - offset_);
                // Set the size to at least for bytes to accomodate offset-data
#ifndef SUPPRESS_WARNINGS
                if (i->type_ < 1 || i->type_ > 10 || i->type_ == 6) {
                    std::cerr << "Warning: "
                              << ExifTags::ifdName(ifdId_) << " tag 0x"
                              << std::setw(4) << std::setfill('0') << std::hex
                              << i->tag_ << " has invalid Exif type "
                              << std::dec << i->type_
                              << "; using 7 (undefined).\n";
                }
#endif
                e.setValue(i->type_, i->count_, buf + start + e.offset(),
                           std::max(long(4), i->size_));
                this->add(e);
            }
        }
        if (!alloc_) pBase_ = const_cast<byte*>(buf + shift);
        if (rc) this->clear();

        return rc;
    } // Ifd::read

    Ifd::const_iterator Ifd::findIdx(int idx) const
    {
        return std::find_if(entries_.begin(), entries_.end(),
                            FindEntryByIdx(idx));
    }

    Ifd::iterator Ifd::findIdx(int idx)
    {
        return std::find_if(entries_.begin(), entries_.end(),
                            FindEntryByIdx(idx));
    }

    Ifd::const_iterator Ifd::findTag(uint16_t tag) const
    {
        return std::find_if(entries_.begin(), entries_.end(),
                            FindEntryByTag(tag));
    }

    Ifd::iterator Ifd::findTag(uint16_t tag)
    {
        return std::find_if(entries_.begin(), entries_.end(),
                            FindEntryByTag(tag));
    }

    void Ifd::sortByTag()
    {
        std::sort(entries_.begin(), entries_.end(), cmpEntriesByTag);
    }

    int Ifd::readSubIfd(
        Ifd& dest, const byte* buf, long len, ByteOrder byteOrder, uint16_t tag
    ) const
    {
        int rc = 0;
        const_iterator pos = findTag(tag);
        if (pos != entries_.end()) {
            long offset = getULong(pos->data(), byteOrder);
            if (len < offset) {
                rc = 6;
            }
            else {
                rc = dest.read(buf, len, offset, byteOrder);
            }
        }
        return rc;
    } // Ifd::readSubIfd

    long Ifd::copy(byte* buf, ByteOrder byteOrder, long offset)
    {
        if (entries_.size() == 0 && next_ == 0) return 0;
        if (offset != 0) offset_ = offset;

        // Add the number of entries to the data buffer
        us2Data(buf, static_cast<uint16_t>(entries_.size()), byteOrder);
        long o = 2;

        // Add all directory entries to the data buffer
        long dataSize = 0;
        long dataAreaSize = 0;
        long totalDataSize = 0;
        const iterator b = entries_.begin();
        const iterator e = entries_.end();
        iterator i;
        for (i = b; i != e; ++i) {
            if (i->size() > 4) {
                totalDataSize += i->size();
            }
        }
        for (i = b; i != e; ++i) {
            us2Data(buf + o, i->tag(), byteOrder);
            us2Data(buf + o + 2, i->type(), byteOrder);
            ul2Data(buf + o + 4, i->count(), byteOrder);
            if (i->sizeDataArea() > 0) {
                long dataAreaOffset = offset_+size()+totalDataSize+dataAreaSize;
                i->setDataAreaOffsets(dataAreaOffset, byteOrder);
                dataAreaSize += i->sizeDataArea();
            }
            if (i->size() > 4) {
                // Set the offset of the entry, data immediately follows the IFD
                i->setOffset(size() + dataSize);
                l2Data(buf + o + 8, offset_ + i->offset(), byteOrder);
                dataSize += i->size();
            }
            else {
                // Copy data into the offset field
                memset(buf + o + 8, 0x0, 4);
                memcpy(buf + o + 8, i->data(), i->size());
            }
            o += 12;
        }

        if (hasNext_) {
            // Add the offset to the next IFD to the data buffer
            if (pNext_) {
                memcpy(buf + o, pNext_, 4);
            }
            else {
                memset(buf + o, 0x0, 4);
            }
            o += 4;
        }

        // Add the data of all IFD entries to the data buffer
        for (i = b; i != e; ++i) {
            if (i->size() > 4) {
                memcpy(buf + o, i->data(), i->size());
                o += i->size();
            }
        }

        // Add all data areas to the data buffer
        for (i = b; i != e; ++i) {
            if (i->sizeDataArea() > 0) {
                memcpy(buf + o, i->dataArea(), i->sizeDataArea());
                o += i->sizeDataArea();
            }
        }

        return o;
    } // Ifd::copy

    void Ifd::clear()
    {
        entries_.clear();
        offset_ = 0;
        dataOffset_ = 0;
        if (hasNext_) {
            if (alloc_) {
                memset(pNext_, 0x0, 4);
            }
            else {
                pBase_ = 0;
                pNext_ = 0;
            }
            next_ = 0;
        }
    } // Ifd::clear

    void Ifd::setNext(uint32_t next, ByteOrder byteOrder)
    {
        if (hasNext_) {
            assert(pNext_);
            ul2Data(pNext_, next, byteOrder);
            next_ = next;
        }
    }

    void Ifd::add(const Entry& entry)
    {
        assert(alloc_ == entry.alloc());
        assert(ifdId_ == entry.ifdId());
        // allow duplicates
        entries_.push_back(entry);
    }

    int Ifd::erase(uint16_t tag)
    {
        int idx = 0;
        iterator pos = findTag(tag);
        if (pos != end()) {
            idx = pos->idx();
            erase(pos);
        }
        return idx;
    }

    Ifd::iterator Ifd::erase(iterator pos)
    {
        return entries_.erase(pos);
    }

    byte* Ifd::updateBase(byte* pNewBase)
    {
        byte *pOld = 0;
        if (!alloc_) {
            iterator end = this->end();
            for (iterator pos = begin(); pos != end; ++pos) {
                pos->updateBase(pBase_, pNewBase);
            }
            if (hasNext_) {
                pNext_ = pNext_ - pBase_ + pNewBase;
            }
            pOld = pBase_;
            pBase_ = pNewBase;
        }
        return pOld;
    }

    long Ifd::size() const
    {
        if (entries_.size() == 0 && next_ == 0) return 0;
        return static_cast<long>(2 + 12 * entries_.size() + (hasNext_ ? 4 : 0));
    }

    long Ifd::dataSize() const
    {
        long dataSize = 0;
        const_iterator end = this->end();
        for (const_iterator i = begin(); i != end; ++i) {
            if (i->size() > 4) dataSize += i->size();
            dataSize += i->sizeDataArea();
        }
        return dataSize;
    }

    void Ifd::print(std::ostream& os, const std::string& prefix) const
    {
        if (entries_.size() == 0) return;
        // Print a header
        os << prefix << "IFD Offset: 0x"
           << std::setw(8) << std::setfill('0') << std::hex << std::right
           << offset_
           << ",   IFD Entries: "
           << std::setfill(' ') << std::dec << std::right
           << static_cast<unsigned int>(entries_.size()) << "\n"
           << prefix << "Entry     Tag  Format   (Bytes each)  Number  Offset\n"
           << prefix << "-----  ------  ---------------------  ------  -----------\n";
        // Print IFD entries
        const const_iterator b = entries_.begin();
        const const_iterator e = entries_.end();
        const_iterator i = b;
        for (; i != e; ++i) {
            std::ostringstream offset;
            if (i->size() > 4) {
                offset << " 0x" << std::setw(8) << std::setfill('0')
                       << std::hex << std::right << i->offset();
            }
            else {
                const byte* data = i->data();
                for (int k = 0; k < i->size(); ++k) {
                    offset << std::setw(2) << std::setfill('0') << std::hex
                           << (int)data[k] << " ";
                }
            }
            os << prefix << std::setw(5) << std::setfill(' ') << std::dec
               << std::right << static_cast<int>(i - b)
               << "  0x" << std::setw(4) << std::setfill('0') << std::hex
               << std::right << i->tag()
               << "  " << std::setw(17) << std::setfill(' ')
               << std::left << i->typeName()
               << " (" << std::dec << i->typeSize() << ")"
               << "  " << std::setw(6) << std::setfill(' ') << std::dec
               << std::right << i->count()
               << "  " << offset.str()
               << "\n";
        }
        if (hasNext_) {
            os << prefix << "Next IFD: 0x"
               << std::setw(8) << std::setfill('0') << std::hex
               << std::right << next() << "\n";
        }
        // Print data of IFD entries
        for (i = b; i != e; ++i) {
            if (i->size() > 4) {
                os << "Data of entry " << static_cast<int>(i - b) << ":\n";
                hexdump(os, i->data(), i->size(), offset_ + i->offset());
            }
        }

    } // Ifd::print

    // *************************************************************************
    // free functions

    bool cmpEntriesByTag(const Entry& lhs, const Entry& rhs)
    {
        return lhs.tag() < rhs.tag();
    }

    bool cmpPreEntriesByOffset(const Ifd::PreEntry& lhs, const Ifd::PreEntry& rhs)
    {
        // We need to ignore entries with size <= 4, so by definition,
        // entries with size <= 4 are greater than those with size > 4
        // when compared by their offset.
        if (lhs.size_ <= 4) {
            return false; // lhs is greater by definition, or they are equal
        }
        if (rhs.size_ <= 4) {
            return true; // rhs is greater by definition (they cannot be equal)
        }
        return lhs.offset_ < rhs.offset_;
    } // cmpPreEntriesByOffset

}                                       // namespace Exiv2
