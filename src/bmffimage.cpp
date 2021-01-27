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

#if defined(__BIG_ENDIAN__)
#define ID(string) ((string[0] << 24) | (string[1] << 16) | (string[2] << 8) | string[3])
#elif defined(__LITTLE_ENDIAN__)
#define ID(string) (string[0] | (string[1] << 8) | (string[2] << 16) | (string[3] << 24))
#else
#error "Unknown endian"
#endif

static const uint32_t ftyp = ID("ftyp");    /**< File type box */
static const uint32_t avif = ID("avif");    /**< AVIF */
static const uint32_t heic = ID("heic");    /**< HEIF */
static const uint32_t heif = ID("heif");    /**< HEIF */
static const uint32_t crx  = ID("crx ");    /**< Canon CR3 */
static const uint32_t moov = ID("moov");    /**< Movie */
static const uint32_t meta = ID("meta");    /**< Metadata */
static const uint32_t mdat = ID("mdat");    /**< Media data */
static const uint32_t uuid = ID("uuid");    /**< UUID */
static const uint32_t dinf = ID("dinf");    /**< Data information */
static const uint32_t iprp = ID("iprp");    /**< Item properties */
static const uint32_t ipco = ID("ipco");    /**< Item property container */
static const uint32_t iinf = ID("iinf");    /**< Item info */
static const uint32_t iloc = ID("iloc");    /**< Item location */
static const uint32_t ispe = ID("ispe");    /**< Image spatial extents */

// *****************************************************************************
// class member definitions
namespace Exiv2
{
    static bool enabled = false;

    EXIV2API bool enableISOBMFF(bool enable)
    {
#ifdef EXV_ENABLE_ISOBMFF
        enabled = enable;
        return true;
#endif // EXV_ENABLE_ISOBMFF
        enable = false; // unused
        return enable;
    }

    BmffImage::BmffImage(BasicIo::AutoPtr io, bool /* create */)
            : Image(ImageType::bmff, mdExif | mdIptc | mdXmp, io)
    {
    } // BmffImage::BmffImage

    static bool isBigEndian()
    {
        union {
            uint32_t i;
            char c[4];
        } e = { 0x01000000 };

        return e.c[0]?true:false;
    }

    static std::string toAscii(long n)
    {
        const char* p = (const char*) &n;
        std::string result;
        bool bBigEndian = isBigEndian();
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
        return      box == moov
                ||  box == dinf
                ||  box == iprp
                ||  box == ipco
                ||  box == meta
                ||  box == iinf
                ||  box == iloc
        ;
    }
    bool fullBox(uint32_t box)
    {
        return      box == meta
                ||  box == iinf
                ||  box == iloc
        ;
    }

    std::string BmffImage::mimeType() const
    {
        switch (fileType)
        {
            case avif:
                return "image/avif";

            case heic:
            case heif:
                return "image/heif";

            case crx:
                return "image/x-canon-cr3";

            default:
                return "image/generic";
        }
    }

    void BmffImage::setComment(const std::string& /*comment*/)
    {
        // Todo: implement me!
        throw(Error(kerInvalidSettingForImage, "Image comment", "ISO BMFF"));
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
            throw Error(kerNotAnImage, "ISO BMFF");
        }

        long              position  = 0;
        BmffBoxHeader     box       = {0,0};
        BmffBoxHeader     subBox    = {0,0};
        size_t            boxes     = 0 ;
        size_t            boxem     = 1000 ; // boxes max

        while (io_->read((byte*)&box, sizeof(box)) == sizeof(box))
        {
            boxes_check(boxes++, boxem);
            position   = io_->tell();
            uint32_t length = getLong((byte*)&box.length, bigEndian);
            uint32_t type   = getLong((byte*)&box.type, bigEndian);
#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::BmffImage::readMetadata: "
                      << "Position: " << position
                      << " box type: " << toAscii(type)
                      << " length: " << length
                      << std::endl;
#endif

            if (length == 0) return ;

            if (length == 1)
            {
            }

            switch (box.type)
            {
                case ftyp:
                {
                    io().read((byte*)&fileType,4);
                    std::string brand_ = boxName(fileType);
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cout << "Exiv2::BmffImage::readMetadata: "
                            << "Brand: " << brand_
                            << std::endl;
#endif
                    break;
                }

                case meta:
                {
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cout << "Exiv2::BmffImage::readMetadata: metadata"
                            << std::endl;
#endif
                    break;
                }

                default:
                {
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cout << " box type: " << toAscii(type)
                            << " length: " << length
                            << std::endl;
#endif
                    break;
                }
            }

            // Move to the next box.
            io_->seek(static_cast<long>(position - sizeof(box) + length), BasicIo::beg);
            if (io_->error())
                throw Error(kerFailedToReadImageData);
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
