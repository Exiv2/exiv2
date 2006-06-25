// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006 Andreas Huggel <ahuggel@gmx.net>
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
  File:      tiffimage.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   15-Mar-06, ahu: created

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

#include "tiffimage.hpp"
#include "tiffparser.hpp"
#include "image.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    TiffImage::TiffImage(BasicIo::AutoPtr io, bool create)
        : Image(mdExif | mdIptc | mdComment), io_(io)
    {
        if (create) {
            IoCloser closer(*io_);
            io_->open();
        }
    } // TiffImage::TiffImage

    bool TiffImage::good() const
    {
        if (io_->open() != 0) return false;
        IoCloser closer(*io_);
        return isThisType(*io_, false);
    }

    void TiffImage::clearMetadata()
    {
        clearExifData();
        clearComment();
    }

    void TiffImage::setMetadata(const Image& image)
    {
        setExifData(image.exifData());
        setComment(image.comment());
    }

    void TiffImage::clearExifData()
    {
        exifData_.clear();
    }

    void TiffImage::setExifData(const ExifData& exifData)
    {
        exifData_ = exifData;
    }

    void TiffImage::clearIptcData()
    {
        iptcData_.clear();
    }

    void TiffImage::setIptcData(const IptcData& iptcData)
    {
        iptcData_ = iptcData;
    }

    void TiffImage::clearComment()
    {
        comment_.erase();
    }

    void TiffImage::setComment(const std::string& comment)
    {
        comment_ = comment;
    }

    void TiffImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading TIFF file " << io_->path() << "\n";
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
        TiffParser::decode(this, io_->mmap(), io_->size(),
                           TiffCreator::create, TiffDecoder::findDecoder);

    } // TiffImage::readMetadata

    void TiffImage::writeMetadata()
    {
/*

       Todo: implement me!

#ifdef DEBUG
        std::cerr << "Writing TIFF file " << io_->path() << "\n";
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

        // Parse image, starting with a TIFF header component
        TiffHeade2::AutoPtr head(new TiffHeade2);
        if (buf.size_ != 0) {
            head->read(buf.pData_, buf.size_);
        }

        Blob blob;
        TiffParser::encode(blob, head.get(), this);

        // Write new buffer to file
        BasicIo::AutoPtr tempIo(io_->temporary()); // may throw
        assert (tempIo.get() != 0);
        tempIo->write(&blob[0], static_cast<long>(blob.size()));
        io_->close();
        io_->transfer(*tempIo); // may throw
*/
    } // TiffImage::writeMetadata

    bool TiffImage::isThisType(BasicIo& iIo, bool advance) const
    {
        return isTiffType(iIo, advance);
    }

    const uint16_t TiffHeade2::tag_ = 42;

    bool TiffHeade2::read(const byte* pData, uint32_t size)
    {
        if (size < 8) return false;

        if (pData[0] == 0x49 && pData[1] == 0x49) {
            byteOrder_ = littleEndian;
        }
        else if (pData[0] == 0x4d && pData[1] == 0x4d) {
            byteOrder_ = bigEndian;
        }
        else {
            return false;
        }
        if (tag_ != getUShort(pData + 2, byteOrder_)) return false;
        offset_ = getULong(pData + 4, byteOrder_);

        return true;
    } // TiffHeade2::read

    void TiffHeade2::print(std::ostream& os, const std::string& prefix) const
    {
        os << prefix
           << "Header, offset = 0x" << std::setw(8) << std::setfill('0')
           << std::hex << std::right << offset_;

        switch (byteOrder_) {
        case littleEndian:     os << ", little endian encoded"; break;
        case bigEndian:        os << ", big endian encoded"; break;
        case invalidByteOrder: break;
        }
        os << "\n";

    } // TiffHeade2::print

    // *************************************************************************
    // free functions

    Image::AutoPtr newTiffInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new TiffImage(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isTiffType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 8;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        TiffHeade2 tiffHeader;
        bool rc = tiffHeader.read(buf, len);
        if (!advance || !rc) {
            iIo.seek(-len, BasicIo::cur);
        }
        return rc;
    }

}                                       // namespace Exiv2
