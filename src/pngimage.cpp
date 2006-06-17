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
  File:    pngimage.cpp
  Version: $Rev: 823 $
  History: 12-Jun-06, gc: submitted
  Credits: See header file
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id: pngimage.cpp 823 2006-06-12 07:35:00Z cgilles $");

// *****************************************************************************

//#define DEBUG 1

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "pngchunk.hpp"
#include "pngimage.hpp"
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

    PngImage::PngImage(BasicIo::AutoPtr io, bool create)
        : Image(mdExif | mdIptc), io_(io)
    {
        if (create) 
        {
            IoCloser closer(*io_);
            io_->open();
        }
    } // PngImage::PngImage

    bool PngImage::good() const
    {
        if (io_->open() != 0) return false;
        IoCloser closer(*io_);
        return isThisType(*io_, false);
    }

    void PngImage::clearMetadata()
    {
        clearExifData();
    }

    void PngImage::setMetadata(const Image& image)
    {
        setExifData(image.exifData());
    }

    void PngImage::clearExifData()
    {
        exifData_.clear();
    }

    void PngImage::setExifData(const ExifData& exifData)
    {
        exifData_ = exifData;
    }

    void PngImage::clearIptcData()
    {
        iptcData_.clear();
    }

    void PngImage::setIptcData(const IptcData& iptcData)
    {
        iptcData_ = iptcData;
    }

    void PngImage::clearComment()
    {
        // not yet supported.
        // TODO : Add 'iTXt' chunk 'Description' tag support here
    }

    void PngImage::setComment(const std::string& comment)
    {
        // not yet supported
        // TODO : Add 'iTXt' chunk 'Description' tag support here
    }

    void PngImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Exiv2::PngImage::readMetadata: Reading PNG file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) 
        {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);

        // Ensure that this is the correct image type
        if (!isThisType(*io_, false)) 
        {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "PNG");
        }

        clearMetadata();

        DataBuf buf = io_->read(io_->size());
        if (io_->error() || io_->eof()) throw Error(14);

        PngChunk::decode(this, buf.pData_, buf.size_);

    } // PngImage::readMetadata

    void PngImage::writeMetadata()
    {
    /*
       TODO: implement me!
    */
    } // PngImage::writeMetadata

    bool PngImage::isThisType(BasicIo& iIo, bool advance) const
    {
        return isPngType(iIo, advance);
    }

    // *************************************************************************
    // free functions

    Image::AutoPtr newPngInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new PngImage(io, create));
        if (!image->good()) 
        {
            image.reset();
        }
        return image;
    }

    bool isPngType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 8;
        const unsigned char pngID[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) 
        {
            return false;
        }
        int rc = memcmp(buf, pngID, 8);
        if (!advance || rc != 0) 
        {
            iIo.seek(-len, BasicIo::cur);
        }

        return rc == 0;
    }
}                                       // namespace Exiv2
