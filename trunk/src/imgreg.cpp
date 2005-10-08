// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2005 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
#include "crwimage.hpp"

// + standard includes

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    ImageFactory::Registry ImageFactory::registry_[] = {
        Registry(ImageType::jpeg, newJpegInstance, isJpegType),
        Registry(ImageType::exv,  newExvInstance,  isExvType),
        Registry(ImageType::crw,  newCrwInstance,  isCrwType)
    };

}                                       // namespace Exiv2
