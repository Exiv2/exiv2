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
  @file    nikonmn2.hpp
  @brief   TIFF Nikon makernote
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    18-Apr-06, ahu: created
 */
#ifndef NIKONMN2_HPP_
#define NIKONMN2_HPP_

// *****************************************************************************
// included header files
#include "makernote2.hpp"
#include "tiffcomposite.hpp"
#include "types.hpp"
#include "tiffimage.hpp"                        // for TiffHeade2

// + standard includes

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    namespace Group {
        const uint16_t nikonmn  = 263; //!< Any Nikon makernote
        const uint16_t nikon1mn = 264; //!< Nikon1 makernote
        const uint16_t nikon2mn = 265; //!< Nikon2 makernote
        const uint16_t nikon3mn = 266; //!< Nikon3 makernote
    }

    //! Header of a Nikon 3 Makernote
    class Nikon3MnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        Nikon3MnHeader();
        //! Virtual destructor.
        virtual ~Nikon3MnHeader() {}
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData, 
                          uint32_t    size, 
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size()      const { return size_; }
        virtual uint32_t ifdOffset() const { return size_; }
        //! Return the byte order for the header
        ByteOrder        byteOrder() const { return byteOrder_; }
        /*!
          @brief Return the base offset for the makernote IFD entries relative
                 to the start of the makernote.
         */
        uint32_t baseOffset()        const { return 10; }
        //@}

    private:
        DataBuf buf_;                   //!< Raw header data
        ByteOrder byteOrder_;           //!< Byteorder for makernote
        uint32_t start_;                //!< Start of the mn IFD rel. to mn start
        static const byte signature_[]; //!< Nikon 3 makernote header signature
        static const uint32_t size_;    //!< Size of the signature

    }; // class Nikon3MnHeader

    /*!
      @brief Nikon 3 Makernote
     */
    class TiffNikon3Mn : public TiffIfdMakernote {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffNikon3Mn(uint16_t tag, uint16_t group, uint16_t mnGroup)
            : TiffIfdMakernote(tag, group, mnGroup) {}
        //! Virtual destructor
        virtual ~TiffNikon3Mn() {}
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
        Nikon3MnHeader header_;                //!< Makernote header

    }; // class TiffNikon3Mn

// *****************************************************************************
// template, inline and free functions

    /*!
      @brief Function to create a Nikon makernote. This will create the 
             appropriate Nikon 1, 2 or 3 makernote, based on the arguments.
     */
    TiffComponent* newNikonMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    mnGroup,
                              const byte* pData,
                              uint32_t    size, 
                              ByteOrder   byteOrder);

}                                       // namespace Exiv2

#endif                                  // #ifndef NIKONMN2_HPP_
