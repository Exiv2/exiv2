// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2021 Exiv2 authors
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

// *****************************************************************************

#define EXIV2_DEBUG_MESSAGES

// included header files
#include "config.h"

#include "image.hpp"
#include "image_int.hpp"
#include "tiffimage.hpp"
#include "bmffimage.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "types.hpp"
#include "safe_op.hpp"
#include "unused.h"

// + standard includes
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>
#include <cstdio>

struct BmffBoxHeader
{
    uint32_t length;
    uint32_t type;
};

#define TAG_ftyp 0x66747970 /**< "ftyp" File type box */
#define TAG_avif 0x61766966 /**< "avif" AVIF */
#define TAG_heic 0x68656963 /**< "heic" HEIF */
#define TAG_heif 0x68656966 /**< "heif" HEIF */
#define TAG_crx  0x63727820 /**< "crx " Canon CR3 */
#define TAG_moov 0x6d6f6f76 /**< "moov" Movie */
#define TAG_meta 0x6d657461 /**< "meta" Metadata */
#define TAG_mdat 0x6d646174 /**< "mdat" Media data */
#define TAG_uuid 0x75756964 /**< "uuid" UUID */
#define TAG_dinf 0x64696e66 /**< "dinf" Data information */
#define TAG_iprp 0x69707270 /**< "iprp" Item properties */
#define TAG_ipco 0x6970636f /**< "ipco" Item property container */
#define TAG_iinf 0x69696e66 /**< "iinf" Item info */
#define TAG_iloc 0x696c6f63 /**< "iloc" Item location */
#define TAG_ispe 0x69737065 /**< "ispe" Image spatial extents */
#define TAG_infe 0x696e6665 /**< "infe" */

// *****************************************************************************
// class member definitions
namespace Exiv2
{
    static bool enabled = false;

    EXIV2API bool enableBMFF(bool enable)
    {
#ifdef EXV_ENABLE_BMFF
        enabled = enable;
        return true;
#endif // EXV_ENABLE_BMFF
        enable = false; // unused
        return enable;
    }

    BmffImage::BmffImage(BasicIo::AutoPtr io, bool /* create */)
            : Image(ImageType::bmff, mdExif | mdIptc | mdXmp, io)
    {
    } // BmffImage::BmffImage

    BmffImage::BmffImage(BasicIo::AutoPtr io, size_t start, size_t count)
        : Image(ImageType::bmff, mdExif | mdIptc | mdXmp, io)
    {
        UNUSED(start);
        UNUSED(count);
    } // BmffImage::BmffImage

    std::string BmffImage::toAscii(long n)
    {
        const char* p = (const char*) &n;
        std::string result;
        bool bBigEndian = isBigEndianPlatform();
        for ( int i = 0 ; i < 4 ; i++) {
            result += p[ bBigEndian ? i : (3-i) ];
        }
        return result;
    }

    std::string BmffImage::boxName(uint32_t box)
    {
        char           name[5];
        std::memcpy   (name,&box,4);
        name[4] = 0   ;
        return std::string(name) ;
    }

    bool BmffImage::superBox(uint32_t box)
    {
        return      box == TAG_moov
                ||  box == TAG_dinf
                ||  box == TAG_iprp
                ||  box == TAG_ipco
                ||  box == TAG_meta
                ||  box == TAG_iinf
                ||  box == TAG_iloc
        ;
    }

    bool BmffImage::fullBox(uint32_t box)
    {
        return      box == TAG_meta
                ||  box == TAG_iinf
                ||  box == TAG_iloc
        ;
    }

    std::string BmffImage::mimeType() const
    {
        switch (fileType)
        {
            case TAG_avif: return "image/avif";
            case TAG_heic:
            case TAG_heif: return "image/heif";
            case TAG_crx : return "image/x-canon-cr3";
            default      : return "image/generic";
        }
    }

    long BmffImage::boxHandler(int indent /* =0 */)
    {
        long          result  = (long) io_->size();
        long          address = (long) io_->tell();
        BmffBoxHeader box     = {0,0};

        if ( io_->read((byte*)&box, sizeof(box)) != sizeof(box)) return result;

        box.length = getLong((byte*)&box.length, bigEndian);
        box.type   = getLong((byte*)&box.type, bigEndian);
#ifdef EXIV2_DEBUG_MESSAGES
        bool bLF = true;
        std::cout << indenter(indent) << "Exiv2::BmffImage::boxHandler: " << toAscii(box.type)
                  << Internal::stringFormat(" %8ld->%u ",address,box.length)
        ;
#endif
        // TODO: This isn't right.  We should check the visits earlier.
        // TAG_mdat should not be processed twice
        if ( box.type == TAG_mdat ) {
            std::cout << std::endl;
            return result ;
        }
        if ( visits_.find(address) != visits_.end() || visits_.size() > visits_max_ ) {
            throw Error(kerCorruptedMetadata);
        }
        visits_.insert(address);

        if ( box.length == 1 ) {
            DataBuf data(8);
            io_->read(data.pData_,data.size_  );
            result = address + (long) getULongLong(data.pData_,littleEndian);
        }

        // read data in box and restore file position
        long      restore = io_->tell();
        DataBuf   data(box.length-8);
        io_->read(data.pData_,data.size_  );
        io_->seek(restore    ,BasicIo::beg);

        uint32_t skip    = 0 ;
        uint8_t  version = 0 ;
        uint32_t flags   = 0 ;

        if ( fullBox(box.type) ) {
            flags    = getLong(data.pData_+skip,bigEndian) ; // version/flags
            version  = (int8_t ) flags >> 24 ;
            version &= 0x00ffffff ;
            skip    += 4 ;
        }

        switch (box.type)
        {
            case TAG_ftyp:
            {
                fileType = getLong(data.pData_, bigEndian);
#ifdef EXIV2_DEBUG_MESSAGES
                std::cout << "Brand: " << toAscii(fileType);
#endif
            } break;


            // 8.11.6.1
            case TAG_iinf:
            {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cout << std::endl;
                bLF=false;
#endif

                int n = getShort(data.pData_+skip,bigEndian) ;
                skip+= 2 ;

                io_->seek(skip,BasicIo::cur);
                while ( n-- > 0 )
                    io_->seek(boxHandler(indent+1),BasicIo::beg);
            } break;

            // 8.11.6.2
            case TAG_infe : { // .__._.__hvc1_ 2 0 0 1 0 1 0 0 104 118 99 49 0
                              /* getLong (data.pData_+skip,bigEndian) ; */ skip+=4;
                uint16_t   ID =  getShort(data.pData_+skip,bigEndian) ;    skip+=2;
                              /* getShort(data.pData_+skip,bigEndian) ; */ skip+=2; // protection
                std::string name((const char*)data.pData_+skip);
                if ( name.find("Exif")== 0 || name.find("Exif")== 0 ) { // "Exif" or "ExifExif"
                    exifID_ = ID ;
                }
#ifdef EXIV2_DEBUG_MESSAGES
                std::cout << Internal::stringFormat("%3d ",ID) << name << " ";
#endif
            } break;

            case TAG_iprp:
            case TAG_ipco:
            case TAG_meta: {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cout << std::endl;
                bLF=false;
#endif
                io_->seek(skip,BasicIo::cur);
                while ( (long) io_->tell() < (long)(address + skip + box.length) ) {
                    io_->seek(boxHandler(indent+1),BasicIo::beg);
                }
            } break;

            // 8.11.3.1
            case TAG_iloc: {
                uint8_t  u          = data.pData_[skip++];
                uint16_t offsetSize = u >> 4  ;
                uint16_t lengthSize = u & 0xF ;
#if 0
                uint16_t indexSize  = 0       ;
                u             = data.pData_[skip++];
                if ( version == 1 || version == 2 ) {
                    indexSize = u & 0xF ;
                }
#endif
                uint32_t itemCount  = version < 2 ? getShort(data.pData_+skip,bigEndian) : getLong(data.pData_+skip,bigEndian);
                skip               += version < 2 ?               2                      :         4                          ;
                if ( itemCount && offsetSize == 4 && lengthSize == 4 && ((box.length-16) % itemCount) == 0 ) {
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cout << std::endl;
                    bLF=false;
#endif
                    uint32_t step = (box.length-16)/itemCount                  ; // length of data per item.
                    uint32_t base = skip;
                    for ( uint32_t i = 0 ; i < itemCount ; i++ ) {
                        skip=base+i*step ; // move in 16 or 14 byte steps
                        uint32_t ID     = version > 2 ? getLong(data.pData_+skip,bigEndian) : getShort(data.pData_+skip,bigEndian);
                        uint32_t offset = getLong(data.pData_+skip+step-8,bigEndian);
                        uint32_t ldata  = getLong(data.pData_+skip+step-4,bigEndian);
#ifdef EXIV2_DEBUG_MESSAGES
                        std::cout << indenter(indent)
                                  << Internal::stringFormat("%8ld | %8u |  ext | %4u | %6u,%6u",address+skip,step,ID,offset,ldata)
                                  << std::endl
                        ;
#endif
                        if ( ID == exifID_) {
                            exifStart_  = offset;
                            exifLength_ = ldata;
                        }
                    }
                }
            } break;

            case TAG_ispe: {
#ifdef EXIV2_DEBUG_MESSAGES
                pixelWidth_  = getLong(data.pData_ + skip, bigEndian); skip+=4;
                pixelHeight_ = getLong(data.pData_ + skip, bigEndian); skip+=8;
                std::cout << "pixelWidth, pixelHeight_ = "
                          << Internal::stringFormat("%d,%d",pixelWidth_, pixelHeight_)
                ;
#endif
            } break;

            case TAG_mdat: {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cout << "MDAT" ;
#endif
            } break;

            default: {} ; /* do nothing */
        }
#ifdef EXIV2_DEBUG_MESSAGES
        if ( bLF ) std::cout  << std::endl;
        if ( exifID_ != unknownID_ && exifStart_ && exifLength_ ) {
            std::cout << indenter(indent) << Internal::stringFormat("Exif: %d->%d",exifStart_,exifLength_) << std::endl;
            exifID_ = unknownID_;
        }
#endif

        // return address of next box
        if ( box.length != 1 ) result = static_cast<long>(address + box.length);

        return result ;
    }

    void BmffImage::setComment(const std::string& /*comment*/)
    {
        // Todo: implement me!
        throw(Error(kerInvalidSettingForImage, "Image comment", "BMFF"));
    } // BmffImage::setComment

    void BmffImage::readMetadata()
    {
        if (io_->open() != 0)
        {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isBmffType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(kerFailedToReadImageData);
            throw Error(kerNotAnImage, "BMFF");
        }

        visits_.clear();
        visits_max_ = io_->size() / 16;

        unknownID_  = 0xffff     ;
        exifID_     = unknownID_ ;
        exifStart_  = 0;
        exifLength_ = 0;

        long      address = 0 ;
        while (   address < (long) io_->size() ) {
            io_->seek(address,BasicIo::beg);
            address = boxHandler();
        }

    } // BmffImage::readMetadata

    void BmffImage::printStructure(std::ostream& out, PrintStructureOption option, int depth)
    {
        if (io_->open() != 0)
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());

        // Ensure that this is the correct image type
        if (!isBmffType(*io_, false)) {
            if (io_->error() || io_->eof())
                throw Error(kerFailedToReadImageData);
            throw Error(kerNotAnImage);
        }
        UNUSED(out);
        UNUSED(option);
        UNUSED(depth);
    } // BmffImage::printStructure

    void BmffImage::writeMetadata()
    {
    } // BmffImage::writeMetadata

    // *************************************************************************
    // free functions
    Image::AutoPtr newBmffInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new BmffImage(io, create));
        if (!image->good())
        {
            image.reset();
        }
        return image;
    }

    bool isBmffType(BasicIo& iIo, bool advance)
    {
        if (!enabled)
        {
            return false;
        }
        const int32_t len = 12;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }

        bool matched = buf[4] == 'f' && buf[5] == 't' && buf[6] == 'y' && buf[7] == 'p';
        if (!advance || !matched) {
            iIo.seek(static_cast<long>(0), BasicIo::beg);
        }
        return matched;
    }
}                                       // namespace Exiv2
