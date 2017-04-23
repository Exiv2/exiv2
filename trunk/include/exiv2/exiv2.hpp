// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2017 Andreas Huggel <ahuggel@gmx.net>
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
/*!
  @file    exiv2.hpp
  @brief   Include all Exiv2 header files.
  @version $Rev: 3091 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    21-Jun-10, ahu: created
 */
#ifndef EXIV2_HPP_
#define EXIV2_HPP_

// *****************************************************************************
// included header files
#include "exiv2/datasets.hpp"
#include "exiv2/basicio.hpp"
#include "exiv2/bmpimage.hpp"
#include "exiv2/convert.hpp"
#include "exiv2/cr2image.hpp"
#include "exiv2/crwimage.hpp"
#include "exiv2/easyaccess.hpp"
#include "exiv2/epsimage.hpp"
#include "exiv2/error.hpp"
#include "exiv2/exif.hpp"
#include "exiv2/futils.hpp"
#include "exiv2/gifimage.hpp"
#include "exiv2/http.hpp"
#include "exiv2/image.hpp"
#include "exiv2/ini.hpp"
#include "exiv2/iptc.hpp"
#include "exiv2/jp2image.hpp"
#include "exiv2/jpgimage.hpp"
#include "exiv2/metadatum.hpp"
#include "exiv2/mrwimage.hpp"
#include "exiv2/orfimage.hpp"
#include "exiv2/pgfimage.hpp"
#ifdef   EXV_HAVE_LIBZ
#include "exiv2/pngimage.hpp"
#endif
#include "exiv2/preview.hpp"
#include "exiv2/properties.hpp"
#include "exiv2/psdimage.hpp"
#include "exiv2/rafimage.hpp"
#include "exiv2/rw2image.hpp"
#if EXV_USE_SSH == 1
#include "exiv2/ssh.hpp"
#endif
#include "exiv2/tags.hpp"
#include "exiv2/tgaimage.hpp"
#include "exiv2/tiffimage.hpp"
#include "exiv2/types.hpp"
#include "exiv2/value.hpp"
#include "exiv2/version.hpp"
#include "exiv2/xmp.hpp"
#include "exiv2/xmpsidecar.hpp"

#endif                                  // #ifndef EXIV2_HPP_
