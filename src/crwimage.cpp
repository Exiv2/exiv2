// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2005 Andreas Huggel <ahuggel@gmx.net>
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
  File:      crwimage.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   28-Aug-05, ahu: created

 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// Define DEBUG to output debug information to std::cerr, e.g, by calling make
// like this: make DEFS=-DDEBUG crwimage.o
//#define DEBUG

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "crwimage.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "value.hpp"
#include "tags.hpp"
#include "canonmn.hpp"

// + standard includes
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const byte CrwImage::blank_[] = {
        0x00
    };

    CrwImage::CrwImage(BasicIo::AutoPtr io, bool create)
        : io_(io)
    {
        if (create) {
            initImage(blank_, sizeof(blank_));
        }
    } // CrwImage::CrwImage

    int CrwImage::initImage(const byte initData[], long dataSize)
    {
        if (io_->open() != 0) {
            return 4;
        }
        IoCloser closer(*io_);
        if (io_->write(initData, dataSize) != dataSize) {
            return 4;
        }
        return 0;
    } // CrwImage::initImage

    bool CrwImage::good() const
    {
        if (io_->open() != 0) return false;
        IoCloser closer(*io_);
        return isThisType(*io_, false);
    }

    void CrwImage::clearMetadata()
    {
        clearExifData();
        clearComment();
    }

    void CrwImage::setMetadata(const Image& image)
    {
        setExifData(image.exifData());
        setComment(image.comment());
    }

    void CrwImage::clearExifData()
    {
        exifData_.clear();
    }

    void CrwImage::setExifData(const ExifData& exifData)
    {
        exifData_ = exifData;
    }

    void CrwImage::clearIptcData()
    {
        throw Error(31, "CrwImage::clearIptcData");
    }

    void CrwImage::setIptcData(const IptcData& iptcData)
    {
        throw Error(31, "CrwImage::setIptcData");
    }

    void CrwImage::clearComment()
    {
        comment_.erase();
    }

    void CrwImage::setComment(const std::string& comment)
    {
        comment_ = comment;
    }

    void CrwImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading CRW file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isThisType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(33);
        }
        clearMetadata();

        // Read the image into a memory buffer
        long len = io_->size();
        DataBuf buf(len);
        io_->read(buf.pData_, len);
        if (io_->error() || io_->eof()) throw Error(14);

        CrwParser::decode(this, buf.pData_, buf.size_);
    } // CrwImage::readMetadata

    void CrwImage::writeMetadata()
    {
        // Todo: implement me!
    } // CrwImage::writeMetadata

    bool CrwImage::isThisType(BasicIo& iIo, bool advance) const
    {
        return isCrwType(iIo, advance);
    }

    void CrwParser::decode(CrwImage* crwImage, const byte* buf, uint32_t len)
    {
        assert(crwImage != 0);
        assert(buf != 0);

        // Parse the image
        RawMetadata::AutoPtr parseTree(new CiffHeader);
        parseTree->read(buf, len, 0, invalidByteOrder);
#ifdef DEBUG
        parseTree->print(std::cerr, invalidByteOrder);
#endif
        parseTree->extract(*crwImage, invalidByteOrder);

    } // CrwParser::decode

    void CiffComponent::read(const byte* buf,
                             uint32_t len,
                             uint32_t start,
                             ByteOrder byteOrder,
                             int32_t /*shift*/)
    {
        if (len < 10) throw Error(33);
        tag_ = getUShort(buf + start, byteOrder);
        switch (dataLocation()) {
        case valueData:
            size_ = getULong(buf + start + 2, byteOrder);
            offset_ = getULong(buf + start + 6, byteOrder);
            break;
        case directoryData:
            size_ = 8;
            offset_ = start + 2;
            break;
        case invalidDataLocId:
        case lastDataLocId:
            // empty
            break;
        }
        pData_ = buf + offset_;
    } // CiffComponent::read

    void CiffComponent::print(std::ostream& os,
                             ByteOrder byteOrder,
                             const std::string& prefix) const
    {
        os << prefix
           << "tag = 0x" << std::setw(4) << std::setfill('0')
           << std::hex << std::right << tagId()
           << ", dir = 0x" << std::setw(4) << std::setfill('0')
           << std::hex << std::right << dir()
           << ", type = " << TypeInfo::typeName(typeId())
           << ", size = " << std::dec << size_
           << ", offset = " << offset_ << "\n";

        Value::AutoPtr value;
        if (typeId() != directory) {
            value = Value::create(typeId());
            value->read(pData_, size_, byteOrder);
            if (value->size() < 100) {
                os << prefix << *value << "\n";
            }
        }
    } // CiffComponent::print

    TypeId CiffComponent::typeId(uint16_t tag)
    {
        TypeId ti = invalidTypeId;
        switch (tag & 0x3800) {
        case 0x0000: ti = unsignedByte; break;
        case 0x0800: ti = asciiString; break;
        case 0x1000: ti = unsignedShort; break;
        case 0x1800: ti = unsignedLong; break;
        case 0x2000: ti = undefined; break;
        case 0x2800: // fallthrough
        case 0x3000: ti = directory; break;
        }
        return ti;
    } // CiffComponent::typeId

    DataLocId CiffComponent::dataLocation(uint16_t tag)
    {
        DataLocId di = invalidDataLocId;
        switch (tag & 0xc000) {
        case 0x0000: di = valueData; break;
        case 0x4000: di = directoryData; break;
        }
        return di;
    } // CiffComponent::dataLocation

    void CiffEntry::extract(Image& image, ByteOrder byteOrder) const
    {
        CrwMap::extract(*this, image, byteOrder);
    } // CiffEntry::extract

    void CiffEntry::add(RawMetadata::AutoPtr component)
    {
        throw Error(34, "CiffEntry::add");
    } // CiffEntry::add

    CiffDirectory::~CiffDirectory()
    {
        RawMetadata::Components::iterator b = components_.begin();
        RawMetadata::Components::iterator e = components_.end();
        for (RawMetadata::Components::iterator i = b; i != e; ++i) {
            delete *i;
        }
    }

    void CiffDirectory::add(RawMetadata::AutoPtr component)
    {
        components_.push_back(component.release());
    } // CiffDirectory::add

    void CiffDirectory::read(const byte* buf,
                             uint32_t len,
                             uint32_t start,
                             ByteOrder byteOrder,
                             int32_t /*shift*/)
    {
        CiffComponent::read(buf, len, start, byteOrder);
        readDirectory(buf + offset(), size(), 0, byteOrder, 0);
    } // CiffDirectory::read

    void CiffDirectory::extract(Image& image, ByteOrder byteOrder) const
    {
        RawMetadata::Components::const_iterator b = components_.begin();
        RawMetadata::Components::const_iterator e = components_.end();
        for (RawMetadata::Components::const_iterator i = b; i != e; ++i) {
            (*i)->extract(image, byteOrder);
        }
    } // CiffDirectory::extract

    void CiffDirectory::print(std::ostream& os,
                             ByteOrder byteOrder,
                             const std::string& prefix) const
    {
        CiffComponent::print(os, byteOrder, prefix);
        RawMetadata::Components::const_iterator b = components_.begin();
        RawMetadata::Components::const_iterator e = components_.end();
        for (RawMetadata::Components::const_iterator i = b; i != e; ++i) {
            (*i)->print(os, byteOrder, prefix + "   ");
        }
    } // CiffDirectory::print

    void CiffDirectory::readDirectory(const byte* buf,
                                      uint32_t len,
                                      uint32_t start,
                                      ByteOrder byteOrder,
                                      int32_t /*shift*/)
    {
        uint32_t dataSize = getULong(buf + len - 4, byteOrder);
        uint32_t o = start + dataSize;
        if (o + 2 > len) throw Error(33);
        uint16_t count = getUShort(buf + o, byteOrder);
        o += 2;
        for (uint16_t i = 0; i < count; ++i) {
            if (o + 10 > len) throw Error(33);
            uint16_t tag = getUShort(buf + o, byteOrder);
            CiffComponent* p = 0;
            switch (CiffComponent::typeId(tag)) {
            case directory: p = new CiffDirectory; break;
            default: p = new CiffEntry; break;
            }
            p->setDir(this->tag());
            RawMetadata::AutoPtr m(p);
            m->read(buf, len, o, byteOrder);
            add(m);
            o += 10;
        }
    }  // CiffDirectory::readDirectory

    const char CiffHeader::signature_[] = "HEAPCCDR";

    CiffHeader::~CiffHeader()
    {
        delete rootDirectory_;
    }

    void CiffHeader::add(RawMetadata::AutoPtr component)
    {
        throw Error(34, "CiffHeader::add");
    } // CiffHeader::add

    void CiffHeader::read(const byte* buf,
                          uint32_t len,
                          uint32_t start,
                          ByteOrder byteOrder,
                          int32_t /*shift*/)
    {
        if (len < 14) throw Error(33);

        if (buf[0] == 0x49 && buf[1] == 0x49) {
            byteOrder_ = littleEndian;
        }
        else if (buf[0] == 0x4d && buf[1] == 0x4d) {
            byteOrder_ = bigEndian;
        }
        else {
            throw Error(33);
        }
        offset_ = getULong(buf + 2, byteOrder_);
        if (std::memcmp(buf + 6, signature_, 8) != 0) {
            throw Error(33);
        }

        rootDirectory_ = new CiffDirectory;
        rootDirectory_->readDirectory(buf + offset_, len - offset_, 0, byteOrder_);
    } // CiffHeader::read

    void CiffHeader::extract(Image& image, ByteOrder byteOrder) const
    {
        // Nothing to extract from the header itself, just add correct byte order
        if (rootDirectory_) rootDirectory_->extract(image, byteOrder_);
    } // CiffHeader::extract

    void CiffHeader::print(std::ostream& os,
                          ByteOrder byteOrder,
                          const std::string& prefix) const
    {
        os << prefix
           << "Header, offset = 0x" << std::setw(8) << std::setfill('0')
           << std::hex << std::right << offset_ << "\n";
        if (rootDirectory_) rootDirectory_->print(os, byteOrder_, prefix);
    } // CiffHeader::print

    const CrwMapInfo CrwMap::crwMapInfos_[] = {
        CrwMapInfo(0x0805, 0x300a, 0, 0x9286, exifIfdId, extract0x0805, 0),
        CrwMapInfo(0x080a, 0x2807, 0, 0x010f, ifd0Id, extract0x080a, 0),
        CrwMapInfo(0x080a, 0x2807, 0, 0x0110, ifd0Id, 0, 0),
        CrwMapInfo(0x080b, 0x3004, 0, 0x0007, canonIfdId, extractBasic, 0),
        CrwMapInfo(0x0810, 0x2807, 0, 0x0009, canonIfdId, extractBasic, 0),
        CrwMapInfo(0x0815, 0x2804, 0, 0x0006, canonIfdId, extractBasic, 0),
        CrwMapInfo(0x1029, 0x300b, 0, 0x0002, canonIfdId, extractBasic, 0),
        CrwMapInfo(0x102a, 0x300b, 0, 0x0004, canonIfdId, extract0x102a, 0),
        CrwMapInfo(0x102d, 0x300b, 0, 0x0001, canonIfdId, extract0x102d, 0),
        CrwMapInfo(0x1033, 0x300b, 0, 0x000f, canonIfdId, extractBasic, 0),
        CrwMapInfo(0x1038, 0x300b, 0, 0x0012, canonIfdId, extractBasic, 0),
        CrwMapInfo(0x10a9, 0x300b, 0, 0x00a9, canonIfdId, extractBasic, 0),
//        CrwMapInfo(0x10b4, 0x300b, 0, 0x00b4, canonIfdId, extractBasic, 0),
        CrwMapInfo(0x10b4, 0x300b, 0, 0xa001, exifIfdId, extractBasic, 0),
        CrwMapInfo(0x10b5, 0x300b, 0, 0x00b5, canonIfdId, extractBasic, 0),
        CrwMapInfo(0x10c0, 0x300b, 0, 0x00c0, canonIfdId, extractBasic, 0),
        CrwMapInfo(0x10c1, 0x300b, 0, 0x00c1, canonIfdId, extractBasic, 0),
        CrwMapInfo(0x1807, 0x3002, 0, 0x9206, exifIfdId, extractBasic, 0),
        CrwMapInfo(0x180b, 0x2807, 0, 0x000c, canonIfdId, extractBasic, 0),
        CrwMapInfo(0x180e, 0x300a, 0, 0x9003, exifIfdId, extract0x180e, 0),
        CrwMapInfo(0x1810, 0x300a, 0, 0xa002, exifIfdId, extract0x1810, 0),
        CrwMapInfo(0x1810, 0x300a, 0, 0xa003, exifIfdId, extract0x1810, 0),
        CrwMapInfo(0x1817, 0x300a, 4, 0x0008, canonIfdId, extractBasic, 0),
//        CrwMapInfo(0x1818, 0x3002, 0, 0x9204, exifIfdId, extractBasic, 0),
        CrwMapInfo(0x183b, 0x300b, 0, 0x0015, canonIfdId, extractBasic, 0),
        CrwMapInfo(0x2008, 0x0000, 0, 0x0201, ifd1Id, extract0x2008, 0),
        CrwMapInfo(0x2008, 0x0000, 0, 0x0202, ifd1Id, 0, 0),
        CrwMapInfo(0x2008, 0x0000, 0, 0x0103, ifd1Id, 0, 0),
        CrwMapInfo(0x0000, 0x0000, 0, 0x0000, ifdIdNotSet, extractBasic, 0)
    }; // CrwMap::crwMapInfos_[]

    void CrwMap::extract(const CiffComponent& ciffComponent,
                         Image& image,
                         ByteOrder byteOrder)
    {
        const CrwMapInfo* cmi = crwMapInfo(ciffComponent.dir(),
                                           ciffComponent.tagId());
        if (cmi && cmi->toExif_) {
            cmi->toExif_(ciffComponent, cmi, image, byteOrder);
        }
    } // CrwMap::extract

    const CrwMapInfo* CrwMap::crwMapInfo(uint16_t dir, uint16_t tagId)
    {
        for (int i = 0; crwMapInfos_[i].ifdId_ != ifdIdNotSet; ++i) {
            if (   crwMapInfos_[i].crwDir_ == dir
                && crwMapInfos_[i].crwTagId_ == tagId) {
                return &(crwMapInfos_[i]);
            }
        }
        return 0;
    } // CrwMap::crwMapInfo

    void CrwMap::extract0x0805(const CiffComponent& ciffComponent,
                               const CrwMapInfo* crwMapInfo,
                               Image& image,
                               ByteOrder /*byteOrder*/)
    {
        std::string s(reinterpret_cast<const char*>(ciffComponent.pData()));
        image.setComment(s);
    } // CrwMap::extract0x0805

    void CrwMap::extract0x080a(const CiffComponent& ciffComponent,
                               const CrwMapInfo* crwMapInfo,
                               Image& image,
                               ByteOrder byteOrder)
    {
        if (ciffComponent.typeId() != asciiString) {
            return extractBasic(ciffComponent, crwMapInfo, image, byteOrder);
        }

        // Make
        ExifKey key1("Exif.Image.Make");
        Value::AutoPtr value1 = Value::create(ciffComponent.typeId());
        uint32_t i = 0;
        for (;    i < ciffComponent.size()
               && ciffComponent.pData()[i] != '\0'; ++i) {
            // empty
        }
        value1->read(ciffComponent.pData(), ++i, byteOrder);
        image.exifData().add(key1, value1.get());

        // Model
        ExifKey key2("Exif.Image.Model");
        Value::AutoPtr value2 = Value::create(ciffComponent.typeId());
        uint32_t j = i;
        for (;    i < ciffComponent.size()
               && ciffComponent.pData()[i] != '\0'; ++i) {
            // empty
        }
        value2->read(ciffComponent.pData() + j, i - j + 1, byteOrder);
        image.exifData().add(key2, value2.get());
    } // CrwMap::extract0x080a

    void CrwMap::extract0x102a(const CiffComponent& ciffComponent,
                               const CrwMapInfo* crwMapInfo,
                               Image& image,
                               ByteOrder byteOrder)
    {
        if (ciffComponent.typeId() != unsignedShort) {
            return extractBasic(ciffComponent, crwMapInfo, image, byteOrder);
        }

        long aperture = 0;
        long shutterSpeed = 0;

        std::string ifdItem(ExifTags::ifdItem(canonCs2IfdId));
        uint16_t c = 1;
        while (uint32_t(c)*2 < ciffComponent.size()) {
            uint16_t n = 1;
            ExifKey key(c, ifdItem);
            UShortValue value;
            value.read(ciffComponent.pData() + c*2, n*2, byteOrder);
            image.exifData().add(key, &value);
            if (c == 21) aperture = value.toLong();
            if (c == 22) shutterSpeed = value.toLong();
            c += n;
        }

        // Exif.Photo.FNumber
        float f = fnumber(canonEv(aperture));
        // Beware: primitive conversion algorithm
        uint32_t den = 1000000;
        uint32_t nom = static_cast<uint32_t>(f * den);
        uint32_t g = gcd(nom, den);
        URational ur(nom/g, den/g);
        URationalValue fn;
        fn.value_.push_back(ur);
        image.exifData().add(ExifKey("Exif.Photo.FNumber"), &fn);

        // Exif.Photo.ExposureTime
        ur = exposureTime(canonEv(shutterSpeed));
        URationalValue et;
        et.value_.push_back(ur);
        image.exifData().add(ExifKey("Exif.Photo.ExposureTime"), &et);

    } // CrwMap::extract0x102a

    void CrwMap::extract0x102d(const CiffComponent& ciffComponent,
                               const CrwMapInfo* crwMapInfo,
                               Image& image,
                               ByteOrder byteOrder)
    {
        if (ciffComponent.typeId() != unsignedShort) {
            return extractBasic(ciffComponent, crwMapInfo, image, byteOrder);
        }

        std::string ifdItem(ExifTags::ifdItem(canonCs1IfdId));
        uint16_t c = 1;
        while (uint32_t(c)*2 < ciffComponent.size()) {
            uint16_t n = 1;
            ExifKey key(c, ifdItem);
            UShortValue value;
            if (c == 23 && ciffComponent.size() > 50) n = 3;
            value.read(ciffComponent.pData() + c*2, n*2, byteOrder);
            image.exifData().add(key, &value);
            c += n;
        }
    } // CrwMap::extract0x102d

    void CrwMap::extract0x180e(const CiffComponent& ciffComponent,
                               const CrwMapInfo* crwMapInfo,
                               Image& image,
                               ByteOrder byteOrder)
    {
        if (ciffComponent.size() < 8 || ciffComponent.typeId() != unsignedLong) {
            return extractBasic(ciffComponent, crwMapInfo, image, byteOrder);
        }

        ULongValue v;
        v.read(ciffComponent.pData(), 8, byteOrder);
        time_t t = v.value_[0];
#ifdef EXV_HAVE_GMTIME_R
        struct tm tms;
        struct tm* tm = &tms;
        tm = gmtime_r(&t, tm);
#else
        struct tm* tm = std::gmtime(&t);
#endif
        if (tm) {
            const size_t m = 20;
            char s[m];
            std::strftime(s, m, "%Y:%m:%d %T", tm);

            ExifKey key(crwMapInfo->tag_, ExifTags::ifdItem(crwMapInfo->ifdId_));
            AsciiValue value;
            value.read(std::string(s));
            image.exifData().add(key, &value);
        }
    } // CrwMap::extract0x180e

    void CrwMap::extract0x1810(const CiffComponent& ciffComponent,
                               const CrwMapInfo* crwMapInfo,
                               Image& image,
                               ByteOrder byteOrder)
    {
        if (ciffComponent.typeId() != unsignedLong || ciffComponent.size() < 28) {
            return extractBasic(ciffComponent, crwMapInfo, image, byteOrder);
        }

        ExifKey key1("Exif.Photo.PixelXDimension");
        ULongValue value1;
        value1.read(ciffComponent.pData(), 4, byteOrder);
        image.exifData().add(key1, &value1);

        ExifKey key2("Exif.Photo.PixelYDimension");
        ULongValue value2;
        value2.read(ciffComponent.pData() + 4, 4, byteOrder);
        image.exifData().add(key2, &value2);

    } // CrwMap::extract0x1810

    void CrwMap::extract0x2008(const CiffComponent& ciffComponent,
                               const CrwMapInfo* /*crwMapInfo*/,
                               Image& image,
                               ByteOrder /*byteOrder*/)
    {
        image.exifData().setJpegThumbnail(ciffComponent.pData(),
                                          ciffComponent.size());
    } // CrwMap::extract0x2008

    void CrwMap::extractBasic(const CiffComponent& ciffComponent,
                              const CrwMapInfo* crwMapInfo,
                              Image& image,
                              ByteOrder byteOrder)
    {
        // create a key and value pair
        ExifKey key(crwMapInfo->tag_, ExifTags::ifdItem(crwMapInfo->ifdId_));
        Value::AutoPtr value;
        if (ciffComponent.typeId() != directory) {
            value = Value::create(ciffComponent.typeId());
            uint32_t size = 0;
            if (crwMapInfo->size_ != 0) {
                // size in the mapping table overrides all
                size = crwMapInfo->size_;
            }
            else if (ciffComponent.typeId() == asciiString) {
                // determine size from the data, by looking for the first 0
                uint32_t i = 0;
                for (;    i < ciffComponent.size()
                       && ciffComponent.pData()[i] != '\0'; ++i) {
                    // empty
                }
                size = ++i;
            }
            else {
                // by default, use the size from the directory entry
                size = ciffComponent.size();
            }
            value->read(ciffComponent.pData(), size, byteOrder);
        }
        // Add metadatum to exif data
        image.exifData().add(key, value.get());
    } // CrwMap::extractBasic


    // *************************************************************************
    // free functions

    Image::AutoPtr newCrwInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image = Image::AutoPtr(new CrwImage(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isCrwType(BasicIo& iIo, bool advance)
    {
        bool result = true;
        byte tmpBuf[14];
        iIo.read(tmpBuf, 14);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        if (!(   ('I' == tmpBuf[0] && 'I' == tmpBuf[1])
              || ('M' == tmpBuf[0] && 'M' == tmpBuf[1]))) {
            result = false;
        }
        if (   true == result
            && std::memcmp(tmpBuf + 6, CiffHeader::signature_, 8) != 0) {
            result = false;
        }
        if (!advance || !result) iIo.seek(-14, BasicIo::cur);
        return result;
    }

}                                       // namespace Exiv2
