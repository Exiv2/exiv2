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
  File:      rw2image.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   06-Jan-09, ahu: created

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

#include "rw2image.hpp"
#include "rw2image_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"
#include "image.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#ifdef DEBUG
# include <iostream>
#endif

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    Rw2Image::Rw2Image(BasicIo::AutoPtr io)
        : Image(ImageType::rw2, mdExif | mdIptc | mdXmp, io)
    {
    } // Rw2Image::Rw2Image

    int Rw2Image::pixelWidth() const
    {
        ExifData::const_iterator imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Image.0x0007"));
        if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
            return imageWidth->toLong();
        }
        return 0;
    }

    int Rw2Image::pixelHeight() const
    {
        ExifData::const_iterator imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Image.0x0006"));
        if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
            return imageHeight->toLong();
        }
        return 0;
    }

    void Rw2Image::setExifData(const ExifData& /*exifData*/)
    {
        // Todo: implement me!
        throw(Error(32, "Exif metadata", "RW2"));
    }

    void Rw2Image::setIptcData(const IptcData& /*iptcData*/)
    {
        // Todo: implement me!
        throw(Error(32, "IPTC metadata", "RW2"));
    }

    void Rw2Image::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "RW2"));
    }

    void Rw2Image::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading RW2 file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isRw2Type(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "RW2");
        }
        clearMetadata();
        ByteOrder bo = Rw2Parser::decode(exifData_,
                                         iptcData_,
                                         xmpData_,
                                         io_->mmap(),
                                         io_->size());
        setByteOrder(bo);
    } // Rw2Image::readMetadata

    void Rw2Image::writeMetadata()
    {
        // Todo: implement me!
        throw(Error(31, "RW2"));
    } // Rw2Image::writeMetadata

    ByteOrder Rw2Parser::decode(
              ExifData& exifData,
              IptcData& iptcData,
              XmpData&  xmpData,
        const byte*     pData,
              uint32_t  size
    )
    {
        Rw2Header rw2Header;
        return TiffParserWorker::decode(exifData,
                                        iptcData,
                                        xmpData,
                                        pData,
                                        size,
                                        Tag::root,
                                        TiffMapping::findDecoder,
                                        &rw2Header);
    }

    // *************************************************************************
    // free functions
    Image::AutoPtr newRw2Instance(BasicIo::AutoPtr io, bool /*create*/)
    {
        Image::AutoPtr image(new Rw2Image(io));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isRw2Type(BasicIo& iIo, bool advance)
    {
        const int32_t len = 24;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        Rw2Header header;
        bool rc = header.read(buf, len);
        if (!advance || !rc) {
            iIo.seek(-len, BasicIo::cur);
        }
        return rc;
    }

}                                       // namespace Exiv2

namespace Exiv2 {
    namespace Internal {

    Rw2Header::Rw2Header()
        : TiffHeaderBase(0x0055, 24, littleEndian, 0x00000018)
    {
    }

    Rw2Header::~Rw2Header()
    {
    }

    uint32_t Rw2Header::write(Blob& blob) const
    {
        // Todo: Implement me!
        return 0;
    }

}}                                      // namespace Internal, Exiv2
