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
  File:      exif.cpp
  Version:   $Name:  $ $Revision: 1.15 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.15 $ $RCSfile: exif.cpp,v $")

// *****************************************************************************
// included header files
#include "exif.hpp"
#include "tags.hpp"

// + standard includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <utility>
#include <algorithm>

#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exif {

    JpegImage::JpegImage()
        : sizeExifData_(0), offsetExifData_(0), exifData_(0)
    {
    }

    JpegImage::~JpegImage()
    {
        delete[] exifData_;
    }

    const uint16 JpegImage::soi_    = 0xffd8;
    const uint16 JpegImage::app1_   = 0xffe1;
    const char JpegImage::exifId_[] = "Exif\0\0";

    bool JpegImage::isJpeg(std::istream& is)
    {
        char c;
        is.get(c);
        if (!is.good()) return false;
        if (static_cast<char>((soi_ & 0xff00) >> 8) != c) {
            is.unget();
            return false;
        }
        is.get(c);
        if (!is.good()) return false;
        if (static_cast<char>(soi_ & 0x00ff) != c) {
            is.unget();
            return false;
        }
        return true;
    }

    int JpegImage::readExifData(const std::string& path)
    {
        std::ifstream file(path.c_str());
        if (!file) return -1;
        return readExifData(file);
    }

    int JpegImage::readExifData(std::istream& is)
    {
        // Check if this is a JPEG image in the first place
        if (!isJpeg(is)) {
            if (!is.good()) return 1;
            return 2;
        }

        // Todo: implement this properly: the APP1 segment may not follow
        //       immediately after SOI.
        char marker[2];
        marker[0] = '\0'; 
        marker[1] = '\0';
        long offsetApp1 = 2;
        // Read the APP1 marker
        is.read(marker, 2);
        if (!is.good()) return 1;
        // Check the APP1 marker
        if (getUShort(marker, bigEndian) != app1_) return 3;

        // Read the length of the APP1 field and the Exif identifier
        char tmpbuf[8];
        ::memset(tmpbuf, 0x0, 8);
        is.read(tmpbuf, 8);
        if (!is.good()) return 1;
        // Get the length of the APP1 field and do a plausibility check
        long app1Length = getUShort(tmpbuf, bigEndian);
        if (app1Length < 8) return 4;
        // Check the Exif identifier
        if (::memcmp(tmpbuf+2, exifId_, 6) != 0) return 4;
 
        // Read the rest of the APP1 field (Exif data)
        long sizeExifData = app1Length - 8;
        exifData_ = new char[sizeExifData];
        ::memset(exifData_, 0x0, sizeExifData);
        is.read(exifData_, sizeExifData);
        if (!is.good()) {
            delete[] exifData_;
            exifData_ = 0;
            return 1;
        }
        // Finally, set the size and offset of the Exif data buffer
        sizeExifData_ = sizeExifData;
        offsetExifData_ = offsetApp1 + 10;

        return 0;
    } // JpegImage::readExifData

    TiffHeader::TiffHeader(ByteOrder byteOrder) 
        : byteOrder_(byteOrder), tag_(0x002a), offset_(0x00000008)
    {
    }

    int TiffHeader::read(const char* buf)
    {
        if (buf[0] == 0x49 && buf[1] == 0x49) {
            byteOrder_ = littleEndian;
        }
        else if (buf[0] == 0x4d && buf[1] == 0x4d) {
            byteOrder_ = bigEndian;
        }
        else {
            return 1;
        }
        tag_ = getUShort(buf+2, byteOrder_);
        offset_ = getULong(buf+4, byteOrder_);
        return 0;
    }

    long TiffHeader::copy(char* buf) const
    {
        switch (byteOrder_) {
        case littleEndian:
            buf[0] = 0x49;
            buf[1] = 0x49;
            break;
        case bigEndian:
            buf[0] = 0x4d;
            buf[1] = 0x4d;
            break;
        }
        us2Data(buf+2, tag_, byteOrder_);
        ul2Data(buf+4, offset_, byteOrder_);
        return size();
    }

    Value* Value::create(TypeId typeId)
    {
        Value* value = 0;
        switch (typeId) {
        case invalid:
            value = new DataValue(invalid);
            break;
        case unsignedByte:
            value = new DataValue(unsignedByte);
            break;
        case asciiString:
            value =  new AsciiValue;
            break;
        case unsignedShort:
            value = new ValueType<uint16>;
            break;
        case unsignedLong:
            value = new ValueType<uint32>;
            break;
        case unsignedRational:
            value = new ValueType<URational>;
            break;
        case invalid6:
            value = new DataValue(invalid6);
            break;
        case undefined:
            value = new DataValue;
            break;
        case signedShort:
            value = new ValueType<int16>;
            break;
        case signedLong:
            value = new ValueType<int32>;
            break;
        case signedRational:
            value = new ValueType<Rational>;
            break;
        default:
            value = new DataValue(typeId);
            break;
        }
        return value;
    } // Value::create

    std::string Value::toString() const
    {
        std::ostringstream os;
        write(os);
        return os.str();
    }

    void DataValue::read(const char* buf, long len, ByteOrder byteOrder)
    {
        // byteOrder not needed 
        value_ = std::string(buf, len);
    }

    void DataValue::read(const std::string& buf)
    {
        std::istringstream is(buf);
        int tmp;
        value_.clear();
        while (is >> tmp) {
            value_ += (char)tmp;
        }
    }

    long DataValue::copy(char* buf, ByteOrder byteOrder) const
    {
        // byteOrder not needed
        return value_.copy(buf, value_.size());
    }

    long DataValue::size() const
    {
        return value_.size();
    }

    Value* DataValue::clone() const
    {
        return new DataValue(*this);
    }

    std::ostream& DataValue::write(std::ostream& os) const
    {
        std::string::size_type end = value_.size();
        for (std::string::size_type i = 0; i != end; ++i) {
            os << (int)(unsigned char)value_[i] << " ";
        }
        return os;
    }

    void AsciiValue::read(const char* buf, long len, ByteOrder byteOrder)
    {
        // byteOrder not needed 
        value_ = std::string(buf, len);
    }

    void AsciiValue::read(const std::string& buf)
    {
        value_ = buf;
        if (value_[value_.size()-1] != '\0') value_ += '\0';
    }

    long AsciiValue::copy(char* buf, ByteOrder byteOrder) const
    {
        // byteOrder not needed
        return value_.copy(buf, value_.size());
    }

    long AsciiValue::size() const
    {
        return value_.size();
    }

    Value* AsciiValue::clone() const
    {
        return new AsciiValue(*this);
    }

    std::ostream& AsciiValue::write(std::ostream& os) const
    {
        return os << value_;
    }

    Metadatum::Metadatum(const Entry& e, ByteOrder byteOrder)
        : tag_(e.tag()), ifdId_(e.ifdId()), ifdIdx_(e.ifdIdx()), value_(0)
    {
        value_ = Value::create(TypeId(e.type()));
        value_->read(e.data(), e.size(), byteOrder);

        key_ = std::string(ifdItem()) 
            + "." + std::string(sectionName()) 
            + "." + std::string(tagName());
    }

    Metadatum::Metadatum(const std::string& key, Value* value)
        : ifdIdx_(-1), value_(0), key_(key)
    {
        if (value) value_ = value->clone();

        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string ifdItem = key.substr(0, pos1);
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string sectionName = key.substr(pos0, pos1 - pos0);
        pos0 = pos1 + 1;
        std::string tagName = key.substr(pos0);
        if (tagName == "") throw Error("Invalid key");
        std::pair<IfdId, uint16> p
            = ExifTags::ifdAndTag(ifdItem, sectionName, tagName);
        if (p.first == ifdIdNotSet) throw Error("Invalid key");
        ifdId_ = p.first;
        if (p.second == 0xffff) throw Error("Invalid key");
        tag_ = p.second;
    }

    Metadatum::~Metadatum()
    {
        delete value_;
    }

    Metadatum::Metadatum(const Metadatum& rhs)
        : tag_(rhs.tag_), ifdId_(rhs.ifdId_), 
          ifdIdx_(rhs.ifdIdx_), value_(0), key_(rhs.key_)
    {
        if (rhs.value_ != 0) value_ = rhs.value_->clone(); // deep copy
    }

    Metadatum& Metadatum::operator=(const Metadatum& rhs)
    {
        if (this == &rhs) return *this;
        tag_ = rhs.tag_;
        ifdId_ = rhs.ifdId_;
        ifdIdx_ = rhs.ifdIdx_;
        delete value_;
        value_ = 0;
        if (rhs.value_ != 0) value_ = rhs.value_->clone(); // deep copy
        key_ = rhs.key_;
        return *this;
    } // Metadatum::operator=
    
    void Metadatum::setValue(const Value* value)
    {
        delete value_;
        value_ = value->clone();
    }

    void Metadatum::setValue(const std::string& buf)
    {
        if (value_ == 0) value_ = Value::create(asciiString);
        value_->read(buf);
    }

    RawEntry::RawEntry()
        : ifdId_(ifdIdNotSet), ifdIdx_(-1),
          tag_(0), type_(0), count_(0), offset_(0), size_(0)
    {
        memset(offsetData_, 0x0, 4);
    }

    Entry::Entry(bool alloc)
        : alloc_(alloc), status_(valid), ifdId_(ifdIdNotSet), ifdIdx_(-1),
          tag_(0), type_(0), count_(0), offset_(0), size_(0), data_(0)
    {
        memset(offsetData_, 0x0, 4);
    }

    Entry::Entry(const RawEntry& e, const char* buf, bool alloc)
        : alloc_(alloc), status_(valid), ifdId_(e.ifdId_), ifdIdx_(e.ifdIdx_),
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
        : alloc_(rhs.alloc_), status_(rhs.status_), ifdId_(rhs.ifdId_),
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
        status_ = rhs.status_;
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
            e.size_ = e.count_ * ExifTags::typeSize(TypeId(e.type_));
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
            i = std::min_element(rawEntries.begin(), rawEntries.end(), cmpOffset);
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
            entries_.push_back(Entry(*i, buf, alloc_));
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
        sort(entries_.begin(), entries_.end(), cmpTag);
    }

    int Ifd::readSubIfd(
        Ifd& dest, char* buf, ByteOrder byteOrder, uint16 tag
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
                ::memcpy(buf+o+8, i->data(), 4);
            }
            o += 12;
        }

        // Add the offset to the next IFD to the data buffer
        o += ul2Data(buf+o, next_, byteOrder);

        // Add the data of all IFD entries to the data buffer
        for (i = b; i != e; ++i) {
            if (i->size() > 4) {
                ::memcpy(buf + o, i->data(), i->size());
                o += i->size();
            }
        }

        return o;
    } // Ifd::copy

    void Ifd::add(Metadata::const_iterator begin, 
                  Metadata::const_iterator end,
                  ByteOrder byteOrder)
    {
        for (Metadata::const_iterator i = begin; i != end; ++i) {
            // add only metadata with matching IFD id
            if (i->ifdId() == ifdId_) {
                add(*i, byteOrder);
            }
        }
    } // Ifd::add

    void Ifd::add(const Metadatum& metadatum, ByteOrder byteOrder)
    {
        // Todo: Implement Assert (Stroustup 24.3.7.2)
        if (!alloc_) throw Error("Invariant violated in Ifd::add");

        RawEntry e;
        e.ifdId_ = metadatum.ifdId();
        e.ifdIdx_ = metadatum.ifdIdx();
        e.tag_ = metadatum.tag();
        e.type_ = metadatum.typeId();
        e.count_ = metadatum.count();
        e.size_ = metadatum.size();
        e.offset_ = 0;  // will be calculated when the IFD is written
        char* buf = 0;
        if (e.size_ > 4) {
            buf = new char[e.size_];
            metadatum.copy(buf, byteOrder);
        }
        else {
            metadatum.copy(e.offsetData_, byteOrder);
        }
        erase(metadatum.tag());
        entries_.push_back(Entry(e, buf, alloc_));
        delete[] buf;
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
        else {
            pos->setStatus(Entry::erased);
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
            entries_.push_back(Entry(e, 0, alloc_));
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

    int Thumbnail::read(const char* buf,
                        const ExifData& exifData,
                        ByteOrder byteOrder)
    {
        int rc = 0;
        std::string key = "Thumbnail.ImageStructure.Compression";
        ExifData::const_iterator pos = exifData.findKey(key);
        if (pos == exifData.end()) return -1; // no thumbnail
        long compression = pos->toLong();
        if (compression == 6) {
            rc = readJpegImage(buf, exifData);
        }
        else {
            rc = readTiffImage(buf, exifData, byteOrder);
        }
        return rc;
    } // Thumbnail::read

    int Thumbnail::readJpegImage(const char* buf, const ExifData& exifData) 
    {
        std::string key = "Thumbnail.RecordingOffset.JPEGInterchangeFormat";
        ExifData::const_iterator pos = exifData.findKey(key);
        if (pos == exifData.end()) return 1;
        long offset = pos->toLong();
        key = "Thumbnail.RecordingOffset.JPEGInterchangeFormatLength";
        pos = exifData.findKey(key);
        if (pos == exifData.end()) return 1;
        long size = pos->toLong();
        image_ = std::string(buf + offset, size);
        type_ = JPEG;
        return 0;
    } // Thumbnail::readJpegImage

    int Thumbnail::readTiffImage(const char* buf,
                                 const ExifData& exifData,
                                 ByteOrder byteOrder)
    {
        char* data = new char[64*1024];     // temporary buffer Todo: handle larger
        memset(data, 0x0, 64*1024);         // images (which violate the Exif Std)
        long len = 0;                       // number of bytes in the buffer

        // Copy the TIFF header
        TiffHeader tiffHeader(byteOrder);
        len += tiffHeader.copy(data);

        // Create IFD (without Exif and GPS tags) from metadata
        Ifd ifd1(ifd1);
        ifd1.add(exifData.begin(), exifData.end(), tiffHeader.byteOrder());
        Ifd::iterator i = ifd1.findTag(0x8769);
        if (i != ifd1.end()) ifd1.erase(i);
        i = ifd1.findTag(0x8825);
        if (i != ifd1.end()) ifd1.erase(i);

        // Do not copy the IFD yet, remember the location and leave a gap
        long ifdOffset = len;
        len += ifd1.size() + ifd1.dataSize();

        // Copy thumbnail image data, remember the offsets used
        std::string key = "Thumbnail.RecordingOffset.StripOffsets";
        ExifData::const_iterator offsets = exifData.findKey(key);
        if (offsets == exifData.end()) return 2;
        key = "Thumbnail.RecordingOffset.StripByteCounts";
        ExifData::const_iterator sizes = exifData.findKey(key);
        if (sizes == exifData.end()) return 2;
        std::ostringstream os;                  // for the new strip offsets
        for (long k = 0; k < offsets->count(); ++k) {
            long offset = offsets->toLong(k);
            long size = sizes->toLong(k);
            memcpy(data + len, buf + offset, size);
            os << len << " ";
            len += size;
        }

        // Update the IFD with the actual strip offsets (replace existing entry)
        Metadatum newOffsets(*offsets);
        newOffsets.setValue(os.str());
        ifd1.add(newOffsets, tiffHeader.byteOrder());

        // Finally, sort and copy the IFD
        ifd1.sortByTag();
        ifd1.copy(data + ifdOffset, tiffHeader.byteOrder(), ifdOffset);

        image_ = std::string(data, len);
        delete[] data;
        type_ = TIFF;

        return 0;
    } // Thumbnail::readTiffImage

    int Thumbnail::write(const std::string& path) const
    {
        std::string p;
        switch (type_) {
        case JPEG: 
            p = path + ".jpg";
            break;
        case TIFF:
            p = path + ".tif";
            break;
        }
        std::ofstream file(p.c_str(), std::ios::binary | std::ios::out);
        if (!file) return 1;
        file.write(image_.data(), image_.size());
        if (!file.good()) return 2;
        return 0;
    } // Thumbnail::write

    void Thumbnail::update(ExifData& exifData) const
    {
        // Todo: properly synchronize the Exif data with the actual thumbnail,
        //       i.e., synch all relevant metadata

        switch (type_) {
        case JPEG: 
            updateJpegImage(exifData);
            break;
        case TIFF:
            updateTiffImage(exifData);
            break;
        }

    } // Thumbnail::update

    void Thumbnail::updateJpegImage(ExifData& exifData) const
    {
        std::string key = "Thumbnail.RecordingOffset.JPEGInterchangeFormat";
        ExifData::iterator pos = exifData.findKey(key);
        if (pos == exifData.end()) {
            Value *value = Value::create(unsignedLong);
            exifData.add(key, value);
            delete value;
            pos = exifData.findKey(key);
        }
        pos->setValue("0");

        key = "Thumbnail.RecordingOffset.JPEGInterchangeFormatLength";
        pos = exifData.findKey(key);
        if (pos == exifData.end()) {
            Value *value = Value::create(unsignedLong);
            exifData.add(key, value);
            delete value;            
            pos = exifData.findKey(key);
        }
        std::ostringstream os;
        os << image_.size();
        pos->setValue(os.str());

    } // Thumbnail::updateJpegImage

    void Thumbnail::updateTiffImage(ExifData& exifData) const
    {
        TiffHeader tiffHeader;
        tiffHeader.read(image_.data());
        long offset = tiffHeader.offset();
        Ifd ifd1(ifd1);
        ifd1.read(image_.data() + offset, tiffHeader.byteOrder(), offset);

        // Create metadata from the StripOffsets and StripByteCounts entries
        // and add these to the Exif data, replacing existing entries
        Ifd::const_iterator pos = ifd1.findTag(0x0111);
        if (pos == ifd1.end()) throw Error("Bad thumbnail (0x0111)");
        exifData.add(Metadatum(*pos, tiffHeader.byteOrder()));

        pos = ifd1.findTag(0x0117);
        if (pos == ifd1.end()) throw Error("Bad thumbnail (0x0117)");
        exifData.add(Metadatum(*pos, tiffHeader.byteOrder()));

    } // Thumbnail::updateTiffImage

    long Thumbnail::copy(char* buf) const
    {
        long ret = 0;
        switch (type_) {
        case JPEG: 
            ret = copyJpegImage(buf);
            break;
        case TIFF:
            ret = copyTiffImage(buf);
            break;
        }
        return ret;
    }

    long Thumbnail::copyJpegImage(char* buf) const
    {
        memcpy(buf, image_.data(), image_.size());
        return image_.size();
    }

    long Thumbnail::copyTiffImage(char* buf) const
    {
        // Read the TIFF header and IFD from the thumbnail image
        TiffHeader tiffHeader;
        tiffHeader.read(image_.data());
        long offset = tiffHeader.offset();
        Ifd thumbIfd(ifd1);
        thumbIfd.read(image_.data() + offset, tiffHeader.byteOrder(), offset);


        offset = thumbIfd.offset() + thumbIfd.size() + thumbIfd.dataSize();
        long size = image_.size() - offset;
        memcpy(buf, image_.data() + offset, size);
        return size;
    }

    void Thumbnail::setOffsets(Ifd& ifd1, ByteOrder byteOrder) const
    {
        switch (type_) {
        case JPEG: 
            setJpegImageOffsets(ifd1, byteOrder);
            break;
        case TIFF:
            setTiffImageOffsets(ifd1, byteOrder);
            break;
        }        
    }

    void Thumbnail::setJpegImageOffsets(Ifd& ifd1, ByteOrder byteOrder) const
    {
        Ifd::iterator pos = ifd1.findTag(0x0201);
        if (pos == ifd1.end()) throw Error("Bad thumbnail (0x0201)");
        pos->setOffset(ifd1.offset() + ifd1.size() + ifd1.dataSize(), byteOrder);
    }

    void Thumbnail::setTiffImageOffsets(Ifd& ifd1, ByteOrder byteOrder) const
    {
        // Read the TIFF header and IFD from the thumbnail image
        TiffHeader tiffHeader;
        tiffHeader.read(image_.data());
        long offset = tiffHeader.offset();
        Ifd thumbIfd(ifd1);
        thumbIfd.read(image_.data() + offset, tiffHeader.byteOrder(), offset);

        // Adjust the StripOffsets, assuming that the existing TIFF strips
        // start immediately after the thumbnail IFD
        long shift = ifd1.offset() + ifd1.size() + ifd1.dataSize() 
            - thumbIfd.offset() - thumbIfd.size() - thumbIfd.dataSize();
        Ifd::const_iterator pos = thumbIfd.findTag(0x0111);
        if (pos == thumbIfd.end()) throw Error("Bad thumbnail (0x0111)");
        Metadatum offsets(*pos, tiffHeader.byteOrder());
        std::ostringstream os;
        for (long k = 0; k < offsets.count(); ++k) {
            os << offsets.toLong(k) + shift << " ";
        }
        offsets.setValue(os.str());

        // Write the offsets to IFD1, encoded in the corresponding byte order
        ifd1.add(offsets, byteOrder);

    } // Thumbnail::setTiffImageOffsets

    ExifData::ExifData() 
        : ifd0_(ifd0, false), exifIfd_(exifIfd, false), iopIfd_(iopIfd, false), 
          gpsIfd_(gpsIfd, false), ifd1_(ifd1, false), valid_(false), 
          size_(0), data_(0)
    {
    }

    ExifData::~ExifData()
    {
        delete[] data_;
    }

    int ExifData::read(const std::string& path)
    {
        JpegImage img;
        int rc = img.readExifData(path);
        if (rc) return rc;
        return read(img.exifData(), img.sizeExifData());
    }

    int ExifData::read(const char* buf, long len)
    {
        // Copy the data buffer
        delete[] data_;
        data_ = new char[len];
        memcpy(data_, buf, len);
        size_ = len;
        valid_ = true;

        // Read the TIFF header
        int ret = 0;
        int rc = tiffHeader_.read(data_);
        if (rc) return rc;

        // Read IFD0
        rc = ifd0_.read(data_ + tiffHeader_.offset(), 
                        byteOrder(), 
                        tiffHeader_.offset());
        if (rc) return rc;
        // Find and read ExifIFD sub-IFD of IFD0
        rc = ifd0_.readSubIfd(exifIfd_, data_, byteOrder(), 0x8769);
        if (rc) return rc;
        // Find and read Interoperability IFD in ExifIFD
        rc = exifIfd_.readSubIfd(iopIfd_, data_, byteOrder(), 0xa005);
        if (rc) return rc;
        // Find and read GPSInfo sub-IFD in IFD0
        rc = ifd0_.readSubIfd(gpsIfd_, data_, byteOrder(), 0x8825);
        if (rc) return rc;
        // Read IFD1
        if (ifd0_.next()) {
            rc = ifd1_.read(data_ + ifd0_.next(), byteOrder(), ifd0_.next());
            if (rc) return rc;
        }

        // Find and delete ExifIFD sub-IFD of IFD1
        Ifd::iterator pos = ifd1_.findTag(0x8769);
        if (pos != ifd1_.end()) {
            ifd1_.erase(pos);
            ret = -99;
        }
        // Find and delete GPSInfo sub-IFD in IFD1
        pos = ifd1_.findTag(0x8825);
        if (pos != ifd1_.end()) {
            ifd1_.erase(pos);
            ret = -99;
        }

        // Copy all entries from the IFDs to the internal metadata
        metadata_.clear();
        add(ifd0_.begin(), ifd0_.end(), byteOrder());
        add(exifIfd_.begin(), exifIfd_.end(), byteOrder());
        add(iopIfd_.begin(), iopIfd_.end(), byteOrder()); 
        add(gpsIfd_.begin(), gpsIfd_.end(), byteOrder());
        add(ifd1_.begin(), ifd1_.end(), byteOrder());

        // Read the thumbnail
        thumbnail_.read(data_, *this, byteOrder());

        return ret;
    } // ExifData::read

    long ExifData::copy(char* buf)
    {
        // Todo: Check if the internal IFDs, i.e., the data buffer, are valid;
        //       if they are, update the IFDs and use the buffer

        // Else do it the hard way...

        // Copy the TIFF header
        long ifd0Offset = tiffHeader_.copy(buf);

        // Build IFD0
        Ifd ifd0(ifd0, ifd0Offset);
        ifd0.add(begin(), end(), byteOrder());

        // Build Exif IFD from metadata
        long exifIfdOffset = ifd0Offset + ifd0.size() + ifd0.dataSize();
        Ifd exifIfd(exifIfd, exifIfdOffset);
        exifIfd.add(begin(), end(), byteOrder());

        // Set the offset to the Exif IFD in IFD0
        ifd0.erase(0x8769);
        if (exifIfd.size() > 0) {
            ifd0.setOffset(0x8769, exifIfdOffset, byteOrder());
        }

        // Build Interoperability IFD from metadata
        long iopIfdOffset = exifIfdOffset + exifIfd.size() + exifIfd.dataSize();
        Ifd iopIfd(iopIfd, iopIfdOffset);
        iopIfd.add(begin(), end(), byteOrder());

        // Set the offset to the Interoperability IFD in Exif IFD
        exifIfd.erase(0xa005);
        if (iopIfd.size() > 0) {
            exifIfd.setOffset(0xa005, iopIfdOffset, byteOrder());
        }

        // Build GPSInfo IFD from metadata
        long gpsIfdOffset = iopIfdOffset + iopIfd.size() + iopIfd.dataSize();
        Ifd gpsIfd(gpsIfd, gpsIfdOffset);
        gpsIfd.add(begin(), end(), byteOrder());

        // Set the offset to the GPSInfo IFD in IFD0
        ifd0.erase(0x8825);
        if (gpsIfd.size() > 0) {
            ifd0.setOffset(0x8825, gpsIfdOffset, byteOrder());
        }

        // Update Exif data from thumbnail, build IFD1 from updated metadata
        thumbnail_.update(*this);
        long ifd1Offset = gpsIfdOffset + gpsIfd.size() + gpsIfd.dataSize();
        Ifd ifd1(ifd1, ifd1Offset);
        ifd1.add(begin(), end(), byteOrder());
        thumbnail_.setOffsets(ifd1, byteOrder());
        long thumbOffset = ifd1Offset + ifd1.size() + ifd1.dataSize();

        // Set the offset to IFD1 in IFD0
        if (ifd1.size() > 0) {
            ifd0.setNext(ifd1Offset);
        }

        // Copy all IFDs and the thumbnail image to the data buffer
        ifd0.sortByTag();
        ifd0.copy(buf + ifd0Offset, byteOrder(), ifd0Offset);
        exifIfd.sortByTag();
        exifIfd.copy(buf + exifIfdOffset, byteOrder(), exifIfdOffset);
        iopIfd.sortByTag();
        iopIfd.copy(buf + iopIfdOffset, byteOrder(), iopIfdOffset);
        gpsIfd.sortByTag();
        gpsIfd.copy(buf + gpsIfdOffset, byteOrder(), gpsIfdOffset);
        ifd1.sortByTag();
        ifd1.copy(buf + ifd1Offset, byteOrder(), ifd1Offset);
        long len = thumbnail_.copy(buf + thumbOffset);

        return len + thumbOffset;

    } // ExifData::copy

    long ExifData::size() const
    {
        // Todo: implement me!
        return 0;
    }

    void ExifData::add(Ifd::const_iterator begin, 
                       Ifd::const_iterator end,
                       ByteOrder byteOrder)
    {
        Ifd::const_iterator i = begin;
        for (; i != end; ++i) {
            add(Metadatum(*i, byteOrder));
        }
    }

    void ExifData::add(const std::string& key, Value* value)
    {
        add(Metadatum(key, value));
    }

    void ExifData::add(const Metadatum& metadatum)
    {
        iterator i = findKey(metadatum.key());
        if (i != end()) {
            i->setValue(&metadatum.value());
        }
        else {
            metadata_.push_back(metadatum);
        }
    }

    ExifData::const_iterator ExifData::findKey(const std::string& key) const
    {
        return std::find_if(metadata_.begin(), metadata_.end(),
                            FindMetadatumByKey(key));
    }

    ExifData::iterator ExifData::findKey(const std::string& key)
    {
        return std::find_if(metadata_.begin(), metadata_.end(),
                            FindMetadatumByKey(key));
    }

    void ExifData::erase(const std::string& key)
    {
        iterator pos = findKey(key);
        if (pos != end()) erase(pos);
    }

    void ExifData::erase(ExifData::iterator pos)
    {
        metadata_.erase(pos);
    }

    // *************************************************************************
    // free functions

    uint16 getUShort(const char* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return (unsigned char)buf[1] << 8 | (unsigned char)buf[0];
        }
        else {
            return (unsigned char)buf[0] << 8 | (unsigned char)buf[1];
        }
    }

    uint32 getULong(const char* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return   (unsigned char)buf[3] << 24 | (unsigned char)buf[2] << 16 
                   | (unsigned char)buf[1] <<  8 | (unsigned char)buf[0];
        }
        else {
            return   (unsigned char)buf[0] << 24 | (unsigned char)buf[1] << 16 
                   | (unsigned char)buf[2] <<  8 | (unsigned char)buf[3];
        }
    }

    URational getURational(const char* buf, ByteOrder byteOrder)
    {
        uint32 nominator = getULong(buf, byteOrder);
        uint32 denominator = getULong(buf + 4, byteOrder);
        return std::make_pair(nominator, denominator);
    }

    int16 getShort(const char* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return (unsigned char)buf[1] << 8 | (unsigned char)buf[0];
        }
        else {
            return (unsigned char)buf[0] << 8 | (unsigned char)buf[1];
        }
    }

    int32 getLong(const char* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return   (unsigned char)buf[3] << 24 | (unsigned char)buf[2] << 16 
                   | (unsigned char)buf[1] <<  8 | (unsigned char)buf[0];
        }
        else {
            return   (unsigned char)buf[0] << 24 | (unsigned char)buf[1] << 16 
                   | (unsigned char)buf[2] <<  8 | (unsigned char)buf[3];
        }
    }

    Rational getRational(const char* buf, ByteOrder byteOrder)
    {
        int32 nominator = getLong(buf, byteOrder);
        int32 denominator = getLong(buf + 4, byteOrder);
        return std::make_pair(nominator, denominator);
    }

    long us2Data(char* buf, uint16 s, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  s & 0x00ff;
            buf[1] = (s & 0xff00) >> 8;
        }
        else {
            buf[0] = (s & 0xff00) >> 8;
            buf[1] =  s & 0x00ff;
        }
        return 2;
    }

    long ul2Data(char* buf, uint32 l, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  l & 0x000000ff;
            buf[1] = (l & 0x0000ff00) >> 8;
            buf[2] = (l & 0x00ff0000) >> 16;
            buf[3] = (l & 0xff000000) >> 24;
        }
        else {
            buf[0] = (l & 0xff000000) >> 24;
            buf[1] = (l & 0x00ff0000) >> 16;
            buf[2] = (l & 0x0000ff00) >> 8;
            buf[3] =  l & 0x000000ff;
        }
        return 4;
    }

    long ur2Data(char* buf, URational l, ByteOrder byteOrder)
    {
        long o = ul2Data(buf, l.first, byteOrder);
        o += ul2Data(buf+o, l.second, byteOrder);
        return o;
    }

    long s2Data(char* buf, int16 s, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  s & 0x00ff;
            buf[1] = (s & 0xff00) >> 8;
        }
        else {
            buf[0] = (s & 0xff00) >> 8;
            buf[1] =  s & 0x00ff;
        }
        return 2;
    }

    long l2Data(char* buf, int32 l, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  l & 0x000000ff;
            buf[1] = (l & 0x0000ff00) >> 8;
            buf[2] = (l & 0x00ff0000) >> 16;
            buf[3] = (l & 0xff000000) >> 24;
        }
        else {
            buf[0] = (l & 0xff000000) >> 24;
            buf[1] = (l & 0x00ff0000) >> 16;
            buf[2] = (l & 0x0000ff00) >> 8;
            buf[3] =  l & 0x000000ff;
        }
        return 4;
    }

    long r2Data(char* buf, Rational l, ByteOrder byteOrder)
    {
        long o = l2Data(buf, l.first, byteOrder);
        o += l2Data(buf+o, l.second, byteOrder);
        return o;
    }

    void hexdump(std::ostream& os, const char* buf, long len)
    {
        const std::string::size_type pos = 9 + 16 * 3; 
        const std::string align(pos, ' '); 

        long i = 0;
        while (i < len) {
            os << "   " 
               << std::setw(4) << std::setfill('0') << std::hex 
               << i << "  ";
            std::ostringstream ss;
            do {
                unsigned char c = buf[i];
                os << std::setw(2) << std::setfill('0') 
                   << std::hex << (int)c << " ";
                ss << ((int)c >= 31 && (int)c < 127 ? buf[i] : '.');
            } while (++i < len && i%16 != 0);
            std::string::size_type width = 9 + ((i-1)%16 + 1) * 3;
            os << (width > pos ? "" : align.substr(width)) << ss.str() << "\n";
        }
        os << std::dec << std::setfill(' ');
    } // hexdump

    bool cmpOffset(const RawEntry& lhs, const RawEntry& rhs)
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

    bool cmpTag(const Entry& lhs, const Entry& rhs)
    {
        return lhs.tag() < rhs.tag();
    }

}                                       // namespace Exif
