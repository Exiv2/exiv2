// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
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
  File:      imgreg.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   19-Sep-05, ahu: created

 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#include "image.hpp"
#include "jpgimage.hpp"
//#include "cr2image.hpp"
#include "crwimage.hpp"
#include "mrwimage.hpp"
#include "tiffimage.hpp"
#ifdef EXV_HAVE_LIBZ
# include "pngimage.hpp"
#endif // EXV_HAVE_LIBZ

// + standard includes

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    ImageFactory::Registry ImageFactory::registry_[] = {
        Registry(ImageType::jpeg, newJpegInstance, isJpegType),
        Registry(ImageType::exv,  newExvInstance,  isExvType),
//      Registry(ImageType::cr2,  newCr2Instance,  isCr2Type),
        Registry(ImageType::crw,  newCrwInstance,  isCrwType),
        Registry(ImageType::mrw,  newMrwInstance,  isMrwType),
        Registry(ImageType::tiff, newTiffInstance, isTiffType),
#ifdef EXV_HAVE_LIBZ
        Registry(ImageType::png,  newPngInstance,  isPngType)
#endif // EXV_HAVE_LIBZ
    };

}                                       // namespace Exiv2
