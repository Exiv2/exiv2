// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*
  File:      ifd.cpp
  Version:   $Name:  $ $Revision: 1.14 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.14 $ $RCSfile: ifd.cpp,v $")

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
namespace Exif {

    Entry::Entry(bool alloc)
        : alloc_(alloc), ifdId_(ifdIdNotSet), idx_(0), makerNote_(0), 
          tag_(0), type_(0), count_(0), offset_(0), size_(0), data_(0)
    {
    }

    Entry::~Entry()
    {
        if (alloc_) delete[] data_;
        // do *not* delete the MakerNote
    }

    Entry::Entry(const Entry& rhs)
        : alloc_(rhs.alloc_), ifdId_(rhs.ifdId_), idx_(rhs.idx_),
          makerNote_(rhs.makerNote_), tag_(rhs.tag_), type_(rhs.type_), 
          count_(rhs.count_), offset_(rhs.offset_), size_(rhs.size_), data_(0)
    {
        if (alloc_) {
            if (rhs.data_) {
                data_ = new char[rhs.size()];
                memcpy(data_, rhs.data_, rhs.size());
            }
        }
        else {
            data_ = rhs.data_;
        }
    }

    Entry::Entry& Entry::operator=(const Entry& rhs)
    {
        if (this == &rhs) return *this;
        alloc_ = rhs.alloc_;
        ifdId_ = rhs.ifdId_;
        idx_ = rhs.idx_;
        makerNote_ = rhs.makerNote_;
        tag_ = rhs.tag_;
        type_ = rhs.type_;
        count_ = rhs.count_;
        offset_ = rhs.offset_;
        size_ = rhs.size_;
        if (alloc_) {
            delete[] data_;
            data_ = 0;
            if (rhs.data_) {
                data_ = new char[rhs.size()];
                memcpy(data_, rhs.data_, rhs.size());
            }
        }
        else {
            data_ = rhs.data_;
        }
        return *this;
    } // Entry::operator=

    void Entry::setValue(uint32 data, ByteOrder byteOrder)
    {
        if (data_ == 0 || size_ < 4) {
            assert(alloc_);
            size_ = 4;
            delete[] data_;
            data_ = new char[size_];
        }
        ul2Data(data_, data, byteOrder);
        // do not change size_
        type_ = unsignedLong;
        count_ = 1;
    }

    void Entry::setValue(uint16 type, uint32 count, const char* buf, long len)
    {
        long dataSize = count * TypeInfo::typeSize(TypeId(type));
        // No minimum size requirement, but make sure the buffer can hold the data
        if (len < dataSize) {
            throw Error("Size too small");
        }
        if (alloc_) {
            delete[] data_;
            data_ = new char[len];
            memset(data_, 0x0, len);
            memcpy(data_, buf, dataSize);
            size_ = len;
        }
        else {
            if (size_ == 0) {
                // Set the data pointer of a virgin entry
                data_ = const_cast<char*>(buf);
                size_ = len;
            }
            else {
                // Overwrite existing data if it fits into the buffer
                if (dataSize > size_) throw Error("Value too large");
                memset(data_, 0x0, size_);
                memcpy(data_, buf, dataSize);
                // do not change size_
            }
        }
        type_ = type;
        count_ = count;
    } // Entry::setValue

    const char* Entry::component(uint32 n) const
    {
        if (n >= count()) return 0;
        return data() + n * typeSize();
    } // Entry::component

    Ifd::Ifd(IfdId ifdId)
        : alloc_(true), ifdId_(ifdId), offset_(0), pNext_(0), next_(0)
    {
        pNext_ = new char[4];
        memset(pNext_, 0x0, 4);
    }

    Ifd::Ifd(IfdId ifdId, uint32 offset)
        : alloc_(true), ifdId_(ifdId), offset_(offset), pNext_(0), next_(0)
    {
        pNext_ = new char[4];
        memset(pNext_, 0x0, 4);
    }

    Ifd::Ifd(IfdId ifdId, uint32 offset, bool alloc)
        : alloc_(alloc), ifdId_(ifdId), offset_(offset), pNext_(0), next_(0)
    {
        if (alloc_) {
            pNext_ = new char[4];
            memset(pNext_, 0x0, 4);
        }
    }

    Ifd::~Ifd()
    {
        if (alloc_) delete[] pNext_;
    }

    Ifd::Ifd(const Ifd& rhs)
        : alloc_(rhs.alloc_), entries_(rhs.entries_), ifdId_(rhs.ifdId_),
          offset_(rhs.offset_), pNext_(rhs.pNext_), next_(rhs.next_)
    {
        if (alloc_ && rhs.pNext_) {
            pNext_ = new char[4];
            memcpy(pNext_, rhs.pNext_, 4); 
        }
    }

    int Ifd::read(const char* buf, ByteOrder byteOrder, long offset)
    {
        offset_ = offset;

        Ifd::PreEntries preEntries;

        int n = getUShort(buf, byteOrder);
        long o = 2;

        for (int i = 0; i < n; ++i) {
            Ifd::PreEntry pe;
            pe.tag_ = getUShort(buf+o, byteOrder);
            pe.type_ = getUShort(buf+o+2, byteOrder);
            pe.count_ = getULong(buf+o+4, byteOrder);
            pe.size_ = pe.count_ * TypeInfo::typeSize(TypeId(pe.type_));
            pe.offsetLoc_ = o + 8;
            pe.offset_ = pe.size_ > 4 ? getULong(buf+o+8, byteOrder) : 0;
            preEntries.push_back(pe);
            o += 12;
        }
        if (alloc_) {
            memcpy(pNext_, buf + o, 4);
        }
        else {
            pNext_ = const_cast<char*>(buf + o);
        }
        next_ = getULong(buf+o, byteOrder);

        // Guess the offset of the IFD, if it was not given. The guess is based
        // on the assumption that the smallest offset points to a data buffer
        // directly following the IFD. Subsequently all offsets of IFD entries
        // will need to be recalculated.
        if (offset_ == 0 && preEntries.size() > 0) {
            // Find the entry with the smallest offset
            Ifd::PreEntries::const_iterator i = std::min_element(
                preEntries.begin(), preEntries.end(), cmpPreEntriesByOffset);
            // Set the 'guessed' IFD offset, the test is needed for the case when
            // all entries have data sizes not exceeding 4.
            if (i->size_ > 4) {
                offset_ = i->offset_ - size();
            }
        }

        // Convert the pre-IFD entries to the actual entries, assign the data
        // to each IFD entry and calculate relative offsets, relative to the
        // start of the IFD
        entries_.clear();
        int idx = 0;
        const Ifd::PreEntries::iterator begin = preEntries.begin();
        const Ifd::PreEntries::iterator end = preEntries.end();
        for (Ifd::PreEntries::iterator i = begin; i != end; ++i) {
            Entry e(alloc_);
            e.setIfdId(ifdId_);
            e.setIdx(++idx);
            e.setTag(i->tag_);
            // Set the offset to the data, relative to start of IFD
            e.setOffset(i->size_ > 4 ? i->offset_ - offset_ : i->offsetLoc_);
            // Set the size to at least for bytes to accomodate offset-data
            e.setValue(i->type_, i->count_, buf + e.offset(), 
                       std::max(long(4), i->size_));
            this->add(e);
        }

        return 0;
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

    Ifd::const_iterator Ifd::findTag(uint16 tag) const 
    {
        return std::find_if(entries_.begin(), entries_.end(),
                            FindEntryByTag(tag));
    }

    Ifd::iterator Ifd::findTag(uint16 tag)
    {
        return std::find_if(entries_.begin(), entries_.end(),
                            FindEntryByTag(tag));
    }

    void Ifd::sortByTag()
    {
        std::sort(entries_.begin(), entries_.end(), cmpEntriesByTag);
    }

    int Ifd::readSubIfd(
        Ifd& dest, const char* buf, ByteOrder byteOrder, uint16 tag
    ) const
    {
        int rc = 0;
        const_iterator pos = findTag(tag);
        if (pos != entries_.end()) {
            uint32 offset = getULong(pos->data(), byteOrder);
            rc = dest.read(buf + offset, byteOrder, offset);
        }
        return rc;
    } // Ifd::readSubIfd

    long Ifd::copy(char* buf, ByteOrder byteOrder, long offset)
    {
        if (offset != 0) offset_ = offset;

        // Add the number of entries to the data buffer
        us2Data(buf, entries_.size(), byteOrder);
        long o = 2;

        // Add all directory entries to the data buffer
        long dataSize = 0;
        const iterator b = entries_.begin();
        const iterator e = entries_.end();
        iterator i = b;
        for (; i != e; ++i) {
            us2Data(buf + o, i->tag(), byteOrder);
            us2Data(buf + o + 2, i->type(), byteOrder);
            ul2Data(buf + o + 4, i->count(), byteOrder);
            if (i->size() > 4) {
                // Set the offset of the entry, data immediately follows the IFD
                i->setOffset(size() + dataSize);
                ul2Data(buf + o + 8, offset_ + i->offset(), byteOrder);
                dataSize += i->size();
            }
            else {
                // Copy data into the offset field
                memset(buf + o + 8, 0x0, 4);
                memcpy(buf + o + 8, i->data(), i->size());
            }
            o += 12;
        }

        // Add the offset to the next IFD to the data buffer
        if (pNext_) {
            memcpy(buf + o, pNext_, 4);
        }
        else {
            memset(buf + o, 0x0, 4);
        }
        o += 4;

        // Add the data of all IFD entries to the data buffer
        for (i = b; i != e; ++i) {
            if (i->size() > 4) {
                memcpy(buf + o, i->data(), i->size());
                o += i->size();
            }
        }

        return o;
    } // Ifd::copy

    void Ifd::clear()
    {
        entries_.clear();
        if (alloc_) {
            memset(pNext_, 0x0, 4);
        }
        else {
            pNext_ = 0;
        }
        offset_ = 0;
    } // Ifd::clear

    void Ifd::setNext(uint32 next, ByteOrder byteOrder)
    {
        assert(pNext_);
        ul2Data(pNext_, next, byteOrder);
        next_ = next;
    }

    void Ifd::add(const Entry& entry)
    {
        assert(alloc_ == entry.alloc());
        assert(ifdId_ == entry.ifdId());
        // allow duplicates
        entries_.push_back(entry);
    }

    int Ifd::erase(uint16 tag)
    {
        int idx = 0;
        iterator pos = findTag(tag);
        if (pos != end()) {
            idx = pos->idx();
            erase(pos);
        }
        return idx;
    }

    void Ifd::erase(iterator pos)
    {
        entries_.erase(pos);
    }

    long Ifd::size() const
    {
        if (entries_.size() == 0) return 0;
        return 2 + 12 * entries_.size() + 4; 
    }

    long Ifd::dataSize() const
    {
        long dataSize = 0;
        const_iterator end = this->end();
        for (const_iterator i = begin(); i != end; ++i) {
            if (i->size() > 4) dataSize += i->size();
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
           << entries_.size() << "\n"
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
                unsigned char* data = (unsigned char*)i->data();
                for (int k = 0; k < i->size(); ++k) {
                    offset << std::setw(2) << std::setfill('0') << std::hex
                           << (int)data[k] << " ";
                }
            }
            os << prefix << std::setw(5) << std::setfill(' ') << std::dec
               << std::right << i - b
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
        os << prefix << "Next IFD: 0x" 
           << std::setw(8) << std::setfill('0') << std::hex
           << std::right << next() << "\n";
        // Print data of IFD entries 
        for (i = b; i != e; ++i) {
            if (i->size() > 4) {
                os << "Data of entry " << i - b << ":\n";
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

}                                       // namespace Exif
