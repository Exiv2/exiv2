// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
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
 */
// *****************************************************************************
// included header files
#include "config.h"

#include "cr2image.hpp"
#include "tiffimage.hpp"
#include "cr2header_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"
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
        : Image(ImageType::cr2, mdExif | mdIptc | mdXmp, io)
    {
    } // Cr2Image::Cr2Image

    std::string Cr2Image::mimeType() const
    {
        return "image/x-canon-cr2";
    }

    int Cr2Image::pixelWidth() const
    {
        ExifData::const_iterator imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
        if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
            return imageWidth->toLong();
        }
        return 0;
    }

    int Cr2Image::pixelHeight() const
    {
        ExifData::const_iterator imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
        if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
            return imageHeight->toLong();
        }
        return 0;
    }

    void Cr2Image::printStructure(std::ostream& out, Exiv2::PrintStructureOption option,int depth)
    {
        if (io_->open() != 0) throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        io_->seek(0,BasicIo::beg);
        printTiffStructure(io(),out,option,depth-1);
    }

    void Cr2Image::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(kerInvalidSettingForImage, "Image comment", "CR2"));
    }

    void Cr2Image::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading CR2 file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isCr2Type(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(kerFailedToReadImageData);
            throw Error(kerNotAnImage, "CR2");
        }
        clearMetadata();
        ByteOrder bo = Cr2Parser::decode(exifData_,
                                         iptcData_,
                                         xmpData_,
                                         io_->mmap(),
                                         (uint32_t) io_->size());
        setByteOrder(bo);
    } // Cr2Image::readMetadata

    void Cr2Image::writeMetadata()
    {
#ifdef DEBUG
        std::cerr << "Writing CR2 file " << io_->path() << "\n";
#endif
        ByteOrder bo = byteOrder();
        byte* pData = 0;
        long size = 0;
        IoCloser closer(*io_);
        if (io_->open() == 0) {
            // Ensure that this is the correct image type
            if (isCr2Type(*io_, false)) {
                pData = io_->mmap(true);
                size = (long) io_->size();
                Cr2Header cr2Header;
                if (0 == cr2Header.read(pData, 16)) {
                    bo = cr2Header.byteOrder();
                }
            }
        }
        if (bo == invalidByteOrder) {
            bo = littleEndian;
        }
        setByteOrder(bo);
        Cr2Parser::encode(*io_, pData, size, bo, exifData_, iptcData_, xmpData_); // may throw
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
                                        Tag::root,
                                        TiffMapping::findDecoder,
                                        &cr2Header);
    }

    WriteMethod Cr2Parser::encode(
              BasicIo&  io,
        const byte*     pData,
              uint32_t  size,
              ByteOrder byteOrder,
        const ExifData& exifData,
        const IptcData& iptcData,
        const XmpData&  xmpData
    )
    {
        // Copy to be able to modify the Exif data
        ExifData ed = exifData;

        // Delete IFDs which do not occur in TIFF images
        static const IfdId filteredIfds[] = {
            panaRawId
        };
        for (unsigned int i = 0; i < EXV_COUNTOF(filteredIfds); ++i) {
#ifdef DEBUG
            std::cerr << "Warning: Exif IFD " << filteredIfds[i] << " not encoded\n";
#endif
            ed.erase(std::remove_if(ed.begin(),
                                    ed.end(),
                                    FindExifdatum(filteredIfds[i])),
                     ed.end());
        }

        std::auto_ptr<TiffHeaderBase> header(new Cr2Header(byteOrder));
        OffsetWriter offsetWriter;
        offsetWriter.setOrigin(OffsetWriter::cr2RawIfdOffset, Cr2Header::offset2addr(), byteOrder);
        return TiffParserWorker::encode(io,
                                        pData,
                                        size,
                                        ed,
                                        iptcData,
                                        xmpData,
                                        Tag::root,
                                        TiffMapping::findEncoder,
                                        header.get(),
                                        &offsetWriter);
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
