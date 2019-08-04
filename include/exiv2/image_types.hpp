// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2019 Exiv2 authors
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

#ifndef INCLUDE_EXIV2_IMAGE_TYPES_H
#define INCLUDE_EXIV2_IMAGE_TYPES_H

namespace Exiv2
{
    /// Supported Image Formats
    enum class ImageType
    {
        none,
        arw,
        bigtiff,
        bmp,        ///< Windows bitmap
        cr2,
        crw,
        dng,
        exv,
        gif,        ///< GIF
        heif,       ///< HEIF
        jp2,        ///< JPEG-2000
        jpeg,
        mrw,
        nef,
        orf,
        pef,
        png,
        pgf,
        psd,        ///< Photoshop (PSD)
        raf,
        rw2,
        sr2,
        srw,
        tga,
        tiff,
        webp,
        xmp,        ///< XMP sidecar files
    };
}  // namespace Exiv2

#endif  // INCLUDE_EXIV2_IMAGE_TYPES_H
