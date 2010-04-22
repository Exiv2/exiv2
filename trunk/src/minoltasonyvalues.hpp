// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2010 Andreas Huggel <ahuggel@gmx.net>
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
  @file    minoltasonyvalues.hpp
  @brief   Wrapper for Minolta and Sony MakerNote Common Values<br>
           Email communication with <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a><br>

  @version $Rev$
  @author  Gilles Caulier (cgilles)
           <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
  @date    14-Apr-10, gc: submitted
 */
#ifndef MINOLTASONYVALUES_HPP_
#define MINOLTASONYVALUES_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "types.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

    //! Print Minolta/Sony Lens id values to readable labels.
    EXIV2API std::ostream& printMinoltaSonyLensID(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony Color Mode values to readable labels.
    EXIV2API std::ostream& printMinoltaSonyColorMode(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony bool function values to readable labels.
    EXIV2API std::ostream& printMinoltaSonyBoolValue(std::ostream&, const Value&, const ExifData*);

    //! Print Minolta/Sony AF Area Mode values to readable labels.
    EXIV2API std::ostream& printMinoltaSonyAFAreaMode(std::ostream&, const Value&, const ExifData*);

    // TODO: Added shared methods here.

}                                       // namespace Exiv2

#endif                                  // #ifndef MINOLTASONYVALUES_HPP_
