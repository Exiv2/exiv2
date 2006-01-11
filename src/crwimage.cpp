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
#include <stack>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const CrwMapping CrwMap::crwMapping_[] = {
        //         CrwTag  CrwDir  Size ExifTag IfdId        decodeFct     encodeFct
        //         ------  ------  ---- ------- -----        ---------     ---------
        CrwMapping(0x0805, 0x300a,   0, 0x9286, exifIfdId,   decode0x0805, encode0x0805),
        CrwMapping(0x080a, 0x2807,   0, 0x010f, ifd0Id,      decode0x080a, encode0x080a),
        CrwMapping(0x080b, 0x3004,   0, 0x0007, canonIfdId,  decodeBasic,  encodeBasic),
        CrwMapping(0x0810, 0x2807,   0, 0x0009, canonIfdId,  decodeBasic,  encodeBasic),
        CrwMapping(0x0815, 0x2804,   0, 0x0006, canonIfdId,  decodeBasic,  encodeBasic),
        CrwMapping(0x1029, 0x300b,   0, 0x0002, canonIfdId,  decodeBasic,  encodeBasic),
        CrwMapping(0x102a, 0x300b,   0, 0x0004, canonIfdId,  decode0x102a, 0),
        CrwMapping(0x102d, 0x300b,   0, 0x0001, canonIfdId,  decode0x102d, 0),
        CrwMapping(0x1033, 0x300b,   0, 0x000f, canonIfdId,  decodeBasic,  encodeBasic),
        CrwMapping(0x1038, 0x300b,   0, 0x0012, canonIfdId,  decodeBasic,  encodeBasic),
        CrwMapping(0x10a9, 0x300b,   0, 0x00a9, canonIfdId,  decodeBasic,  encodeBasic),
        // Mapped to Exif.Photo.ColorSpace instead (see below)
        //CrwMapping(0x10b4, 0x300b,   0, 0x00b4, canonIfdId,  decodeBasic,  encodeBasic),
        CrwMapping(0x10b4, 0x300b,   0, 0xa001, exifIfdId,   decodeBasic,  encodeBasic),
        CrwMapping(0x10b5, 0x300b,   0, 0x00b5, canonIfdId,  decodeBasic,  encodeBasic),
        CrwMapping(0x10c0, 0x300b,   0, 0x00c0, canonIfdId,  decodeBasic,  encodeBasic),
        CrwMapping(0x10c1, 0x300b,   0, 0x00c1, canonIfdId,  decodeBasic,  encodeBasic),
        CrwMapping(0x1807, 0x3002,   0, 0x9206, exifIfdId,   decodeBasic,  encodeBasic),
        CrwMapping(0x180b, 0x2807,   0, 0x000c, canonIfdId,  decodeBasic,  encodeBasic),
        CrwMapping(0x180e, 0x300a,   0, 0x9003, exifIfdId,   decode0x180e, 0),
        CrwMapping(0x1810, 0x300a,   0, 0xa002, exifIfdId,   decode0x1810, 0),
        CrwMapping(0x1810, 0x300a,   0, 0xa003, exifIfdId,   0,            0),
        CrwMapping(0x1817, 0x300a,   4, 0x0008, canonIfdId,  decodeBasic,  encodeBasic),
        //CrwMapping(0x1818, 0x3002,   0, 0x9204, exifIfdId,   decodeBasic,  encodeBasic),
        CrwMapping(0x183b, 0x300b,   0, 0x0015, canonIfdId,  decodeBasic,  encodeBasic),
        CrwMapping(0x2008, 0x0000,   0, 0x0201, ifd1Id,      decode0x2008, 0),
        CrwMapping(0x2008, 0x0000,   0, 0x0202, ifd1Id,      0,            0),
        CrwMapping(0x2008, 0x0000,   0, 0x0103, ifd1Id,      0,            0),
        // End of list marker
        CrwMapping(0x0000, 0x0000,   0, 0x0000, ifdIdNotSet, 0,            0)
    }; // CrwMap::crwMapping_[]

    /*
      CIFF directory hierarchy

                root
                 |
                300a
                 |
       +----+----+----+----+
       |    |    |    |    |
      2804 2807 3002 3003 300b
            |
           3004

      The array is arranged bottom-up so that starting with a directory at the
      bottom, the (unique) path to root can be determined in a single loop.
    */
    const CrwSubDir CrwMap::crwSubDir_[] = {
        // dir,   parent
        { 0x3004, 0x2807 },
        { 0x300b, 0x300a },
        { 0x3003, 0x300a },
        { 0x3002, 0x300a },
        { 0x2807, 0x300a },
        { 0x2804, 0x300a },
        { 0x300a, 0x0000 },
        { 0x0000, 0xffff },
        // End of list marker
        { 0xffff, 0xffff }
    };

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
        CiffHeader::AutoPtr head(new CiffHeader);
        if (buf.size_ != 0) {
            head->read(buf.pData_, buf.size_);
        }

        Blob blob;
        CrwParser::encode(blob, head.get(), this);

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

    void CrwParser::decode(CrwImage* pCrwImage, const byte* pData, uint32_t size)
    {
        assert(pCrwImage != 0);
        assert(pData != 0);

        // Parse the image, starting with a CIFF header component
        CiffHeader::AutoPtr head(new CiffHeader);
        head->read(pData, size);
#ifdef DEBUG
        head->print(std::cerr);
#endif
        head->decode(*pCrwImage);

    } // CrwParser::decode

    void CrwParser::encode(Blob& blob, CiffHeader* pHead, const CrwImage* pCrwImage)
    {
        assert(pCrwImage != 0);
        assert(pHead != 0);

        // Encode Exif tags from image into the Crw parse tree and write the structure
        // to the binary image blob
        CrwMap::encode(pHead, *pCrwImage);
        pHead->write(blob);

    } // CrwParser::encode

    const char CiffHeader::signature_[] = "HEAPCCDR";

    CiffHeader::~CiffHeader()
    {
        delete pRootDir_;
    }

    CiffComponent::~CiffComponent()
    {
        if (isAllocated_) delete[] pData_; 
    }

    CiffDirectory::~CiffDirectory()
    {
        Components::iterator b = components_.begin();
        Components::iterator e = components_.end();
        for (Components::iterator i = b; i != e; ++i) {
            delete *i;
        }
    }

    void CiffComponent::add(AutoPtr component)
    {
        doAdd(component);
    }

    void CiffEntry::doAdd(AutoPtr component)
    {
        throw Error(34, "CiffEntry::add");
    } // CiffEntry::doAdd
    
    void CiffDirectory::doAdd(AutoPtr component)
    {
        components_.push_back(component.release());
    } // CiffDirectory::doAdd

    void CiffHeader::read(const byte* pData, uint32_t size)
    {
        if (size < 14) throw Error(33);

        if (pData[0] == 0x49 && pData[1] == 0x49) {
            byteOrder_ = littleEndian;
        }
        else if (pData[0] == 0x4d && pData[1] == 0x4d) {
            byteOrder_ = bigEndian;
        }
        else {
            throw Error(33);
        }
        offset_ = getULong(pData + 2, byteOrder_);
        if (std::memcmp(pData + 6, signature(), 8) != 0) {
            throw Error(33);
        }

        pRootDir_ = new CiffDirectory;
        pRootDir_->readDirectory(pData + offset_, size - offset_, byteOrder_);
    } // CiffHeader::read

    void CiffDirectory::readDirectory(const byte* pData,
                                      uint32_t    size,
                                      ByteOrder   byteOrder)
    {
        uint32_t o = getULong(pData + size - 4, byteOrder);
        if (o + 2 > size) throw Error(33);
        uint16_t count = getUShort(pData + o, byteOrder);
        o += 2;
        for (uint16_t i = 0; i < count; ++i) {
            if (o + 10 > size) throw Error(33);
            uint16_t tag = getUShort(pData + o, byteOrder);
            AutoPtr m;
            switch (CiffComponent::typeId(tag)) {
            case directory: m = AutoPtr(new CiffDirectory); break;
            default: m = AutoPtr(new CiffEntry); break;
            }
            m->setDir(this->tag());
            m->read(pData, size, o, byteOrder);
            add(m);
            o += 10;
        }
    }  // CiffDirectory::readDirectory

    void CiffComponent::read(const byte* pData,
                             uint32_t    size,
                             uint32_t    start,
                             ByteOrder   byteOrder)
    {
        doRead(pData, size, start, byteOrder);
    }

    void CiffComponent::doRead(const byte* pData,
                               uint32_t    size,
                               uint32_t    start,
                               ByteOrder   byteOrder)
    {
        if (size < 10) throw Error(33);
        tag_ = getUShort(pData + start, byteOrder);

        DataLocId dl = dataLocation();
        assert(dl == directoryData || dl == valueData);

        if (dl == valueData) {
            size_   = getULong(pData + start + 2, byteOrder);
            offset_ = getULong(pData + start + 6, byteOrder);
        }
        if (dl == directoryData) {
            size_ = 8;
            offset_ = start + 2;
        }
        pData_ = pData + offset_;
    } // CiffComponent::doRead

    void CiffDirectory::doRead(const byte* pData,
                               uint32_t    size,
                               uint32_t    start,
                               ByteOrder   byteOrder)
    {
        CiffComponent::doRead(pData, size, start, byteOrder);
        readDirectory(pData + offset(), this->size(), byteOrder);
    } // CiffDirectory::doRead

    void CiffHeader::decode(Image& image) const
    {
        // Nothing to decode from the header itself, just add correct byte order
        if (pRootDir_) pRootDir_->decode(image, byteOrder_);
    } // CiffHeader::decode

    void CiffComponent::decode(Image& image, ByteOrder byteOrder) const
    {
        doDecode(image, byteOrder);
    }

    void CiffEntry::doDecode(Image& image, ByteOrder byteOrder) const
    {
        CrwMap::decode(*this, image, byteOrder);
    } // CiffEntry::doDecode

    void CiffDirectory::doDecode(Image& image, ByteOrder byteOrder) const
    {
        Components::const_iterator b = components_.begin();
        Components::const_iterator e = components_.end();
        for (Components::const_iterator i = b; i != e; ++i) {
            (*i)->decode(image, byteOrder);
        }
    } // CiffDirectory::doDecode

    void CiffHeader::write(Blob& blob) const
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
        if (pRootDir_) {
            pRootDir_->write(blob, byteOrder_, offset_);
        }
    }

    uint32_t CiffComponent::write(Blob&     blob, 
                                  ByteOrder byteOrder, 
                                  uint32_t  offset)
    {
        if (remove_) return offset;
        return doWrite(blob, byteOrder, offset);
    }

    uint32_t CiffEntry::doWrite(Blob&     blob, 
                                ByteOrder /*byteOrder*/, 
                                uint32_t  offset)
    {
        return writeValueData(blob, offset);
    } // CiffEntry::doWrite

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

    uint32_t CiffDirectory::doWrite(Blob&     blob, 
                                    ByteOrder byteOrder, 
                                    uint32_t  offset)
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

    void CiffHeader::print(std::ostream& os, const std::string& prefix) const
    {
        os << prefix
           << "Header, offset = 0x" << std::setw(8) << std::setfill('0')
           << std::hex << std::right << offset_ << "\n";
        if (pRootDir_) pRootDir_->print(os, byteOrder_, prefix);
    } // CiffHeader::print

    void CiffComponent::print(std::ostream&      os,
                              ByteOrder          byteOrder,
                              const std::string& prefix) const
    {
        doPrint(os, byteOrder, prefix);
    }

    void CiffComponent::doPrint(std::ostream&      os,
                                ByteOrder          byteOrder,
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

    void CiffDirectory::doPrint(std::ostream&      os,
                                ByteOrder          byteOrder,
                                const std::string& prefix) const
    {
        CiffComponent::doPrint(os, byteOrder, prefix);
        Components::const_iterator b = components_.begin();
        Components::const_iterator e = components_.end();
        for (Components::const_iterator i = b; i != e; ++i) {
            (*i)->print(os, byteOrder, prefix + "   ");
        }
    } // CiffDirectory::doPrint

    void CiffComponent::setValue(DataBuf buf)
    {
        if (isAllocated_) {
            delete pData_;
            pData_ = 0;
            size_ = 0;
        }
        isAllocated_ = true;
        std::pair<byte *, long> p = buf.release();
        pData_ = p.first;
        size_  = p.second;
        if (size_ > 8 && dataLocation() == directoryData) {
            tag_ &= 0x3fff;
        }
    } // CiffComponent::setValue

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

    /*!
      @brief Finds \em crwTag in directory \em crwDir, returning a pointer to
             the component or 0 if not found.

     */
    CiffComponent* CiffHeader::findComponent(uint16_t crwTag,
                                             uint16_t crwDir) const
    {
        if (pRootDir_ == 0) return 0;
        return pRootDir_->findComponent(crwTag, crwDir);
    } // CiffHeader::findComponent

    CiffComponent* CiffComponent::findComponent(uint16_t crwTag, 
                                                uint16_t crwDir) const
    {
        return doFindComponent(crwTag, crwDir);
    } // CiffComponent::findComponent

    CiffComponent* CiffComponent::doFindComponent(uint16_t crwTag, 
                                                  uint16_t crwDir) const
    {
        if (tagId() == crwTag && dir() == crwDir) {
            return const_cast<CiffComponent*>(this);
        }
        return 0;
    } // CiffComponent::doFindComponent

    CiffComponent* CiffDirectory::doFindComponent(uint16_t crwTag, 
                                                  uint16_t crwDir) const 
    {
        CiffComponent* cc = 0;
        const Components::const_iterator b = components_.begin();
        const Components::const_iterator e = components_.end();
        for (Components::const_iterator i = b; i != e; ++i) {
            cc = (*i)->findComponent(crwTag, crwDir);
            if (cc) return cc;
        }
        return 0;
    } // CiffDirectory::doFindComponent

    CiffComponent* CiffHeader::addTag(uint16_t crwTag, uint16_t crwDir)
    {
        CrwDirs crwDirs;
        CrwMap::loadStack(crwDirs, crwDir);
        uint16_t rootDirectory = crwDirs.top().crwDir_;
        assert(rootDirectory == 0x0000);
        crwDirs.pop();
        if (!pRootDir_) pRootDir_ = new CiffDirectory;
        return pRootDir_->addTag(crwDirs, crwTag);
    } // CiffHeader::addTag

    CiffComponent* CiffComponent::addTag(CrwDirs& crwDirs, uint16_t crwTag)
    {
        return doAddTag(crwDirs, crwTag);
    } // CiffComponent::addTag

    CiffComponent* CiffComponent::doAddTag(CrwDirs& crwDirs, uint16_t crwTag)
    {
        return 0;
    } // CiffComponent::doAddTag

    CiffComponent* CiffDirectory::doAddTag(CrwDirs& crwDirs, uint16_t crwTag)
    {
        /*
          addTag()
            if stack not empty
              pop from stack
              find dir among components
              if not found, create it
              addTag()
            else
              find tag among components
              if not found, create it
              set value
        */

        CiffComponent* cc = 0;
        const Components::iterator b = components_.begin();
        const Components::iterator e = components_.end();

        if (!crwDirs.empty()) {
            CrwSubDir csd = crwDirs.top();
            crwDirs.pop();
            // Find the directory
            for (Components::iterator i = b; i != e; ++i) {
                if ((*i)->tag() == csd.crwDir_) {
                    cc = *i;
                    break;
                }
            }
            if (cc == 0) {
                // Directory doesn't exist yet, add it
                AutoPtr m(new CiffDirectory(csd.crwDir_, csd.parent_));
                cc = m.get();
                add(m);
            }
            // Recursive call to next lower level directory
            cc = cc->addTag(crwDirs, crwTag);
        }
        else {
            // Find the tag
            for (Components::iterator i = b; i != e; ++i) {
                if ((*i)->tag() == crwTag) {
                    cc = *i;
                    break;
                }
            }
            if (cc == 0) {
                // Tag doesn't exist yet, add it
                AutoPtr m(new CiffEntry(crwTag, tag()));
                cc = m.get();
                add(m);
            }
        }
        return cc;
    } // CiffDirectory::doAddTag

    void CrwMap::decode(const CiffComponent& ciffComponent,
                        Image&               image,
                        ByteOrder            byteOrder)
    {
        const CrwMapping* cmi = crwMapping(ciffComponent.dir(),
                                           ciffComponent.tagId());
        if (cmi && cmi->toExif_) {
            cmi->toExif_(ciffComponent, cmi, image, byteOrder);
        }
    } // CrwMap::decode

    const CrwMapping* CrwMap::crwMapping(uint16_t dir, uint16_t tagId)
    {
        for (int i = 0; crwMapping_[i].ifdId_ != ifdIdNotSet; ++i) {
            if (   crwMapping_[i].crwDir_ == dir
                && crwMapping_[i].crwTagId_ == tagId) {
                return &(crwMapping_[i]);
            }
        }
        return 0;
    } // CrwMap::crwMapping

    void CrwMap::decode0x0805(const CiffComponent& ciffComponent,
                              const CrwMapping*    /*pCrwMapping*/,
                                    Image&         image,
                                    ByteOrder      /*byteOrder*/)
    {
        std::string s(reinterpret_cast<const char*>(ciffComponent.pData()));
        image.setComment(s);
    } // CrwMap::decode0x0805

    void CrwMap::decode0x080a(const CiffComponent& ciffComponent,
                              const CrwMapping*    pCrwMapping,
                                    Image&         image,
                                    ByteOrder      byteOrder)
    {
        if (ciffComponent.typeId() != asciiString) {
            return decodeBasic(ciffComponent, pCrwMapping, image, byteOrder);
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
                              const CrwMapping*    pCrwMapping,
                                    Image&         image,
                                    ByteOrder      byteOrder)
    {
        if (ciffComponent.typeId() != unsignedShort) {
            return decodeBasic(ciffComponent, pCrwMapping, image, byteOrder);
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
                              const CrwMapping*    pCrwMapping,
                                    Image&         image,
                                    ByteOrder      byteOrder)
    {
        if (ciffComponent.typeId() != unsignedShort) {
            return decodeBasic(ciffComponent, pCrwMapping, image, byteOrder);
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
                              const CrwMapping*    pCrwMapping,
                                    Image&         image,
                                    ByteOrder      byteOrder)
    {
        if (ciffComponent.size() < 8 || ciffComponent.typeId() != unsignedLong) {
            return decodeBasic(ciffComponent, pCrwMapping, image, byteOrder);
        }
        assert(pCrwMapping != 0);
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

            ExifKey key(pCrwMapping->tag_, ExifTags::ifdItem(pCrwMapping->ifdId_));
            AsciiValue value;
            value.read(std::string(s));
            image.exifData().add(key, &value);
        }
    } // CrwMap::decode0x180e

    void CrwMap::decode0x1810(const CiffComponent& ciffComponent,
                              const CrwMapping*    pCrwMapping,
                                    Image&         image,
                                    ByteOrder      byteOrder)
    {
        if (ciffComponent.typeId() != unsignedLong || ciffComponent.size() < 28) {
            return decodeBasic(ciffComponent, pCrwMapping, image, byteOrder);
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
                              const CrwMapping*    /*pCrwMapping*/,
                                    Image&         image,
                                    ByteOrder      /*byteOrder*/)
    {
        image.exifData().setJpegThumbnail(ciffComponent.pData(),
                                          ciffComponent.size());
    } // CrwMap::decode0x2008

    void CrwMap::decodeBasic(const CiffComponent& ciffComponent,
                             const CrwMapping*    pCrwMapping,
                                   Image&         image,
                                   ByteOrder      byteOrder)
    {
        assert(pCrwMapping != 0);
        // create a key and value pair
        ExifKey key(pCrwMapping->tag_, ExifTags::ifdItem(pCrwMapping->ifdId_));
        Value::AutoPtr value;
        if (ciffComponent.typeId() != directory) {
            value = Value::create(ciffComponent.typeId());
            uint32_t size = 0;
            if (pCrwMapping->size_ != 0) {
                // size in the mapping table overrides all
                size = pCrwMapping->size_;
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

    void CrwMap::loadStack(CrwDirs& crwDirs, uint16_t crwDir)
    {
        for (int i = 0; crwSubDir_[i].crwDir_ != 0xffff; ++i) {
            if (crwSubDir_[i].crwDir_ == crwDir) {
                crwDirs.push(crwSubDir_[i]);
                crwDir = crwSubDir_[i].parent_;
            }
        }
    } // CrwMap::loadStack

    void CrwMap::encode(CiffHeader* pHead, const Image& image)
    {
        for (const CrwMapping* cmi = crwMapping_; 
             cmi->ifdId_ != ifdIdNotSet; ++cmi) {
            if (cmi->fromExif_ != 0) {
                cmi->fromExif_(image, cmi, pHead);
            }
        }
    } // CrwMap::encode

    void CrwMap::encodeBasic(const Image&      image,
                             const CrwMapping* pCrwMapping,
                                   CiffHeader* pHead)
    {
        assert(pCrwMapping != 0);
        assert(pHead != 0);
        
        // Determine the source Exif metadatum
        ExifKey ek(pCrwMapping->tag_, ExifTags::ifdItem(pCrwMapping->ifdId_));
        ExifData::const_iterator ed = image.exifData().findKey(ek);

        // Find the target metadatum in the Ciff parse tree
        CiffComponent* cc = pHead->findComponent(pCrwMapping->crwTagId_,
                                                 pCrwMapping->crwDir_);

        if (ed != image.exifData().end()) {
            // Create the directory and component as needed
            if (cc == 0) cc = pHead->addTag(pCrwMapping->crwTagId_,
                                            pCrwMapping->crwDir_);
            // Set the new value
            DataBuf buf(ed->size());
            ed->copy(buf.pData_, pHead->byteOrder());
            cc->setValue(buf);
        }
        else {
            if (cc) cc->remove();
        }
    } // CrwMap::encodeBasic

    void CrwMap::encode0x0805(const Image&      image,
                              const CrwMapping* /*pCrwMapping*/,
                                    CiffHeader* pHead)
    {
        assert(pHead != 0);

        std::string comment = image.comment();
        
        const uint16_t crwTag = 0x0805;
        const uint16_t crwDir = 0x300a;
        CiffComponent* cc = pHead->findComponent(crwTag, crwDir);

        if (!comment.empty()) {
            if (cc == 0) cc = pHead->addTag(crwTag, crwDir);
            DataBuf buf(std::max(cc->size(), comment.size()));
            memset(buf.pData_, 0x0, buf.size_);
            memcpy(buf.pData_, comment.data(), comment.size());
            cc->setValue(buf);
        }
        else {
            if (cc) {
                // Just delete the value, do not remove the tag
                DataBuf buf(cc->size());
                memset(buf.pData_, 0x0, buf.size_);
            }
        }
    } // CrwMap::encode0x0805

    void CrwMap::encode0x080a(const Image&      image,
                              const CrwMapping* /*pCrwMapping*/,
                                    CiffHeader* pHead)
    {
        assert(pHead != 0);

        const ExifKey k1("Exif.Image.Make");
        const ExifKey k2("Exif.Image.Model");
        const ExifData::const_iterator ed1 = image.exifData().findKey(k1);
        const ExifData::const_iterator ed2 = image.exifData().findKey(k2);
        const ExifData::const_iterator edEnd = image.exifData().end();
        
        const uint16_t crwTag = 0x080a;
        const uint16_t crwDir = 0x2807;
        CiffComponent* cc = pHead->findComponent(crwTag, crwDir);

        long size = 0;
        if (ed1 != edEnd) size += ed1->size();
        if (ed2 != edEnd) size += ed2->size();
        if (size != 0) {
            if (cc == 0) cc = pHead->addTag(crwTag, crwDir);
            DataBuf buf(size);
            if (ed1 != edEnd) ed1->copy(buf.pData_, pHead->byteOrder());
            if (ed2 != edEnd) ed2->copy(buf.pData_ + ed1->size(), pHead->byteOrder());
            cc->setValue(buf);
        }
        else {
            if (cc) cc->remove();
        }
    } // encode0x080a

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
