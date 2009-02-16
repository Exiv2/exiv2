// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2009 Andreas Huggel <ahuggel@gmx.net>
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
  File:      orfimage.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   13-May-06, ahu: created
  Credits:   See header file
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

#include "orfimage.hpp"
#include "orfimage_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"
#include "image.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    OrfImage::OrfImage(BasicIo::AutoPtr io, bool /*create*/)
        : Image(ImageType::orf, mdExif | mdIptc, io)
    {
    } // OrfImage::OrfImage

    int OrfImage::pixelWidth() const
    {
        ExifData::const_iterator imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Image.ImageWidth"));
        if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
            return imageWidth->toLong();
        }
        return 0;
    }

    int OrfImage::pixelHeight() const
    {
        ExifData::const_iterator imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Image.ImageLength"));
        if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
            return imageHeight->toLong();
        }
        return 0;
    }

    void OrfImage::setExifData(const ExifData& /*exifData*/)
    {
        // Todo: implement me!
        throw(Error(32, "Exif metadata", "ORF"));
    }

    void OrfImage::setIptcData(const IptcData& /*iptcData*/)
    {
        // Todo: implement me!
        throw(Error(32, "IPTC metadata", "ORF"));
    }

    void OrfImage::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "ORF"));
    }

    void OrfImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading ORF file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isOrfType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "ORF");
        }
        clearMetadata();
        ByteOrder bo = OrfParser::decode(exifData_,
                                         iptcData_,
                                         xmpData_,
                                         io_->mmap(),
                                         io_->size());
        setByteOrder(bo);
    } // OrfImage::readMetadata

    void OrfImage::writeMetadata()
    {
        // Todo: implement me!
        throw(Error(31, "ORF"));
    } // OrfImage::writeMetadata

    ByteOrder OrfParser::decode(
              ExifData& exifData,
              IptcData& iptcData,
              XmpData&  xmpData,
        const byte*     pData,
              uint32_t  size
    )
    {
        OrfHeader orfHeader;
        return TiffParserWorker::decode(exifData,
                                        iptcData,
                                        xmpData,
                                        pData,
                                        size,
                                        Tag::root,
                                        TiffMapping::findDecoder,
                                        &orfHeader);
    }

    WriteMethod OrfParser::encode(
              Blob&     blob,
        const byte*     pData,
              uint32_t  size,
        const ExifData& exifData,
        const IptcData& iptcData,
        const XmpData&  xmpData
    )
    {
        /* Todo: Implement me!

        return TiffParserWorker::encode(blob,
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
    Image::AutoPtr newOrfInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new OrfImage(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isOrfType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 8;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        OrfHeader orfHeader;
        bool rc = orfHeader.read(buf, len);
        if (!advance || !rc) {
            iIo.seek(-len, BasicIo::cur);
        }
        return rc;
    }

}                                       // namespace Exiv2

namespace Exiv2 {
    namespace Internal {

    OrfHeader::OrfHeader()
        : TiffHeaderBase('O'<< 8 | 'R', 8, littleEndian, 0x00000008)
    {
    }

    OrfHeader::~OrfHeader()
    {
    }

    bool OrfHeader::read(const byte* pData, uint32_t size)
    {
        if (size < 8) return false;

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
        if (offset() != 0x00000008) return false;

        return true;
    } // OrfHeader::read

    uint32_t OrfHeader::write(Blob& blob) const
    {
        // Todo: Implement me!
        return 0;
    }

}}                                      // namespace Internal, Exiv2
