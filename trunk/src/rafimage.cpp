// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2007 Andreas Huggel <ahuggel@gmx.net>
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
  File:      rafimage.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   05-Feb-07, ahu: created
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

#include "rafimage.hpp"
#include "tiffparser.hpp"
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

    RafImage::RafImage(BasicIo::AutoPtr io, bool /*create*/)
        : Image(ImageType::raf, mdExif | mdIptc, io)
    {
    } // RafImage::RafImage

    void RafImage::setExifData(const ExifData& /*exifData*/)
    {
        // Todo: implement me!
        throw(Error(32, "Exif metadata", "RAF"));
    }

    void RafImage::setIptcData(const IptcData& /*iptcData*/)
    {
        // Todo: implement me!
        throw(Error(32, "IPTC metadata", "RAF"));
    }

    void RafImage::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "RAF"));
    }

    void RafImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading RAF file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) throw Error(9, io_->path(), strError());
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isRafType(*io_, true)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "RAF");
        }
        if (io_->seek(84, BasicIo::beg) != 0) throw Error(14);
        byte tmp[4];
        io_->read(tmp, 4);
        if (io_->error() || io_->eof()) throw Error(14);
        uint32_t const pos = getULong(tmp, bigEndian) + 4;
        if (io_->seek(pos, BasicIo::beg) != 0) throw Error(14);
        io_->read(tmp, 2);
        if (io_->error() || io_->eof()) throw Error(14);
        DataBuf buf(getUShort(tmp, bigEndian) - 10);
        if (io_->seek(pos + 8, BasicIo::beg) != 0) throw Error(14);
        io_->read(buf.pData_, buf.size_);
        if (io_->error() || io_->eof()) throw Error(14);
        clearMetadata();
        TiffParser::decode(this, buf.pData_, buf.size_,
                           TiffCreator::create, TiffDecoder::findDecoder);
    } // RafImage::readMetadata

    void RafImage::writeMetadata()
    {
        //! Todo: implement me!
        throw(Error(31, "RAF"));
    } // RafImage::writeMetadata

    // *************************************************************************
    // free functions
    Image::AutoPtr newRafInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new RafImage(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isRafType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 8;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        int rc = memcmp(buf, "FUJIFILM", 8);
        if (!advance || rc != 0) {
            iIo.seek(-len, BasicIo::cur);
        }
        return rc == 0;
    }

}                                       // namespace Exiv2
