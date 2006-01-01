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

    // Forward declaration
    class CrwEncoder;

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
#ifdef DEBUG
        std::cerr << "Writing CRW file " << io_->path() << "\n";
#endif
        // Read existing image
        DataBuf buf;
        if (io_->open() == 0) {
            IoCloser closer(*io_);
            // Ensure that this is the correct image type
            if (isThisType(*io_, false)) {
                // Read the image into a memory buffer
                buf.alloc(io_->size());
                io_->read(buf.pData_, buf.size_);
                if (io_->error() || io_->eof()) {
                    buf.reset();
                }
            }
        }

        // Parse image, starting with a CIFF header component
        CiffHeader::AutoPtr parseTree(new CiffHeader);
        if (buf.size_ != 0) {
            parseTree->read(buf.pData_, buf.size_);
        }

        Blob blob;
        CrwParser::encode(blob, parseTree.get(), this);

        // Write new buffer to file
        BasicIo::AutoPtr tempIo(io_->temporary()); // may throw
        assert (tempIo.get() != 0);
        tempIo->write(&blob[0], blob.size());
        io_->close();
        io_->transfer(*tempIo); // may throw

    } // CrwImage::writeMetadata

    bool CrwImage::isThisType(BasicIo& iIo, bool advance) const
    {
        return isCrwType(iIo, advance);
    }

    void CrwParser::decode(CrwImage* crwImage, const byte* buf, uint32_t len)
    {
        assert(crwImage != 0);
        assert(buf != 0);

        // Parse the image, starting with a CIFF header component
        CiffHeader::AutoPtr parseTree(new CiffHeader);
        parseTree->read(buf, len);
#ifdef DEBUG
        parseTree->print(std::cerr);
#endif
        parseTree->decode(*crwImage);

    } // CrwParser::decode

    void CrwParser::encode(Blob& blob, CiffHeader* parseTree, const CrwImage* crwImage)
    {
        assert(crwImage != 0);
        assert(parseTree != 0);

        // Encode Exif tags from image into the Crw parse tree and write the structure
        // to the binary image blob
        CrwMap::encode(parseTree, *crwImage);
        parseTree->write(blob);

    } // CrwParser::encode

    void CiffComponent::add(AutoPtr component)
    {
        doAdd(component);
    }
    
    void CiffComponent::read(const byte* buf,
                             uint32_t len,
                             uint32_t start,
                             ByteOrder byteOrder)
    {
        doRead(buf, len, start, byteOrder);
    }

    void CiffComponent::decode(Image& image, ByteOrder byteOrder) const
    {
        doDecode(image, byteOrder);
    }

    uint32_t CiffComponent::write(Blob& blob, 
                                  ByteOrder byteOrder, 
                                  uint32_t offset)
    {
        return doWrite(blob, byteOrder, offset);
    }

    void CiffComponent::print(std::ostream& os,
                              ByteOrder byteOrder,
                              const std::string& prefix) const
    {
        doPrint(os, byteOrder, prefix);
    }

    uint32_t CiffComponent::writeValueData(Blob& blob, uint32_t offset)
    {
        if (dataLocation() == valueData) {
            offset_ = offset;
            append(blob, pData_, size_);
            offset += size_;
            // Pad the value to an even number of bytes
            if (size_ % 2 == 1) {
                blob.push_back(0);
                ++offset;
            }
        }
        return offset;
    } // CiffComponent::writeValueData

    void CiffComponent::writeDirEntry(Blob& blob, ByteOrder byteOrder) const
    {
        byte buf[4];

        DataLocId dl = dataLocation();
        assert(dl == directoryData || dl == valueData);

        if (dl == valueData) {
            us2Data(buf, tag_, byteOrder);
            append(blob, buf, 2);

            ul2Data(buf, size_, byteOrder);
            append(blob, buf, 4);

            ul2Data(buf, offset_, byteOrder);
            append(blob, buf, 4);
        }

        if (dl == directoryData) {
            // Only 8 bytes fit in the directory entry
            assert(size_ <= 8);

            us2Data(buf, tag_, byteOrder);
            append(blob, buf, 2);
            // Copy value instead of size and offset
            append(blob, pData_, size_);
            // Pad with 0s
            for (uint32_t i = size_; i < 8; ++i) {
                blob.push_back(0);
            }
        }
    } // CiffComponent::writeDirEntry

    void CiffComponent::doRead(const byte* buf,
                               uint32_t len,
                               uint32_t start,
                               ByteOrder byteOrder)
    {
        if (len < 10) throw Error(33);
        tag_ = getUShort(buf + start, byteOrder);

        DataLocId dl = dataLocation();
        assert(dl == directoryData || dl == valueData);

        if (dl == valueData) {
            size_   = getULong(buf + start + 2, byteOrder);
            offset_ = getULong(buf + start + 6, byteOrder);
        }
        if (dl == directoryData) {
            size_ = 8;
            offset_ = start + 2;
        }
        pData_ = buf + offset_;
    } // CiffComponent::doRead

    void CiffComponent::doPrint(std::ostream& os,
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
    } // CiffComponent::doPrint

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

    void CiffEntry::doAdd(AutoPtr component)
    {
        throw Error(34, "CiffEntry::add");
    } // CiffEntry::doAdd

    CiffDirectory::~CiffDirectory()
    {
        Components::iterator b = components_.begin();
        Components::iterator e = components_.end();
        for (Components::iterator i = b; i != e; ++i) {
            delete *i;
        }
    }

    uint32_t CiffEntry::doWrite(Blob& blob, 
                                ByteOrder /*byteOrder*/, 
                                uint32_t offset)
    {
        return writeValueData(blob, offset);
    } // CiffEntry::doWrite

    void CiffEntry::doDecode(Image& image, ByteOrder byteOrder) const
    {
        CrwMap::decode(*this, image, byteOrder);
    } // CiffEntry::doDecode

    void CiffDirectory::doAdd(AutoPtr component)
    {
        components_.push_back(component.release());
    } // CiffDirectory::doAdd

    void CiffDirectory::doRead(const byte* buf,
                               uint32_t len,
                               uint32_t start,
                               ByteOrder byteOrder)
    {
        CiffComponent::doRead(buf, len, start, byteOrder);
        readDirectory(buf + offset(), size(), byteOrder);
    } // CiffDirectory::doRead

    uint32_t CiffDirectory::doWrite(Blob& blob, 
                                    ByteOrder byteOrder, 
                                    uint32_t offset)
    {
        // Ciff offsets are relative to the start of the directory
        uint32_t dirOffset = 0; 

        // Value data
        const Components::iterator b = components_.begin();
        const Components::iterator e = components_.end();
        for (Components::iterator i = b; i != e; ++i) {
            dirOffset = (*i)->write(blob, byteOrder, dirOffset);
        }
        const uint32_t dirStart = dirOffset;

        // Number of directory entries
        byte buf[4];
        us2Data(buf, components_.size(), byteOrder);
        append(blob, buf, 2);
        dirOffset += 2;

        // Directory entries 
        for (Components::iterator i = b; i != e; ++i) {
            (*i)->writeDirEntry(blob, byteOrder);
            dirOffset += 10;
        }

        // Offset of directory
        ul2Data(buf, dirStart, byteOrder);
        append(blob, buf, 4);
        dirOffset += 4;

        // Update directory entry
        setOffset(offset);
        setSize(dirOffset);

        return offset + dirOffset;
    } // CiffDirectory::doWrite

    void CiffDirectory::doDecode(Image& image, ByteOrder byteOrder) const
    {
        Components::const_iterator b = components_.begin();
        Components::const_iterator e = components_.end();
        for (Components::const_iterator i = b; i != e; ++i) {
            (*i)->decode(image, byteOrder);
        }
    } // CiffDirectory::doDecode

    void CiffDirectory::doPrint(std::ostream& os,
                                ByteOrder byteOrder,
                                const std::string& prefix) const
    {
        CiffComponent::doPrint(os, byteOrder, prefix);
        Components::const_iterator b = components_.begin();
        Components::const_iterator e = components_.end();
        for (Components::const_iterator i = b; i != e; ++i) {
            (*i)->print(os, byteOrder, prefix + "   ");
        }
    } // CiffDirectory::doPrint

    void CiffDirectory::readDirectory(const byte* buf,
                                      uint32_t len,
                                      ByteOrder byteOrder)
    {
        uint32_t o = getULong(buf + len - 4, byteOrder);
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
            AutoPtr m(p);
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

    void CiffHeader::read(const byte* buf, uint32_t len)
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
        if (std::memcmp(buf + 6, signature(), 8) != 0) {
            throw Error(33);
        }

        rootDirectory_ = new CiffDirectory;
        rootDirectory_->readDirectory(buf + offset_, len - offset_, byteOrder_);
    } // CiffHeader::read

    void CiffHeader::write(Blob& blob)
    {
        assert(   byteOrder_ == littleEndian
               || byteOrder_ == bigEndian);
        if (byteOrder_ == littleEndian) {
            blob.push_back(0x49);
            blob.push_back(0x49);
        }
        else {
            blob.push_back(0x4d);
            blob.push_back(0x4d);
        }
        uint32_t o = 2;
        byte buf[4];
        ul2Data(buf, offset_, byteOrder_);
        append(blob, buf, 4);
        o += 4;
        append(blob, reinterpret_cast<const byte*>(signature_), 8);
        o += 8;
        // Pad with 0s if needed
        for (uint32_t i = o; i < offset_; ++i) {
            blob.push_back(0);
            ++o;
        }
        if (rootDirectory_) {
            rootDirectory_->write(blob, byteOrder_, offset_);
        }
    }

    void CiffHeader::decode(Image& image) const
    {
        // Nothing to decode from the header itself, just add correct byte order
        if (rootDirectory_) rootDirectory_->decode(image, byteOrder_);
    } // CiffHeader::decode

    void CiffHeader::print(std::ostream& os, const std::string& prefix) const
    {
        os << prefix
           << "Header, offset = 0x" << std::setw(8) << std::setfill('0')
           << std::hex << std::right << offset_ << "\n";
        if (rootDirectory_) rootDirectory_->print(os, byteOrder_, prefix);
    } // CiffHeader::print

    const CrwDecodeMap CrwMap::crwDecodeInfos_[] = {
        //           CrwTag  CrwDir  Size ExifTag IfdId        decodeFct     encodeFct
        CrwDecodeMap(0x0805, 0x300a,   0, 0x9286, exifIfdId,   decode0x0805, 0),
        CrwDecodeMap(0x080a, 0x2807,   0, 0x010f, ifd0Id,      decode0x080a, 0),
        CrwDecodeMap(0x080a, 0x2807,   0, 0x0110, ifd0Id,      0,            0),
        CrwDecodeMap(0x080b, 0x3004,   0, 0x0007, canonIfdId,  decodeBasic,  0),
        CrwDecodeMap(0x0810, 0x2807,   0, 0x0009, canonIfdId,  decodeBasic,  0),
        CrwDecodeMap(0x0815, 0x2804,   0, 0x0006, canonIfdId,  decodeBasic,  0),
        CrwDecodeMap(0x1029, 0x300b,   0, 0x0002, canonIfdId,  decodeBasic,  0),
        CrwDecodeMap(0x102a, 0x300b,   0, 0x0004, canonIfdId,  decode0x102a, 0),
        CrwDecodeMap(0x102d, 0x300b,   0, 0x0001, canonIfdId,  decode0x102d, 0),
        CrwDecodeMap(0x1033, 0x300b,   0, 0x000f, canonIfdId,  decodeBasic,  0),
        CrwDecodeMap(0x1038, 0x300b,   0, 0x0012, canonIfdId,  decodeBasic,  0),
        CrwDecodeMap(0x10a9, 0x300b,   0, 0x00a9, canonIfdId,  decodeBasic,  0),
//      CrwDecodeMap(0x10b4, 0x300b,   0, 0x00b4, canonIfdId,  decodeBasic,  0),
        CrwDecodeMap(0x10b4, 0x300b,   0, 0xa001, exifIfdId,   decodeBasic,  encodeBasic),
        CrwDecodeMap(0x10b5, 0x300b,   0, 0x00b5, canonIfdId,  decodeBasic,  0),
        CrwDecodeMap(0x10c0, 0x300b,   0, 0x00c0, canonIfdId,  decodeBasic,  0),
        CrwDecodeMap(0x10c1, 0x300b,   0, 0x00c1, canonIfdId,  decodeBasic,  0),
        CrwDecodeMap(0x1807, 0x3002,   0, 0x9206, exifIfdId,   decodeBasic,  0),
        CrwDecodeMap(0x180b, 0x2807,   0, 0x000c, canonIfdId,  decodeBasic,  0),
        CrwDecodeMap(0x180e, 0x300a,   0, 0x9003, exifIfdId,   decode0x180e, 0),
        CrwDecodeMap(0x1810, 0x300a,   0, 0xa002, exifIfdId,   decode0x1810, 0),
        CrwDecodeMap(0x1810, 0x300a,   0, 0xa003, exifIfdId,   decode0x1810, 0),
        CrwDecodeMap(0x1817, 0x300a,   4, 0x0008, canonIfdId,  decodeBasic,  0),
//      CrwDecodeMap(0x1818, 0x3002,   0, 0x9204, exifIfdId,   decodeBasic,  0),
        CrwDecodeMap(0x183b, 0x300b,   0, 0x0015, canonIfdId,  decodeBasic,  0),
        CrwDecodeMap(0x2008, 0x0000,   0, 0x0201, ifd1Id,      decode0x2008, 0),
        CrwDecodeMap(0x2008, 0x0000,   0, 0x0202, ifd1Id,      0,            0),
        CrwDecodeMap(0x2008, 0x0000,   0, 0x0103, ifd1Id,      0,            0),
        CrwDecodeMap(0x0000, 0x0000,   0, 0x0000, ifdIdNotSet, decodeBasic,  0)
    }; // CrwMap::crwDecodeInfos_[]

    void CrwMap::decode(const CiffComponent& ciffComponent,
                        Image& image,
                        ByteOrder byteOrder)
    {
        const CrwDecodeMap* cmi = crwDecodeInfo(ciffComponent.dir(),
                                                ciffComponent.tagId());
        if (cmi && cmi->toExif_) {
            cmi->toExif_(ciffComponent, cmi, image, byteOrder);
        }
    } // CrwMap::decode

    void CrwMap::encode(CiffHeader* parseTree, const Image& image)
    {
        for (const CrwDecodeMap* cmi = crwDecodeInfos_; 
             cmi->ifdId_ != ifdIdNotSet; ++cmi) {
            if (cmi->fromExif_ != 0) {
                cmi->fromExif_(image, cmi, parseTree);
            }
        }
    } // CrwMap::encode

    const CrwDecodeMap* CrwMap::crwDecodeInfo(uint16_t dir, uint16_t tagId)
    {
        for (int i = 0; crwDecodeInfos_[i].ifdId_ != ifdIdNotSet; ++i) {
            if (   crwDecodeInfos_[i].crwDir_ == dir
                && crwDecodeInfos_[i].crwTagId_ == tagId) {
                return &(crwDecodeInfos_[i]);
            }
        }
        return 0;
    } // CrwMap::crwDecodeInfo

    void CrwMap::decode0x0805(const CiffComponent& ciffComponent,
                              const CrwDecodeMap* /*crwDecodeInfo*/,
                              Image& image,
                              ByteOrder /*byteOrder*/)
    {
        std::string s(reinterpret_cast<const char*>(ciffComponent.pData()));
        image.setComment(s);
    } // CrwMap::decode0x0805

    void CrwMap::decode0x080a(const CiffComponent& ciffComponent,
                              const CrwDecodeMap* crwDecodeInfo,
                              Image& image,
                              ByteOrder byteOrder)
    {
        if (ciffComponent.typeId() != asciiString) {
            return decodeBasic(ciffComponent, crwDecodeInfo, image, byteOrder);
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
    } // CrwMap::decode0x080a

    void CrwMap::decode0x102a(const CiffComponent& ciffComponent,
                              const CrwDecodeMap* crwDecodeInfo,
                              Image& image,
                              ByteOrder byteOrder)
    {
        if (ciffComponent.typeId() != unsignedShort) {
            return decodeBasic(ciffComponent, crwDecodeInfo, image, byteOrder);
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

    } // CrwMap::decode0x102a

    void CrwMap::decode0x102d(const CiffComponent& ciffComponent,
                              const CrwDecodeMap* crwDecodeInfo,
                              Image& image,
                              ByteOrder byteOrder)
    {
        if (ciffComponent.typeId() != unsignedShort) {
            return decodeBasic(ciffComponent, crwDecodeInfo, image, byteOrder);
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
    } // CrwMap::decode0x102d

    void CrwMap::decode0x180e(const CiffComponent& ciffComponent,
                              const CrwDecodeMap* crwDecodeInfo,
                              Image& image,
                              ByteOrder byteOrder)
    {
        if (ciffComponent.size() < 8 || ciffComponent.typeId() != unsignedLong) {
            return decodeBasic(ciffComponent, crwDecodeInfo, image, byteOrder);
        }
        assert(crwDecodeInfo != 0);
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

            ExifKey key(crwDecodeInfo->tag_, ExifTags::ifdItem(crwDecodeInfo->ifdId_));
            AsciiValue value;
            value.read(std::string(s));
            image.exifData().add(key, &value);
        }
    } // CrwMap::decode0x180e

    void CrwMap::decode0x1810(const CiffComponent& ciffComponent,
                              const CrwDecodeMap* crwDecodeInfo,
                              Image& image,
                              ByteOrder byteOrder)
    {
        if (ciffComponent.typeId() != unsignedLong || ciffComponent.size() < 28) {
            return decodeBasic(ciffComponent, crwDecodeInfo, image, byteOrder);
        }

        ExifKey key1("Exif.Photo.PixelXDimension");
        ULongValue value1;
        value1.read(ciffComponent.pData(), 4, byteOrder);
        image.exifData().add(key1, &value1);

        ExifKey key2("Exif.Photo.PixelYDimension");
        ULongValue value2;
        value2.read(ciffComponent.pData() + 4, 4, byteOrder);
        image.exifData().add(key2, &value2);

    } // CrwMap::decode0x1810

    void CrwMap::decode0x2008(const CiffComponent& ciffComponent,
                              const CrwDecodeMap* /*crwDecodeInfo*/,
                              Image& image,
                              ByteOrder /*byteOrder*/)
    {
        image.exifData().setJpegThumbnail(ciffComponent.pData(),
                                          ciffComponent.size());
    } // CrwMap::decode0x2008

    void CrwMap::decodeBasic(const CiffComponent& ciffComponent,
                             const CrwDecodeMap* crwDecodeInfo,
                             Image& image,
                             ByteOrder byteOrder)
    {
        assert(crwDecodeInfo != 0);
        // create a key and value pair
        ExifKey key(crwDecodeInfo->tag_, ExifTags::ifdItem(crwDecodeInfo->ifdId_));
        Value::AutoPtr value;
        if (ciffComponent.typeId() != directory) {
            value = Value::create(ciffComponent.typeId());
            uint32_t size = 0;
            if (crwDecodeInfo->size_ != 0) {
                // size in the mapping table overrides all
                size = crwDecodeInfo->size_;
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
    } // CrwMap::decodeBasic

    void CrwMap::encodeBasic(const Image& image,
                             const CrwDecodeMap* crwDecodeInfo,
                             CiffHeader* parseTree)
    {
        assert(crwDecodeInfo != 0);
        assert(parseTree != 0);
        
        ExifKey ek(crwDecodeInfo->tag_, ExifTags::ifdItem(crwDecodeInfo->ifdId_));
        ExifData::const_iterator ed = image.exifData().findKey(ek);

        if (ed == image.exifData().end()) {
            // delete component if it exists
        }
        else {
            // get or create the correct component

            // set the new value, using
            // ed->size()
            // ed->copy(buf, parseTree->byteOrder())
        }

    } // CrwMap::encodeBasic

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
            && std::memcmp(tmpBuf + 6, CiffHeader::signature(), 8) != 0) {
            result = false;
        }
        if (!advance || !result) iIo.seek(-14, BasicIo::cur);
        return result;
    }

}                                       // namespace Exiv2
