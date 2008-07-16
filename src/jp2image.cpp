// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  File:      jp2image.cpp
  Version:   $Rev$
  Author(s): Marco Piovanelli, Ovolab (marco)
  History:   12-Mar-2007, marco: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

//#define DEBUG 1

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif
#include "jp2image.hpp"
#include "image.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <string>
#include <cstring>
#include <iostream>

// JPEG-2000 box types
const uint32_t kJp2BoxTypeJp2Header   = 0x6a703268; // 'jp2h'
const uint32_t kJp2BoxTypeImageHeader = 0x69686472; // 'ihdr'
const uint32_t kJp2BoxTypeUuid        = 0x75756964; // 'uuid'

// JPEG-2000 UUIDs for embedded metadata
//
// See http://www.jpeg.org/public/wg1n2600.doc for information about embedding IPTC-NAA data in JPEG-2000 files
// See http://www.adobe.com/devnet/xmp/pdfs/xmp_specification.pdf for information about embedding XMP data in JPEG-2000 files
const char* const kJp2UuidExif = "JpgTiffExif->JP2";
const char* const kJp2UuidIptc = "\x33\xc7\xa4\xd2\xb8\x1d\x47\x23\xa0\xba\xf1\xa3\xe0\x97\xad\x38";
const char* const kJp2UuidXmp  = "\xbe\x7a\xcf\xcb\x97\xa9\x42\xe8\x9c\x71\x99\x94\x91\xe3\xaf\xac";

//! @cond IGNORE
struct Jp2BoxHeader {
    uint32_t boxLength;
    uint32_t boxType;
};

struct Jp2ImageHeaderBox {
    uint32_t imageHeight;
    uint32_t imageWidth;
    uint16_t componentCount;
    uint8_t  bitsPerComponent;
    uint8_t  compressionType;
    uint8_t  colorspaceIsUnknown;
    uint8_t  intellectualPropertyFlag;
    uint16_t compressionTypeProfile;
};

struct Jp2UuidBox {
    uint8_t  uuid[16];
};
//! @endcond

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    Jp2Image::Jp2Image(BasicIo::AutoPtr io)
        : Image(ImageType::jp2, mdExif | mdIptc | mdXmp, io)
    {
    } // Jp2Image::Jp2Image

    void Jp2Image::setExifData(const ExifData& /*exifData*/)
    {
        // Todo: implement me!
        throw(Error(32, "Exif metadata", "JP2"));
    }

    void Jp2Image::setIptcData(const IptcData& /*iptcData*/)
    {
        // Todo: implement me!
        throw(Error(32, "IPTC metadata", "JP2"));
    }

    void Jp2Image::setComment(const std::string& /*comment*/)
    {
        // Todo: implement me!
        throw(Error(32, "Image comment", "JP2"));
    }

    void Jp2Image::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Exiv2::Jp2Image::readMetadata: Reading JPEG-2000 file " << io_->path() << "\n";
#endif
        if (io_->open() != 0)
        {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isJp2Type(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "JPEG-2000");
        }

        Jp2BoxHeader      box       = {0,0};
        Jp2BoxHeader      subBox    = {0,0};
        Jp2ImageHeaderBox ihdr      = {0,0,0,0,0,0,0,0};
        Jp2UuidBox        uuid      = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
        long              curOffset = io_->tell();

        while (io_->read((byte*)&box, sizeof(box)) == sizeof(box))
        {
            box.boxLength = getLong((byte*)&box.boxLength, bigEndian);
            box.boxType = getLong((byte*)&box.boxType, bigEndian);

            if (box.boxLength == 0)
            {
                break;
            }

            switch(box.boxType)
            {
                case kJp2BoxTypeJp2Header:
                {
                    if (io_->read((byte*)&subBox, sizeof(subBox)) == sizeof(subBox))
                    {
                        subBox.boxLength = getLong((byte*)&subBox.boxLength, bigEndian);
                        subBox.boxType = getLong((byte*)&subBox.boxType, bigEndian);

                        if((subBox.boxType == kJp2BoxTypeImageHeader) && (io_->read((byte*)&ihdr, sizeof(ihdr)) == sizeof(ihdr)))
                        {
                            ihdr.imageHeight = getLong((byte*)&ihdr.imageHeight, bigEndian);
                            ihdr.imageWidth = getLong((byte*)&ihdr.imageWidth, bigEndian);
                            ihdr.componentCount = getShort((byte*)&ihdr.componentCount, bigEndian);
                            ihdr.compressionTypeProfile = getShort((byte*)&ihdr.compressionTypeProfile, bigEndian);

                            pixelWidth_ = ihdr.imageWidth;
                            pixelHeight_ = ihdr.imageHeight;
                        }
                    }
                    break;
                }

                case kJp2BoxTypeUuid:
                {
                    if (io_->read((byte*)&uuid, sizeof(uuid)) == sizeof(uuid))
                    {
                        if(memcmp(uuid.uuid, kJp2UuidExif, sizeof(uuid)) == 0)
                        {
                            // we've hit an embedded Exif block
                            DataBuf rawExif(box.boxLength - (sizeof(box) + sizeof(uuid)));
                            io_->read(rawExif.pData_, rawExif.size_);
                            if (io_->error() || io_->eof()) throw Error(14);
                            ByteOrder bo = ExifParser::decode(exifData_, rawExif.pData_, rawExif.size_);
                            setByteOrder(bo);
                            if (rawExif.size_ > 0 && byteOrder() == invalidByteOrder) {
#ifndef SUPPRESS_WARNINGS
                                std::cerr << "Warning: Failed to decode Exif metadata.\n";
#endif
                                exifData_.clear();
                            }
                        }
                        else if(memcmp(uuid.uuid, kJp2UuidIptc, sizeof(uuid)) == 0)
                        {
                            // we've hit an embedded IPTC block
                            DataBuf rawIPTC(box.boxLength - (sizeof(box) + sizeof(uuid)));
                            io_->read(rawIPTC.pData_, rawIPTC.size_);
                            if (io_->error() || io_->eof()) throw Error(14);
                            if (IptcParser::decode(iptcData_, rawIPTC.pData_, rawIPTC.size_)) {
#ifndef SUPPRESS_WARNINGS
                                std::cerr << "Warning: Failed to decode IPTC metadata.\n";
#endif
                                iptcData_.clear();
                            }
                        }
                        else if(memcmp(uuid.uuid, kJp2UuidXmp, sizeof(uuid)) == 0)
                        {
                            // we've hit an embedded XMP block
                            DataBuf xmpPacket(box.boxLength - (sizeof(box) + sizeof(uuid)));
                            io_->read(xmpPacket.pData_, xmpPacket.size_);
                            if (io_->error() || io_->eof()) throw Error(14);
                            xmpPacket_.assign(reinterpret_cast<char *>(xmpPacket.pData_), xmpPacket.size_);
                            if (xmpPacket_.size() > 0 && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
                                std::cerr << "Warning: Failed to decode XMP metadata.\n";
#endif
                            }
                        }
                    }
                    break;
                }

                default:
                {
                    break;
                }
            }

            curOffset += box.boxLength;
            if(io_->seek(curOffset, BasicIo::beg) != 0)
            {
                break;        // Todo: should throw an error here
            }
        }
    } // Jp2Image::readMetadata

    void Jp2Image::writeMetadata()
    {
        // Todo: implement me!
        throw(Error(31, "JP2"));
    } // Jp2Image::writeMetadata

    // *************************************************************************
    // free functions
    Image::AutoPtr newJp2Instance(BasicIo::AutoPtr io, bool /*create*/)
    {
        Image::AutoPtr image(new Jp2Image(io));
        if (!image->good())
        {
            image.reset();
        }
        return image;
    }

    bool isJp2Type(BasicIo& iIo, bool advance)
    {
        // see section B.1.1 (JPEG 2000 Signature box) of JPEG-2000 specification
        const int32_t len = 12;
        const unsigned char Jp2Header[len] = { 0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50, 0x20, 0x20, 0x0d, 0x0a, 0x87, 0x0a };
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof())
        {
            return false;
        }
        bool matched = (memcmp(buf, Jp2Header, len) == 0);
        if (!advance || !matched)
        {
            iIo.seek(-len, BasicIo::cur);
        }
        return matched;
    }
}                                       // namespace Exiv2
