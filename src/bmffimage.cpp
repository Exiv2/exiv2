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

#include "bmffimage.hpp"
#include "tiffimage.hpp"
#include "image.hpp"
#include "image_int.hpp"
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

    std::string boxName(uint32_t box)
    {
        char           name[5];
        std::memcpy   (name,&box,4);
        name[4] = 0   ;
        return std::string(name) ;
    }

    static void boxes_check(size_t b, size_t m)
    {
        if ( b > m ) {
#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::BmffImage::readMetadata box maximum exceeded" << std::endl;
#endif
            throw Error(kerCorruptedMetadata);
        }
    }

    bool superBox(uint32_t box)
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

    bool fullBox(uint32_t box)
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
            case TAG_avif:
                return "image/avif";

            case TAG_heic:
            case TAG_heif:
                return "image/heif";

            case TAG_crx:
                return "image/x-canon-cr3";

            default:
                return "image/generic";
        }
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

        long            position = 0;
        BmffBoxHeader   box      = { 0, 0 };
        size_t          boxes    = 0 ;
        size_t          boxem    = 1000 ; // boxes max
        uint64_t        address  = position;
        uint64_t        length   = 8;
        uint32_t        type;

        while (io_->read((byte*)&box, sizeof(box)) == sizeof(box))
        {
            boxes_check(boxes++, boxem);
            position = io_->tell();
            length   = getLong((byte*)&box.length, bigEndian);
            type     = getLong((byte*)&box.type, bigEndian);
#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::BmffImage::readMetadata: "
                      << "Position: " << position
                      << " box type: " << toAscii(type)
                      << " box length: " << length
                      << std::endl;
#endif
            if (length == 0)
            {
                return;
            }

            if (length == 1)
            {
                DataBuf data(sizeof(length));
                io().read(data.pData_, data.size_);
                length = getULongLong(data.pData_, bigEndian);
#ifdef EXIV2_DEBUG_MESSAGES
                std::cout << "Exiv2::BmffImage::readMetadata: "
                        << "length " << length
                        << std::endl;
#endif
            }

            if ((length > 8) && (static_cast<size_t>(position) + length) <= io().size())
            {
                switch (type)
                {
                    case TAG_ftyp:
                    {
                        DataBuf data(static_cast<size_t>(length));
                        io().read(data.pData_, data.size_);
                        fileType = getLong(data.pData_, bigEndian);
#ifdef EXIV2_DEBUG_MESSAGES
                        std::string brand_ = toAscii(fileType);
                        std::cout << "Exiv2::BmffImage::readMetadata: "
                                << "Brand: " << brand_
                                << std::endl;
#endif
                        break;
                    }

                    case TAG_meta:
                    {
                        DataBuf data(static_cast<size_t>(length));
                        io().read(data.pData_, data.size_);
#ifdef EXIV2_DEBUG_MESSAGES
                        std::cout << "Exiv2::BmffImage::readMetadata: metadata "
                                << data.size_ << " bytes"
                                << std::endl;
#endif
                        break;
                    }

                    case TAG_ispe:
                    {
                        DataBuf data(static_cast<size_t>(length));
                        io().read(data.pData_, data.size_);

                        uint32_t flags = getLong(data.pData_, bigEndian);
                        uint8_t version = (uint8_t)(flags >> 24);
                        flags &= 0x00ffffff;
                        pixelWidth_  = getLong(data.pData_ + 4, bigEndian);
                        pixelHeight_ = getLong(data.pData_ + 8, bigEndian);
#ifdef EXIV2_DEBUG_MESSAGES
                        std::cout << "Exiv2::BmffImage::readMetadata: Image spatial extents "
                                << "version: " << version
                                << "flags: " << flags
                                << std::endl;
#endif
                        break;
                    }

                    default:
                    {
#ifdef EXIV2_DEBUG_MESSAGES
                        std::cout << "Exiv2::BmffImage::readMetadata: box type: " << toAscii(type)
                                << " box length: " << length
                                << std::endl;
#endif
                        break;
                    }
                }
            }

            // Move to the next box.
            io_->seek(static_cast<long>(position - sizeof(box) + length), BasicIo::beg);
            if (io_->error())
            {
                throw Error(kerFailedToReadImageData);
            }
        }
        UNUSED(address);
    } // BmffImage::readMetadata

    void BmffImage::printStructure(std::ostream& out, PrintStructureOption option, int depth)
    {
        if (io_->open() != 0)
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());

        // Ensure that this is the correct image type
        if (!isBmffType(*io_, false))
        {
            if (io_->error() || io_->eof())
            {
                throw Error(kerFailedToReadImageData);
            }
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
