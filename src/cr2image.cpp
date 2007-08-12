// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006-2007 Andreas Huggel <ahuggel@gmx.net>
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
#include "tiffcomposite.hpp"
#include "tiffparser.hpp"
#include "tiffvisitor.hpp"
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

    // CR2 decoder table for special CR2 decoding requirements
    const TiffDecoderInfo Cr2Decoder::cr2DecoderInfo_[] = {
        { "*",       Tag::all, Group::ignr,    0 }, // Do not decode tags with group == Group::ignr
        { "*",         0x014a, Group::ifd0,    0 }, // Todo: Controversial, causes problems with Exiftool
        { "*",         0x0100, Group::ifd0,    0 }, // CR2 IFD0 refers to a preview image, ignore these tags
        { "*",         0x0101, Group::ifd0,    0 },
        { "*",         0x0102, Group::ifd0,    0 },
        { "*",         0x0103, Group::ifd0,    0 },
        { "*",         0x0111, Group::ifd0,    0 },
        { "*",         0x0117, Group::ifd0,    0 },
        { "*",         0x011a, Group::ifd0,    0 },
        { "*",         0x011b, Group::ifd0,    0 },
        { "*",         0x0128, Group::ifd0,    0 },
        { "*",         0x83bb, Group::ifd0,    &TiffMetadataDecoder::decodeIptc },
        { "*",         0x8649, Group::ifd0,    &TiffMetadataDecoder::decodeIptc }
    };

    const DecoderFct Cr2Decoder::findDecoder(const std::string& make,
                                                   uint32_t     extendedTag,
                                                   uint16_t     group)
    {
        DecoderFct decoderFct = &TiffMetadataDecoder::decodeStdTiffEntry;
        const TiffDecoderInfo* td = find(cr2DecoderInfo_,
                                         TiffDecoderInfo::Key(make, extendedTag, group));
        if (td) {
            // This may set decoderFct to 0, meaning that the tag should not be decoded
            decoderFct = td->decoderFct_;
        }
        return decoderFct;
    }

    Cr2Image::Cr2Image(BasicIo::AutoPtr io, bool /*create*/)
        : Image(ImageType::cr2, mdExif | mdIptc, io)
    {
    } // Cr2Image::Cr2Image

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
        Cr2Header cr2Header;
        TiffParser::decode(this, io_->mmap(), io_->size(),
                           TiffCreator::create, Cr2Decoder::findDecoder,
                           &cr2Header);

    } // Cr2Image::readMetadata

    void Cr2Image::writeMetadata()
    {
        //! Todo: implement me!
        throw(Error(31, "CR2"));
    } // Cr2Image::writeMetadata

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

    void Cr2Header::write(Blob& blob) const
    {
        // Todo: Implement me!
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
