// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
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

#include "rw2image_int.hpp"

namespace Exiv2 {
    namespace Internal {

    Rw2Header::Rw2Header()
        : TiffHeaderBase(0x0055, 24, littleEndian, 0x00000018)
    {
    }

    Rw2Header::~Rw2Header() = default;

    DataBuf Rw2Header::write() const
    {
        // Todo: Implement me!
        return DataBuf();
    }

}}                                      // namespace Internal, Exiv2
