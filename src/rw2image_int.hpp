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
#ifndef RW2IMAGE_INT_HPP_
#define RW2IMAGE_INT_HPP_

// *****************************************************************************
// included header files
#include "tiffimage_int.hpp"
#include "types.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

// *****************************************************************************
// class definitions

    /*!
      @brief Panasonic RW2 header structure.
     */
    class Rw2Header : public TiffHeaderBase {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        Rw2Header();
        //! Destructor.
        ~Rw2Header();
        //@}

        //! @name Accessors
        //@{
        //! Not yet implemented. Does nothing and returns an empty buffer.
        DataBuf write() const;
        //@}

    }; // class Rw2Header

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef RW2IMAGE_INT_HPP_
