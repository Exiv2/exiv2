// ***************************************************************** -*- C++ -*-
/*
 * Copyright (c) 2004 Andreas Huggel
 * All rights reserved. This program and the accompanying materials 
 * are made available under the terms of the Common Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.ibm.com/developerworks/oss/CPLv1.0.htm
 */
/*
  Author(s): Andreas Huggel (ahu)
  History:
   13-Jan-04, ahu: created

  RCS information
   $Name:  $
   $Revision: 1.1 $
 */
// *****************************************************************************
// included header files
#include "exif.h"

// + PMT includes

// + standard includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include <cstring>

// *****************************************************************************
// local declarations
namespace {

    // Compare two IFD entries by offset, taking care of special cases
    // where one or both of the entries don't have an offset.
    bool cmpOffset(const Exif::Metadatum& lhs, const Exif::Metadatum& rhs);

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
        char buf[8];
        ::memset(buf, 0x0, 8);
        is.read(buf, 8);
        if (!is.good()) return 1;
        // Get the length of the APP1 field and do a plausibility check
        long app1Length = getUShort(buf, bigEndian);
        if (app1Length < 8) return 4;
        // Check the Exif identifier
        if (::memcmp(buf+2, exifId_, 6) != 0) return 4;
 
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

    void TiffHeader::data(char* buf) const
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
    }

    Format::Format(uint16 type, const std::string& name, long size)
        : type_(type), name_(name), size_(size)
    {
    }

    //! Lookup list of IFD tag data formats and their properties
    static const Format tagDataFormat[] = {
        Format( 0, "invalid",           0),
        Format( 1, "unsigned byte",     1),
        Format( 2, "ascii strings",     1),
        Format( 3, "unsigned short",    2),
        Format( 4, "unsigned long",     4),
        Format( 5, "unsigned rational", 8),
        Format( 6, "signed byte",       1),
        Format( 7, "undefined",         1),
        Format( 8, "signed short",      2),
        Format( 9, "signed long",       4),
        Format(10, "signed rational",   8),
        Format(11, "single float",      4),
        Format(12, "double float",      8)
    };

    TagInfo::TagInfo(
        uint16 tag, 
        const std::string& fieldName, 
        const std::string& tagName, 
        IfdId ifdId, 
        TagSection tagSection
    )
        : tag_(tag), fieldName_(fieldName), tagName_(tagName), 
          ifdId_(ifdId), tagSection_(tagSection)
    {
    }

    //! Lookup list with tags, their names and where they belong to
    static const TagInfo tagInfo[] = {
        TagInfo(0x0100, "ImageWidth", "Image width", ifd0, ifd0Tiff),
        TagInfo(0x0101, "ImageLength", "Image height", ifd0, ifd0Tiff),
        TagInfo(0x0102, "BitsPerSample", "Number of bits per component", ifd0, ifd0Tiff),
        TagInfo(0x0103, "Compression", "Compression scheme", ifd0, ifd0Tiff),
        TagInfo(0x0106, "PhotometricInterpretation", "Pixel composition", ifd0, ifd0Tiff),
        TagInfo(0x010e, "ImageDescription", "Image title", ifd0, ifd0Tiff),
        TagInfo(0x010f, "Make", "Manufacturer of image input equipment", ifd0, ifd0Tiff),
        TagInfo(0x0110, "Model", "Model of image input equipment", ifd0, ifd0Tiff),
        TagInfo(0x0111, "StripOffsets", "Image data location", ifd0, ifd0Tiff),
        TagInfo(0x0112, "Orientation", "Orientation of image", ifd0, ifd0Tiff),
        TagInfo(0x0115, "SamplesPerPixel", "Number of components", ifd0, ifd0Tiff),
        TagInfo(0x0116, "RowsPerStrip", "Number of rows per strip", ifd0, ifd0Tiff),
        TagInfo(0x0117, "StripByteCounts", "Bytes per compressed strip", ifd0, ifd0Tiff),
        TagInfo(0x011a, "XResolution", "Image resolution in width direction", ifd0, ifd0Tiff),
        TagInfo(0x011b, "YResolution", "Image resolution in height direction", ifd0, ifd0Tiff),
        TagInfo(0x011c, "PlanarConfiguration", "Image data arrangement", ifd0, ifd0Tiff),
        TagInfo(0x0128, "ResolutionUnit", "Unit of X and Y resolution", ifd0, ifd0Tiff),
        TagInfo(0x012d, "TransferFunction", "Transfer function", ifd0, ifd0Tiff),
        TagInfo(0x0131, "Software", "Software used", ifd0, ifd0Tiff),
        TagInfo(0x0132, "DateTime", "File change date and time", ifd0, ifd0Tiff),
        TagInfo(0x013b, "Artist", "Person who created the image", ifd0, ifd0Tiff),
        TagInfo(0x013e, "WhitePoint", "White point chromaticity", ifd0, ifd0Tiff),
        TagInfo(0x013f, "PrimaryChromaticities", "Chromaticities of primaries", ifd0, ifd0Tiff),
        TagInfo(0x0201, "JPEGInterchangeFormat", "Offset to JPEG SOI", ifd0, ifd0Tiff),
        TagInfo(0x0202, "JPEGInterchangeFormatLength", "Bytes of JPEG data", ifd0, ifd0Tiff),
        TagInfo(0x0211, "YCbCrCoefficients", "Color space transformation matrix coefficients", ifd0, ifd0Tiff),
        TagInfo(0x0212, "YCbCrSubSampling", "Subsampling ratio of Y to C", ifd0, ifd0Tiff),
        TagInfo(0x0213, "YCbCrPositioning", "Y and C positioning", ifd0, ifd0Tiff),
        TagInfo(0x0214, "ReferenceBlackWhite", "Pair of black and white reference values", ifd0, ifd0Tiff),
        TagInfo(0x8298, "Copyright", "Copyright holder", ifd0, ifd0Tiff),
        TagInfo(0x8769, "ExifTag", "Exif IFD Pointer", ifd0, ifd0Tiff),
        TagInfo(0x8825, "GPSTag", "GPSInfo IFD Pointer", ifd0, ifd0Tiff)
    };

    Metadatum::Metadatum()
        : tag_(0), type_(0), count_(0), offset_(0), size_(0), 
          ifdId_(unknown), ifdIdx_(-1), data_(0)
    {
    }

    Metadatum::~Metadatum()
    {
        delete[] data_;
    }

    Metadatum::Metadatum(const Metadatum& rhs)
    {
        tag_ = rhs.tag_;
        type_ = rhs.type_;
        count_ = rhs.count_;
        offset_ = rhs.offset_;   
        size_ = rhs.size_;

        ifdId_ = rhs.ifdId_;
        ifdIdx_ = rhs.ifdIdx_;

        // deep copy
        data_ = 0;
        if (rhs.data_ != 0) {
            data_ = new char[rhs.size_];
            ::memcpy(data_, rhs.data_, rhs.size_);
        }
    }

    Metadatum& Metadatum::operator=(const Metadatum& rhs)
    {
        tag_ = rhs.tag_;
        type_ = rhs.type_;
        count_ = rhs.count_;
        offset_ = rhs.offset_;   
        size_ = rhs.size_;

        ifdId_ = rhs.ifdId_;
        ifdIdx_ = rhs.ifdIdx_;

        delete[] data_;
        if (rhs.data_ != 0) {
            data_ = new char[rhs.size_];
            ::memcpy(data_, rhs.data_, rhs.size_);
        }

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

        for (int i=0; i<n; ++i) {
            Metadatum e;
            e.ifdId_ = ifdId_;
            e.ifdIdx_ = i;
            e.tag_ = getUShort(buf+o, byteOrder);
            e.type_ = getUShort(buf+o+2, byteOrder);
            e.count_ = getULong(buf+o+4, byteOrder);
            // offset will be converted to a relative offset below
            e.offset_ = getULong(buf+o+8, byteOrder); 
            e.size_ = e.count_ * tagDataFormat[e.type_].size_;
            // data_ is set later, see below
            entries_.push_back(e);
            o += 12;
        }
        next_ = getULong(buf+o, byteOrder);
        size_ = 2 + 12 * entries_.size() + 4;

        // Guess the offset if it was not given. The guess is based 
        // on the assumption that the smallest offset points to a data 
        // buffer directly following the IFD.
        // Subsequently all offsets of IFD entries need to be recalculated.
        const Metadata::iterator eb = entries_.begin();
        const Metadata::iterator ee = entries_.end();
        Metadata::iterator i = eb;
        if (offset_ == 0 && i != ee) {
            // Find the entry with the smallest offset
            i = std::min_element(eb, ee, cmpOffset);
            // Set the guessed IFD offset
            if (i->size_ > 4) {
                offset_ = i->offset_ - size_;
            }
        }

        // Assign the data to each IFD entry and 
        // calculate offsets relative to the start of the IFD
        for (i = eb; i != ee; ++i) {
            delete[] i->data_;
            i->data_ = 0;
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

    int Ifd::readSubIfd(
        Ifd& dest, const char* buf, ByteOrder byteOrder, uint16 tag
    ) const
    {
        int rc = 0;
        Metadata::const_iterator pos;
        Metadata::const_iterator end = entries_.end();
        pos = std::find_if(entries_.begin(), end, matchTag(tag));
        if (pos != end) {
            rc = dest.read(buf + pos->offset_, byteOrder, pos->offset_);
        }
        return rc;
    } // Ifd::readSubIfd

    char* Ifd::data(char* buf, ByteOrder byteOrder, long offset) const
    {
        if (offset == 0) offset = offset_;

        // Add the number of entries to the data buffer
        us2Data(buf, entries_.size(), byteOrder);
        long o = 2;

        // Add all directory entries to the data buffer
        long dataSize = 0;
        const Metadata::const_iterator b = entries_.begin();
        const Metadata::const_iterator e = entries_.end();
        Metadata::const_iterator i = b;
        for (; i != e; ++i) {
            us2Data(buf+o, i->tag_, byteOrder);
            us2Data(buf+o+2, i->type_, byteOrder);
            ul2Data(buf+o+4, i->count_, byteOrder);
            if (i->size_ > 4) {
                ul2Data(buf+o+8, offset + size_ + dataSize, byteOrder);
                dataSize += i->size_;
            }
            else {
                ul2Data(buf+o+8, i->offset_, byteOrder);
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
                ::memcpy(buf+o, i->data_, i->size_);
                o += i->size_;
            }
        }

        return buf;
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
           << prefix << "Entry     Tag  Format   (Bytes each)  Number  Offset/Data\n"
           << prefix << "-----  ------  ---------------------  ------  -----------\n";

        const Metadata::const_iterator b = entries_.begin();
        const Metadata::const_iterator e = entries_.end();
        Metadata::const_iterator i = b;
        for (; i != e; ++i) {
            std::ostringstream offset;
            if (tagDataFormat[i->type_].size_ * i->count_ <= 4) {
                // Minor cheat here: we use data_ instead of offset_ to avoid
                // having to invoke ul2Data() which would require byte order.
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
               << std::left << tagDataFormat[i->type_].name_ 
               << " (" << std::dec << tagDataFormat[i->type_].size_ << ")"
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

        const ByteOrder byteOrder = tiffHeader_.byteOrder(); // shortcut

        // Read IFD0
        Ifd ifd0(ifd0);
        rc = ifd0.read(buf + tiffHeader_.offset(), byteOrder, tiffHeader_.offset());
        if (rc) return rc;

        // Find and read ExifIFD sub-IFD of IFD0
        Ifd exifIfd(exifIfd);
        rc = ifd0.readSubIfd(exifIfd, buf, byteOrder, 0x8769);
        if (rc) return rc;

        // Find and read Interoperability IFD in ExifIFD
        Ifd exifIopIfd(exifIopIfd);
        rc = exifIfd.readSubIfd(exifIopIfd, buf, byteOrder, 0xa005);
        if (rc) return rc;

        // Find and read GPSInfo sub-IFD in IFD0
        Ifd gpsIfd(gpsIfd);
        rc = ifd0.readSubIfd(gpsIfd, buf, byteOrder, 0x8825);
        if (rc) return rc;

        // Read IFD1
        Ifd ifd1(ifd1);
        if (ifd0.next()) {
            rc = ifd1.read(buf + ifd0.next(), byteOrder, ifd0.next());
            if (rc) return rc;
        }

        // Find and read Interoperability IFD in IFD1
        Ifd ifd1IopIfd(ifd1IopIfd);
        rc = ifd1.readSubIfd(ifd1IopIfd, buf, byteOrder, 0xa005);
        if (rc) return rc;

        // Finally, copy all metadata from the IFDs to the internal metadata
        metadata_.clear();
        add(ifd0.entries());
        add(exifIfd.entries());
        add(exifIopIfd.entries()); 
        add(gpsIfd.entries());
        add(ifd1.entries());
        add(ifd1IopIfd.entries());

        return 0;
    } // ExifData::read

    void ExifData::data(char* buf) const
    {
        // Todo: implement me!
    }

    long ExifData::size() const
    {
        // Todo: implement me!
        return 0;
    }

    void ExifData::add(const Metadata& src)
    {
        metadata_.insert(metadata_.end(), src.begin(), src.end());
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

    std::string getString(const char* buf, long len)
    {
        std::string txt(buf, len);
        return txt;
    }

    char* us2Data(char* buf, uint16 s, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  s & 0x00ff;
            buf[1] = (s & 0xff00) >> 8;
        }
        else {
            buf[0] = (s & 0xff00) >> 8;
            buf[1] =  s & 0x00ff;
        }
        return buf;
    }

    char* ul2Data(char* buf, uint32 l, ByteOrder byteOrder)
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
        return buf;
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

    bool cmpOffset(const Exif::Metadatum& lhs, const Exif::Metadatum& rhs)
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
