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
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:
   13-Jan-04, ahu: created

  RCS information
   $Name:  $
   $Revision: 1.6 $
 */
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

#include <cstring>

// *****************************************************************************
// local declarations
namespace {

    // Compare two IFD entries by offset, taking care of special cases
    // where one or both of the entries don't have an offset.
    bool cmpOffset(const Exif::Ifd::Entry& lhs, const Exif::Ifd::Entry& rhs);

}

// *****************************************************************************
// class member definitions
namespace Exif {

    JpegImage::JpegImage()
        : sizeExifData_(0), offsetExifData_(0), exifData_(0)
    {
    }

    JpegImage::~JpegImage()
    {
        delete exifData_;
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

    TiffHeader::TiffHeader() 
        : byteOrder_(littleEndian), tag_(0x002a), offset_(0x00000008)
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
        }
        return value;
    } // Value::create

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

    Metadatum::Metadatum()
        : tag_(0), type_(0), ifdId_(IfdIdNotSet), ifdIdx_(-1), value_(0)
    {
    }

    Metadatum::Metadatum(uint16 tag, uint16 type, 
                         IfdId ifdId, int ifdIdx, Value* value)
        : tag_(tag), type_(type), ifdId_(ifdId), ifdIdx_(ifdIdx), value_(value)
    {
        key_ = std::string(ifdItem()) 
            + "." + std::string(sectionName()) 
            + "." + std::string(tagName());
    }

    Metadatum::~Metadatum()
    {
        delete value_;
    }

    Metadatum::Metadatum(const Metadatum& rhs)
    {
        tag_ = rhs.tag_;
        type_ = rhs.type_;
        ifdId_ = rhs.ifdId_;
        ifdIdx_ = rhs.ifdIdx_;
        value_ = 0;
        if (rhs.value_ != 0) value_ = rhs.value_->clone(); // deep copy
        key_ = rhs.key_;
    }

    Metadatum& Metadatum::operator=(const Metadatum& rhs)
    {
        if (this == &rhs) return *this;
        tag_ = rhs.tag_;
        type_ = rhs.type_;
        ifdId_ = rhs.ifdId_;
        ifdIdx_ = rhs.ifdIdx_;
        delete value_;
        value_ = 0;
        if (rhs.value_ != 0) value_ = rhs.value_->clone(); // deep copy
        key_ = rhs.key_;
        return *this;
    } // Metadatum::operator=
    
    void Metadatum::setValue(Value* value)
    {
        delete value_;
        value_ = value;
    }

    Ifd::Entry::Entry() 
        : ifdIdx_(-1), tag_(0), type_(0), count_(0), offset_(0),
          data_(0), size_(0)
    {
    }

    Ifd::Entry::~Entry()
    {
        delete[] data_;
    }

    Ifd::Entry::Entry(const Entry& rhs)
    {
        ifdIdx_ = rhs.ifdIdx_;
        tag_ = rhs.tag_;
        type_ = rhs.type_;
        count_ = rhs.count_;
        offset_ = rhs.offset_;
        data_ = 0;
        if (rhs.data_) {
            data_ = new char[rhs.size_];
            ::memcpy(data_, rhs.data_, rhs.size_);
        }
        size_ = rhs.size_;
    }

    Ifd::Entry::Entry& Ifd::Entry::operator=(const Entry& rhs)
    {
        if (this == &rhs) return *this;
        ifdIdx_ = rhs.ifdIdx_;
        tag_ = rhs.tag_;
        type_ = rhs.type_;
        count_ = rhs.count_;
        offset_ = rhs.offset_;
        delete data_;
        data_ = 0;
        if (rhs.data_) {
            data_ = new char[rhs.size_];
            ::memcpy(data_, rhs.data_, rhs.size_);
        }
        size_ = rhs.size_;
        return *this;
    }

    Ifd::Ifd(IfdId ifdId)
        : ifdId_(ifdId), offset_(0), next_(0), size_(0)
    {
    }

    int Ifd::read(const char* buf, ByteOrder byteOrder, long offset)
    {
        offset_ = offset;
        int n = getUShort(buf, byteOrder);
        long o = 2;

        entries_.clear();
        for (int i=0; i<n; ++i) {
            Entry e;
            e.ifdIdx_ = i;
            e.tag_ = getUShort(buf+o, byteOrder);
            e.type_ = getUShort(buf+o+2, byteOrder);
            e.count_ = getULong(buf+o+4, byteOrder);
            // offset will be converted to a relative offset below
            e.offset_ = getULong(buf+o+8, byteOrder); 
            // data_ is set later, see below
            e.size_ = e.count_ * e.typeSize();
            entries_.push_back(e);
            o += 12;
        }
        next_ = getULong(buf+o, byteOrder);
        size_ = 2 + 12 * entries_.size() + 4;

        // Guess the offset if it was not given. The guess is based 
        // on the assumption that the smallest offset points to a data 
        // buffer directly following the IFD.
        // Subsequently all offsets of IFD entries need to be recalculated.
        const Entries::iterator eb = entries_.begin();
        const Entries::iterator ee = entries_.end();
        Entries::iterator i = eb;
        if (offset_ == 0 && i != ee) {
            // Find the entry with the smallest offset
            i = std::min_element(eb, ee, cmpOffset);
            // Set the guessed IFD offset
            if (i->size_ > 4) {
                offset_ = i->offset_ - size_;
            }
        }

        // Assign the values to each IFD entry and
        // calculate offsets relative to the start of the IFD
        for (i = eb; i != ee; ++i) {
            delete[] i->data_;
            if (i->size_ > 4) {
                i->offset_ = i->offset_ - offset_;
                i->data_ = new char[i->size_];
                ::memcpy(i->data_, buf + i->offset_, i->size_);
            }
            else {
                i->data_ = new char[4];
                ul2Data(i->data_, i->offset_, byteOrder);
            }
        }

        return 0;
    } // Ifd::read

    Ifd::Entries::const_iterator Ifd::findTag(uint16 tag) const 
    {
        return std::find_if(entries_.begin(), entries_.end(),
                            FindEntryByTag(tag));
    }

    int Ifd::readSubIfd(
        Ifd& dest, const char* buf, ByteOrder byteOrder, uint16 tag
    ) const
    {
        int rc = 0;
        Entries::const_iterator pos = findTag(tag);
        if (pos != entries_.end()) {
            rc = dest.read(buf + pos->offset_, byteOrder, pos->offset_);
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
        const Entries::const_iterator b = entries_.begin();
        const Entries::const_iterator e = entries_.end();
        Entries::const_iterator i = b;
        for (; i != e; ++i) {
            us2Data(buf+o, i->tag_, byteOrder);
            us2Data(buf+o+2, i->type_, byteOrder);
            ul2Data(buf+o+4, i->count_, byteOrder);
            if (i->size_ > 4) {
                ul2Data(buf+o+8, offset + size_ + dataSize, byteOrder);
                dataSize += i->size_;
            }
            else {
                ::memcpy(buf+o+8, i->data_, 4);
            }
            o += 12;
        }

        // Add the offset to the next IFD to the data buffer pointing
        // directly behind this IFD and its data
        if (next_ != 0) {
            ul2Data(buf+o, offset + size_ + dataSize, byteOrder);
        }
        else {
            ul2Data(buf+o, 0, byteOrder);
        }
        o += 4;

        // Add the data of all IFD entries to the data buffer
        for (i = b; i != e; ++i) {
            if (i->size_ > 4) {
                ::memcpy(buf + o, i->data_, i->size_);
                o += i->size_;
            }
        }

        return o;
    } // Ifd::data

    void Ifd::print(std::ostream& os, const std::string& prefix) const
    {
        if (entries_.size() == 0) return;

        os << prefix << "IFD Offset: 0x"
           << std::setw(8) << std::setfill('0') << std::hex << std::right 
           << offset_ 
           << ",   IFD Entries: " 
           << std::setfill(' ') << std::dec << std::right
           << entries_.size() << "\n"
           << prefix << "Entry     Tag  Format   (Bytes each)  Number  Offset\n"
           << prefix << "-----  ------  ---------------------  ------  -----------\n";

        const Entries::const_iterator b = entries_.begin();
        const Entries::const_iterator e = entries_.end();
        Entries::const_iterator i = b;
        for (; i != e; ++i) {
            std::ostringstream offset;
            if (i->size_ <= 4) {
                offset << std::setw(2) << std::setfill('0') << std::hex
                       << (int)*(unsigned char*)i->data_ << " "
                       << std::setw(2) << std::setfill('0') << std::hex
                       << (int)*(unsigned char*)(i->data_+1) << " "
                       << std::setw(2) << std::setfill('0') << std::hex
                       << (int)*(unsigned char*)(i->data_+2) << " "
                       << std::setw(2) << std::setfill('0') << std::hex
                       << (int)*(unsigned char*)(i->data_+3) << " ";
            }
            else {
                offset << " 0x" << std::setw(8) << std::setfill('0') << std::hex
                       << std::right << i->offset_;
            }

            os << prefix << std::setw(5) << std::setfill(' ') << std::dec
               << std::right << i - b
               << "  0x" << std::setw(4) << std::setfill('0') << std::hex 
               << std::right << i->tag_ 
               << "  " << std::setw(17) << std::setfill(' ') 
               << std::left << i->typeName() 
               << " (" << std::dec << i->typeSize() << ")"
               << "  " << std::setw(6) << std::setfill(' ') << std::dec
               << std::right << i->count_
               << "  " << offset.str()
               << "\n";
        }
        os << prefix << "Next IFD: 0x" 
           << std::setw(8) << std::setfill('0') << std::hex
           << std::right << next_ << "\n";

        for (i = b; i != e; ++i) {
            if (i->size_ > 4) {
                os << "Data of entry " << i-b << ":\n";
                hexdump(os, i->data_, i->size_);
            }
        }

    } // Ifd::print

    // Todo: implement this properly..
    //       - Tag values 0x0201 and 0x0202 may be long OR short types...
    //       - TIFF thumbnails
    // Rewrite: it should use the higher level Metadata interface
    int Thumbnail::read(const char* buf, const Ifd& ifd1, ByteOrder byteOrder)
    {
//         Ifd::Entries::const_iterator pos = ifd1.findTag(0x0103);
//         if (pos == ifd1.entries().end()) return 1;
//         const UShortValue& compression = dynamic_cast<const UShortValue&>(pos->value());
//         if (compression.value() == 6) {
//             pos = ifd1.findTag(0x0201);
//             if (pos == ifd1.entries().end()) return 2;
//             const ULongValue& offset = dynamic_cast<const ULongValue&>(pos->value());
//             pos = ifd1.findTag(0x0202);
//             if (pos == ifd1.entries().end()) return 3;
//             const ULongValue& size = dynamic_cast<const ULongValue&>(pos->value());

//             thumbnail_ = std::string(buf + offset.value(), size.value());
//         }
//         else if (compression.value() == 1) {
//             // Todo: to be continued...
//             return 4;
//         }
//         else {
//             // invalid compression value
//             return 5;
//         }
        return 0;
    }

    int Thumbnail::write(const std::string& path) const
    {
        std::ofstream file(path.c_str(), std::ios::binary | std::ios::out);
        if (!file) return 1;
        file.write(thumbnail_.data(), thumbnail_.size());
        if (!file.good()) return 2;
        return 0;
    }

    int ExifData::read(const std::string& path)
    {
        JpegImage img;
        int rc = img.readExifData(path);
        if (rc) return rc;
        offset_ = img.offsetExifData();
        return read(img.exifData(), img.sizeExifData());
    }

    int ExifData::read(const char* buf, long len)
    {
        int rc = tiffHeader_.read(buf);
        if (rc) return rc;

        // Read IFD0
        Ifd ifd0(ifd0);
        rc = ifd0.read(buf + tiffHeader_.offset(), 
                       byteOrder(), 
                       tiffHeader_.offset());
        if (rc) return rc;

        // Find and read ExifIFD sub-IFD of IFD0
        Ifd exifIfd(exifIfd);
        rc = ifd0.readSubIfd(exifIfd, buf, byteOrder(), 0x8769);
        if (rc) return rc;

        // Find and read Interoperability IFD in ExifIFD
        Ifd iopIfd(iopIfd);
        rc = exifIfd.readSubIfd(iopIfd, buf, byteOrder(), 0xa005);
        if (rc) return rc;

        // Find and read GPSInfo sub-IFD in IFD0
        Ifd gpsIfd(gpsIfd);
        rc = ifd0.readSubIfd(gpsIfd, buf, byteOrder(), 0x8825);
        if (rc) return rc;

        // Read IFD1
        Ifd ifd1(ifd1);
        if (ifd0.next()) {
            rc = ifd1.read(buf + ifd0.next(), byteOrder(), ifd0.next());
            if (rc) return rc;
        }

        // Find and read ExifIFD sub-IFD of IFD1
        Ifd ifd1ExifIfd(ifd1ExifIfd);
        rc = ifd1.readSubIfd(ifd1ExifIfd, buf, byteOrder(), 0x8769);
        if (rc) return rc;

        // Find and read Interoperability IFD in ExifIFD of IFD1
        Ifd ifd1IopIfd(ifd1IopIfd);
        rc = ifd1ExifIfd.readSubIfd(ifd1IopIfd, buf, byteOrder(), 0xa005);
        if (rc) return rc;

        // Find and read GPSInfo sub-IFD in IFD1
        Ifd ifd1GpsIfd(ifd1GpsIfd);
        rc = ifd1.readSubIfd(ifd1GpsIfd, buf, byteOrder(), 0x8825);
        if (rc) return rc;

        // Copy all entries from the IFDs to the internal metadata
        metadata_.clear();
        add(ifd0, byteOrder());
        add(exifIfd, byteOrder());
        add(iopIfd, byteOrder()); 
        add(gpsIfd, byteOrder());
        add(ifd1, byteOrder());
        add(ifd1ExifIfd, byteOrder());
        add(ifd1IopIfd, byteOrder());
        add(ifd1GpsIfd, byteOrder());

        // Read the thumbnail
        thumbnail_.read(buf, ifd1, byteOrder());

        return 0;
    } // ExifData::read

    long ExifData::copy(char* buf) const
    {
        // Todo: implement me!
        return 0;
    }

    long ExifData::size() const
    {
        // Todo: implement me!
        return 0;
    }

    void ExifData::add(const Ifd& ifd, ByteOrder byteOrder)
    {
        Ifd::const_iterator i = ifd.begin();
        Ifd::const_iterator end = ifd.end(); 
        for (; i != end; ++i) {
            Value* value = Value::create(TypeId(i->type_));
            value->read(i->data_, i->size_, byteOrder);
            Metadatum md(i->tag_, i->type_, ifd.ifdId(), i->ifdIdx_, value);
            add(md);
        }
    }

    void ExifData::add(const Metadatum& src)
    {
        metadata_.push_back(src);
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
    }

}                                       // namespace Exif

// *****************************************************************************
// local definitions
namespace {

    bool cmpOffset(const Exif::Ifd::Entry& lhs, const Exif::Ifd::Entry& rhs)
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

}
