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
  Version:   $Name:  $ $Revision: 1.1 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.1 $ $RCSfile: ifd.cpp,v $")

// *****************************************************************************
// included header files
#include "ifd.hpp"
#include "types.hpp"
#include "error.hpp"

// + standard includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exif {

    RawEntry::RawEntry()
        : ifdId_(ifdIdNotSet), ifdIdx_(-1),
          tag_(0), type_(0), count_(0), offset_(0), size_(0)
    {
        memset(offsetData_, 0x0, 4);
    }

    Entry::Entry(bool alloc)
        : alloc_(alloc), ifdId_(ifdIdNotSet), ifdIdx_(-1),
          tag_(0), type_(0), count_(0), offset_(0), size_(0), data_(0)
    {
        memset(offsetData_, 0x0, 4);
    }

    Entry::Entry(const RawEntry& e, const char* buf, bool alloc)
        : alloc_(alloc), ifdId_(e.ifdId_), ifdIdx_(e.ifdIdx_),
          tag_(e.tag_), type_(e.type_), count_(e.count_), offset_(e.offset_), 
          size_(e.size_), data_(0)
    {
        if (size_ > 4) {
            if (alloc_) {
                data_ = new char[size_]; 
                memcpy(data_, buf + offset_, size_);
            }
            else {
                data_ = const_cast<char*>(buf) + offset_;
            }
        }
        else {
            memcpy(offsetData_, e.offsetData_, 4);
        }
    }

    Entry::~Entry()
    {
        if (alloc_) delete[] data_;
    }

    Entry::Entry(const Entry& rhs)
        : alloc_(rhs.alloc_), ifdId_(rhs.ifdId_),
          ifdIdx_(rhs.ifdIdx_), tag_(rhs.tag_), type_(rhs.type_), 
          count_(rhs.count_), offset_(rhs.offset_), size_(rhs.size_), data_(0)
    {
        memcpy(offsetData_, rhs.offsetData_, 4);
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
        ifdIdx_ = rhs.ifdIdx_;
        tag_ = rhs.tag_;
        type_ = rhs.type_;
        count_ = rhs.count_;
        offset_ = rhs.offset_;
        memcpy(offsetData_, rhs.offsetData_, 4);
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

    const char* Entry::data() const
    {
        if (size_ > 4) return data_;
        return offsetData_;
    }

    void Entry::setOffset(uint32 offset, ByteOrder byteOrder)
    {
        if (size_ > 4) {
            offset_ = offset;
        }
        else {
            ul2Data(offsetData_, offset, byteOrder);
        }
    }

    void Entry::setValue(uint16 type, const char* buf, long size)
    {
        if (size > 4 && alloc_) {
            delete[] data_;
            data_ = new char[size];
            memcpy(data_, buf, size);
        }
        if (size <= 4 && alloc_) {
            delete[] data_;
            data_ = 0;
            memset(offsetData_, 0x0, 4);
            memcpy(offsetData_, buf, size);
        }
        if (size > 4 && !alloc_) {
            if (size > size_) throw Error("Size too large");
            memset(data_, 0x0, size_);
            memcpy(data_, buf, size);
        }
        if (size <= 4 && !alloc_) {
            data_ = 0;
            memset(offsetData_, 0x0, 4);
            memcpy(offsetData_, buf, size);
        }
        size_ = size;
        type_ = type;
        count_ = size / TypeInfo::typeSize(TypeId(type));
    } // Entry::setValue

    Ifd::Ifd(IfdId ifdId)
        : alloc_(true), ifdId_(ifdId), offset_(0), next_(0)
    {
    }

    Ifd::Ifd(IfdId ifdId, uint32 offset)
        : alloc_(true), ifdId_(ifdId), offset_(offset), next_(0)
    {
    }

    Ifd::Ifd(IfdId ifdId, uint32 offset, bool alloc)
        : alloc_(alloc), ifdId_(ifdId), offset_(offset), next_(0)
    {
    }

    int Ifd::read(const char* buf, ByteOrder byteOrder, long offset)
    {
        offset_ = offset;
        int n = getUShort(buf, byteOrder);
        long o = 2;

        // Create an array of raw entries
        RawEntries rawEntries;
        for (int i = 0; i < n; ++i) {
            RawEntry e; 
            e.ifdId_ = ifdId_;
            e.ifdIdx_ = i;
            e.tag_ = getUShort(buf+o, byteOrder);
            e.type_ = getUShort(buf+o+2, byteOrder);
            e.count_ = getULong(buf+o+4, byteOrder);
            e.size_ = e.count_ * TypeInfo::typeSize(TypeId(e.type_));
            e.offset_ = e.size_ > 4 ? getULong(buf+o+8, byteOrder) : 0;
            memcpy(e.offsetData_, buf+o+8, 4);
            rawEntries.push_back(e);
            o += 12;
        }
        next_ = getULong(buf+o, byteOrder);

        // Guess the offset of the IFD, if it was not given. The guess is based
        // on the assumption that the smallest offset points to a data buffer
        // directly following the IFD. Subsequently all offsets of IFD entries
        // will need to be recalculated.
        if (offset_ == 0 && rawEntries.size() > 0) {
            // Find the entry with the smallest offset
            RawEntries::const_iterator i;
            i = std::min_element(
                rawEntries.begin(), rawEntries.end(), cmpRawEntriesByOffset);
            // Set the 'guessed' IFD offset, the test is needed for the case when
            // all entries have data sizes not exceeding 4.
            if (i->size_ > 4) {
                offset_ = i->offset_ - size();
            }
        }

        // Convert 'raw' IFD entries to the actual entries, assign the data
        // to each IFD entry and calculate relative offsets, relative to the
        // start of the IFD
        entries_.clear();
        const RawEntries::iterator begin = rawEntries.begin();
        const RawEntries::iterator end = rawEntries.end();
        for (RawEntries::iterator i = begin; i != end; ++i) {
            if (i->size_ > 4) {
                i->offset_ = i->offset_ - offset_;
            }
            add(Entry(*i, buf, alloc_));
        }

        return 0;
    } // Ifd::read

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

    long Ifd::copy(char* buf, ByteOrder byteOrder, long offset) const
    {
        if (offset == 0) offset = offset_;

        // Add the number of entries to the data buffer
        us2Data(buf, entries_.size(), byteOrder);
        long o = 2;

        // Add all directory entries to the data buffer
        long dataSize = 0;
        const const_iterator b = entries_.begin();
        const const_iterator e = entries_.end();
        const_iterator i = b;
        for (; i != e; ++i) {
            us2Data(buf+o, i->tag(), byteOrder);
            us2Data(buf+o+2, i->type(), byteOrder);
            ul2Data(buf+o+4, i->count(), byteOrder);
            if (i->size() > 4) {
                // Calculate offset, data immediately follows the IFD
                ul2Data(buf+o+8, offset + size() + dataSize, byteOrder);
                dataSize += i->size();
            }
            else {
                // Copy data into the offset field
                memcpy(buf+o+8, i->data(), 4);
            }
            o += 12;
        }

        // Add the offset to the next IFD to the data buffer
        o += ul2Data(buf+o, next_, byteOrder);

        // Add the data of all IFD entries to the data buffer
        for (i = b; i != e; ++i) {
            if (i->size() > 4) {
                memcpy(buf + o, i->data(), i->size());
                o += i->size();
            }
        }

        return o;
    } // Ifd::copy

    void Ifd::add(const Entry& entry)
    {
        // Todo: Implement Assert (Stroustup 24.3.7.2)
        if (alloc_ != entry.alloc()) {
            throw Error("Invariant alloc violated in Ifd::add");
        }
        if (ifdId_ != entry.ifdId()) {
            throw Error("Invariant ifdId violated in Ifd::add");
        }
        erase(entry.tag());
        entries_.push_back(entry);
    }

    void Ifd::erase(uint16 tag)
    {
        iterator pos = findTag(tag);
        if (pos != end()) erase(pos);
    }

    void Ifd::erase(iterator pos)
    {
        if (alloc_) {
            entries_.erase(pos);
        }
    }

    void Ifd::setOffset(uint16 tag, uint32 offset, ByteOrder byteOrder)
    {
        iterator pos = findTag(tag);
        if (pos == entries_.end()) {
            RawEntry e;
            e.ifdId_ = ifdId_;
            e.ifdIdx_ = 0;
            e.tag_ = tag;
            e.type_ = unsignedLong;
            e.count_ = 1;
            e.offset_ = 0;
            e.size_ = 4;
            add(Entry(e, 0, alloc_));
            pos = findTag(tag);
        }
        pos->setOffset(offset, byteOrder);
    } // Ifd::setOffset

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
                offset << std::setw(2) << std::setfill('0') << std::hex
                       << (int)data[0] << " "
                       << std::setw(2) << std::setfill('0') << std::hex
                       << (int)data[1] << " "
                       << std::setw(2) << std::setfill('0') << std::hex
                       << (int)data[2] << " "
                       << std::setw(2) << std::setfill('0') << std::hex
                       << (int)data[3] << " ";
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
           << std::right << next_ << "\n";
        // Print data of IFD entries 
        for (i = b; i != e; ++i) {
            if (i->size() > 4) {
                os << "Data of entry " << i - b << ":\n";
                hexdump(os, i->data(), i->size());
            }
        }

    } // Ifd::print

    // *************************************************************************
    // free functions

    bool cmpRawEntriesByOffset(const RawEntry& lhs, const RawEntry& rhs)
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
    }

    bool cmpEntriesByTag(const Entry& lhs, const Entry& rhs)
    {
        return lhs.tag() < rhs.tag();
    }

}                                       // namespace Exif
