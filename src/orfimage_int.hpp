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
#ifndef ORFIMAGE_INT_HPP_
#define ORFIMAGE_INT_HPP_

// *****************************************************************************
// included header files
#include "tiffimage_int.hpp"
#include "types.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

// *****************************************************************************
// class definitions

    /*!
      @brief Olympus ORF header structure.
     */
    class OrfHeader : public TiffHeaderBase {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        explicit OrfHeader(ByteOrder byteOrder =littleEndian);
        //! Destructor.
        ~OrfHeader();
        //@}

        //! @name Manipulators
        //@{
        bool read(const byte* pData, uint32_t size);
        //@}

        //! @name Accessors
        //@{
        DataBuf write() const;
        //@}
    private:
        // DATA
        uint16_t sig_;       //<! The actual magic number
    }; // class OrfHeader

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef ORFIMAGE_INT_HPP_
