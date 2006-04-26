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
  @file    sonymn2.hpp
  @brief   TIFF Sony makernote
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    18-Apr-06, ahu: created
 */
#ifndef SONYMN2_HPP_
#define SONYMN2_HPP_

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
        const uint16_t sonymn = 269; //!< Sony makernote
    }

    //! Header of a Sony Makernote
    class SonyMnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        SonyMnHeader();
        //! Virtual destructor.
        virtual ~SonyMnHeader() {}
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
        virtual uint32_t ifdOffset() const { return start_; }
        //@}

    private:
        DataBuf buf_;                   //!< Raw header data
        uint32_t start_;                //!< Start of the mn IFD rel. to mn start
        static const byte signature_[]; //!< Sony makernote header signature
        static const uint32_t size_;    //!< Size of the signature

    }; // class SonyMnHeader

    /*!
      @brief Sony Makernote
     */
    class TiffSonyMn : public TiffIfdMakernote {
    public:
        //! @name Creators
        //@{
        //! Default constructor.
        TiffSonyMn(uint16_t tag, uint16_t group, uint16_t mnGroup)
            : TiffIfdMakernote(tag, group, mnGroup) {}
        //! Virtual destructor
        virtual ~TiffSonyMn() {}
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
        //@}

    private:
        // DATA
        SonyMnHeader header_;                //!< Makernote header

    }; // class TiffSonyMn

// *****************************************************************************
// template, inline and free functions

    //! Function to create a Sony makernote
    TiffComponent* newSonyMn(uint16_t    tag,
                             uint16_t    group,
                             uint16_t    mnGroup,
                             const byte* pData,
                             uint32_t    size, 
                             ByteOrder   byteOrder);

}                                       // namespace Exiv2

#endif                                  // #ifndef SONYMN2_HPP_
