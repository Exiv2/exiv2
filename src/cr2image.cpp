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
  File:      cr2image.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   22-Apr-06, ahu: created

 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "cr2image.hpp"
#include "cr2image_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"
#include "tiffvisitor_int.hpp"
#include "image.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    Cr2Image::Cr2Image(BasicIo::AutoPtr io, bool /*create*/)
        : Image(ImageType::cr2, mdExif | mdIptc, io)
    {
    } // Cr2Image::Cr2Image

    int Cr2Image::pixelWidth() const
    {
        Exiv2::ExifData::const_iterator widthIter = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
        return (widthIter == exifData_.end()) ? 0 : widthIter->toLong();
    }

    int Cr2Image::pixelHeight() const
    {
        Exiv2::ExifData::const_iterator heightIter = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
        return (heightIter == exifData_.end()) ? 0 : heightIter->toLong();
    }

    void Cr2Image::setExifData(const ExifData& /*exifData*/)
    {
        // Todo: implement me!
        throw(Error(32, "Exif metadata", "CR2"));
    }

    void Cr2Image::setIptcData(const IptcData& /*iptcData*/)
    {
        // Todo: implement me!
        throw(Error(32, "IPTC metadata", "CR2"));
    }

    void Cr2Image::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "CR2"));
    }

    void Cr2Image::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading CR2 file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isCr2Type(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "CR2");
        }
        clearMetadata();
        ByteOrder bo = Cr2Parser::decode(exifData_,
                                         iptcData_,
                                         xmpData_,
                                         io_->mmap(),
                                         io_->size());
        setByteOrder(bo);
    } // Cr2Image::readMetadata

    void Cr2Image::writeMetadata()
    {
        // Todo: implement me!
        throw(Error(31, "CR2"));
    } // Cr2Image::writeMetadata

    ByteOrder Cr2Parser::decode(
              ExifData& exifData,
              IptcData& iptcData,
              XmpData&  xmpData,
        const byte*     pData,
              uint32_t  size
    )
    {
        Cr2Header cr2Header;
        return TiffParserWorker::decode(exifData,
                                        iptcData,
                                        xmpData,
                                        pData,
                                        size,
                                        TiffCreator::create,
                                        Cr2Mapping::findDecoder,
                                        &cr2Header);
    }

    WriteMethod Cr2Parser::encode(
              Blob&     blob,
        const byte*     pData,
              uint32_t  size,
        const ExifData& exifData,
        const IptcData& iptcData,
        const XmpData&  xmpData
    )
    {
        /* Todo: Implement me!

        TiffParserWorker::encode(blob,
                                 pData,
                                 size,
                                 exifData,
                                 iptcData,
                                 xmpData,
                                 TiffCreator::create,
                                 TiffMapping::findEncoder);
        */
        blob.clear();
        return wmIntrusive;
    }

    // *************************************************************************
    // free functions
    Image::AutoPtr newCr2Instance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new Cr2Image(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isCr2Type(BasicIo& iIo, bool advance)
    {
        const int32_t len = 16;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        Cr2Header header;
        bool rc = header.read(buf, len);
        if (!advance || !rc) {
            iIo.seek(-len, BasicIo::cur);
        }
        return rc;
    }

}                                       // namespace Exiv2

namespace Exiv2 {
    namespace Internal {

    // CR2 mapping table for special CR2 decoding requirements
    const TiffMappingInfo Cr2Mapping::cr2MappingInfo_[] = {
        { "*",       Tag::all, Group::ignr,    0, 0 }, // Do not decode tags with group == Group::ignr
        { "*",         0x014a, Group::ifd0,    0, 0 }, // Todo: Controversial, causes problems with Exiftool
        { "*",         0x0100, Group::ifd0,    0, 0 }, // CR2 IFD0 refers to a preview image, ignore these tags
        { "*",         0x0101, Group::ifd0,    0, 0 },
        { "*",         0x0102, Group::ifd0,    0, 0 },
        { "*",         0x0103, Group::ifd0,    0, 0 },
        { "*",         0x0111, Group::ifd0,    0, 0 },
        { "*",         0x0117, Group::ifd0,    0, 0 },
        { "*",         0x011a, Group::ifd0,    0, 0 },
        { "*",         0x011b, Group::ifd0,    0, 0 },
        { "*",         0x0128, Group::ifd0,    0, 0 },
        { "*",         0x02bc, Group::ifd0,    &TiffDecoder::decodeXmp,    &TiffEncoder::encodeXmp    },
        { "*",         0x83bb, Group::ifd0,    &TiffDecoder::decodeIptc,   &TiffEncoder::encodeIptc   },
        { "*",         0x8649, Group::ifd0,    &TiffDecoder::decodeIptc,   &TiffEncoder::encodeIptc   }
    };

    DecoderFct Cr2Mapping::findDecoder(const std::string& make,
                                             uint32_t     extendedTag,
                                             uint16_t     group)
    {
        DecoderFct decoderFct = &TiffDecoder::decodeStdTiffEntry;
        const TiffMappingInfo* td = find(cr2MappingInfo_,
                                         TiffMappingInfo::Key(make, extendedTag, group));
        if (td) {
            // This may set decoderFct to 0, meaning that the tag should not be decoded
            decoderFct = td->decoderFct_;
        }
        return decoderFct;
    }

    EncoderFct Cr2Mapping::findEncoder(const std::string& make,
                                             uint32_t     extendedTag,
                                             uint16_t     group)
    {
        EncoderFct encoderFct = 0;
        const TiffMappingInfo* td = find(cr2MappingInfo_,
                                         TiffMappingInfo::Key(make, extendedTag, group));
        if (td) {
            // Returns 0 if no special encoder function is found
            encoderFct = td->encoderFct_;
        }
        return encoderFct;
    }

    const char* Cr2Header::cr2sig_ = "CR\2\0";

    Cr2Header::Cr2Header()
        : TiffHeaderBase(42, 16, littleEndian, 0x00000010),
          offset2_(0x00000000)
    {
    }

    Cr2Header::~Cr2Header()
    {
    }

    bool Cr2Header::read(const byte* pData, uint32_t size)
    {
        if (size < 16) return false;

        if (pData[0] == 0x49 && pData[1] == 0x49) {
            setByteOrder(littleEndian);
        }
        else if (pData[0] == 0x4d && pData[1] == 0x4d) {
            setByteOrder(bigEndian);
        }
        else {
            return false;
        }
        if (tag() != getUShort(pData + 2, byteOrder())) return false;
        setOffset(getULong(pData + 4, byteOrder()));
        if (0 != memcmp(pData + 8, cr2sig_, 4)) return false;
        offset2_ = getULong(pData + 12, byteOrder());

        return true;
    } // Cr2Header::read

    uint32_t Cr2Header::write(Blob& blob) const
    {
        // Todo: Implement me!
        return 0;
    }

}}                                      // namespace Internal, Exiv2
