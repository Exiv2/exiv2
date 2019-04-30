// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
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
  @file    cr2image_int.hpp
  @brief   Internal classes to support CR2 image format
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    23-Apr-08, ahu: created
 */
#pragma once

// *****************************************************************************
// included header files
#include "tiffimage_int.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

// *****************************************************************************
// class definitions

    /// @brief Canon CR2 header structure.
    class Cr2Header : public TiffHeaderBase {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        explicit Cr2Header(ByteOrder byteOrder =littleEndian);
        //! Destructor.
        ~Cr2Header() override;
        //@}

        //! @name Manipulators
        //@{
        bool read(const byte* pData, size_t size) override;
        //@}

        //! @name Accessors
        //@{
        DataBuf write() const override;
        bool isImageTag(uint16_t tag, IfdId group, const PrimaryGroups* pPrimaryGroups) const override;
        //@}

        //! Return the address of offset2 from the start of the header
        static uint32_t offset2addr() { return 12; }

    private:
        // DATA
        uint32_t              offset2_;   //!< Bytes 12-15 from the header
        static const char*    cr2sig_;    //!< Signature for CR2 type TIFF
    }; // class Cr2Header

}}                                      // namespace Internal, Exiv2
