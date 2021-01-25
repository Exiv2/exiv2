// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2021 Exiv2 authors
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

// *****************************************************************************

// included header files
#include "config.h"

#include "bmffimage.hpp"
#include "tiffimage.hpp"
#include "image.hpp"
#include "image_int.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "types.hpp"
#include "safe_op.hpp"
#include "unused.h"

// + standard includes
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>
#include <cstdio>

// *****************************************************************************
// class member definitions
namespace Exiv2
{
    static bool enabled = false;

    EXIV2API bool enableISOBMFF(bool enable)
    {
#ifdef  EXV_ENABLE_ISOBMFF
        enabled = enable;
        return true;
#endif//EXV_ENABLE_ISOBMFF
        enable=false;// unused
        return enable;
    }

    BmffImage::BmffImage(BasicIo::AutoPtr io, bool /* create */)
            : Image(ImageType::bmff, mdExif | mdIptc | mdXmp, io)
    {
    } // BmffImage::BmffImage

    std::string BmffImage::mimeType() const
    {
        /*
        switch (fileType)
        {
            case ImageType::avif:
                return "image/avif";

            case ImageType::heif:
                return "image/heif";

            default:
                return "image/unknown";
        }
        */
        return "image/bmff";

    }

    void BmffImage::setComment(const std::string& /*comment*/)
    {
        // Todo: implement me!
        throw(Error(kerInvalidSettingForImage, "Image comment", "ISO BMFF"));
    } // ISOBMFF::setComment

    void BmffImage::readMetadata()
    {
    } // ISOBMFF::readMetadata

    void BmffImage::printStructure(std::ostream& out, PrintStructureOption option, int depth)
    {
        if (io_->open() != 0)
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());

        // Ensure that this is the correct image type
        if (!isBmffType(*io_, false)) {
            if (io_->error() || io_->eof())
                throw Error(kerFailedToReadImageData);
            throw Error(kerNotAnImage);
        }
        UNUSED(out);
        UNUSED(option);
        UNUSED(depth);
    } // ISOBMFF::printStructure

    void BmffImage::writeMetadata()
    {
    } // BmffImage::writeMetadata

    // *************************************************************************
    // free functions
    Image::AutoPtr newBmffInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new BmffImage(io, create));
        if (!image->good())
        {
            image.reset();
        }
        return image;
    }

    bool isBmffType(BasicIo& iIo, bool advance)
    {
        if (!enabled)
        {
            return false;
        }
        const int32_t len = 12;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof())
        {
            return false;
        }
        
        bool result = buf[4] == 'f' && buf[5] == 't' && buf[6] == 'y' && buf[7] == 'p';
        if (!advance)
        {
            iIo.seek(-len, BasicIo::cur);
        }
        return result;
    }
}                                       // namespace Exiv2
