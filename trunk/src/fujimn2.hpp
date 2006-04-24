// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006 Andreas Huggel <ahuggel@gmx.net>
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
  @file    fujimn2.hpp
  @brief   TIFF Fujifilm makernote
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    15-Apr-06, ahu: created
 */
#ifndef FUJIMN2_HPP_
#define FUJIMN2_HPP_

// *****************************************************************************
// included header files
#include "makernote2.hpp"
#include "tiffcomposite.hpp"
#include "types.hpp"

// + standard includes

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    namespace Group {
        const uint16_t fujimn = 258; //!< Fujifilm makernote
    }

    //! Header of a Fujifilm Makernote
    class FujiMnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        FujiMnHeader();
        //! Virtual destructor.
        virtual ~FujiMnHeader() {}
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData, 
                          uint32_t    size, 
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size()      const { return header_.size_; }
        virtual uint32_t ifdOffset() const { return start_; }
        //! Return the byte order for the header
        ByteOrder        byteOrder() const { return byteOrder_; }
        //@}

    private:
        DataBuf header_;               //!< Data buffer for the makernote header
        static const char* signature_; //!< Fujifilm makernote header signature
        static const uint32_t size_;   //!< Size of the signature
        static const ByteOrder byteOrder_; //!< Byteorder for makernote (II)
        uint32_t start_;               //!< Start of the mn IFD rel. to mn start

    }; // class FujiMnHeader

    /*!
      @brief Fujifilm Makernote
     */
    class TiffFujiMn : public TiffIfdMakernote {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffFujiMn(uint16_t tag, uint16_t group, uint16_t mnGroup)
            : TiffIfdMakernote(tag, group, mnGroup) {}
        //! Virtual destructor
        virtual ~TiffFujiMn() {}
        //@}

    private:
        //! @name Manipulators
        //@{
        virtual bool doReadHeader(const byte* pData,
                                  uint32_t    size, 
                                  ByteOrder   byteOrder);
        //@}

        //! @name Accessors
        //@{
        virtual uint32_t doIfdOffset() const;
        virtual TiffRwState::AutoPtr doGetState(uint32_t  mnOffset,
                                                ByteOrder byteOrder) const;
        //@}

    private:
        // DATA
        FujiMnHeader header_;                //!< Makernote header

    }; // TiffFujiMn

// *****************************************************************************
// template, inline and free functions

    //! Function to create a Fujifilm makernote
    TiffComponent* newFujiMn(uint16_t    tag,
                             uint16_t    group,
                             uint16_t    mnGroup,
                             const byte* pData,
                             uint32_t    size, 
                             ByteOrder   byteOrder);

}                                       // namespace Exiv2

#endif                                  // #ifndef FUJIMN2_HPP_
