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
  File:    pngimage.cpp
  Version: $Rev: 823 $
  Author(s): Gilles Caulier (cgilles) <caulier dot gilles at gmail dot com>
  History: 12-Jun-06, gc: submitted
  Credits: See header file
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id: pngimage.cpp 823 2006-06-12 07:35:00Z cgilles $")

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

    PngImage::PngImage(BasicIo::AutoPtr io, bool /*create*/)
        : Image(ImageType::png, mdExif | mdIptc, io)
    {
    } // PngImage::PngImage

    void PngImage::setExifData(const ExifData& /*exifData*/)
    {
        // Todo: implement me!
        throw(Error(32, "Exif metadata", "PNG"));
    }

    void PngImage::setIptcData(const IptcData& /*iptcData*/)
    {
        // Todo: implement me!
        throw(Error(32, "IPTC metadata", "PNG"));
    }

    void PngImage::setComment(const std::string& /*comment*/)
    {
        // Todo: implement me!
        // Add 'iTXt' chunk 'Description' tag support here
        throw(Error(32, "Image comment", "PNG"));
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
        if (!isPngType(*io_, false))
        {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "PNG");
        }
        clearMetadata();
        PngChunk::decode(this, io_->mmap(), io_->size());

        /*
          Todo:

          - readMetadata implements the loop over the chunks in the image and
            makes decisions what to do. It reads only one chunk at a time
            instead of the whole file.

          - new class PngChunk, initialized with the memory of one chunk and
            has all the access methods required to determine the field, key,
            to access and decompress data etc.
        */

    } // PngImage::readMetadata

    void PngImage::writeMetadata()
    {
        //! Todo: implement me!
        throw(Error(31, "PNG"));
    } // PngImage::writeMetadata

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
        const unsigned char pngId[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof())
        {
            return false;
        }
        int rc = memcmp(buf, pngId, 8);
        if (!advance || rc != 0)
        {
            iIo.seek(-len, BasicIo::cur);
        }

        return rc == 0;
    }
}                                       // namespace Exiv2
