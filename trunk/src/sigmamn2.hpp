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
  @file    sigmamn2.hpp
  @brief   TIFF Sigma makernote
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    18-Apr-06, ahu: created
 */
#ifndef SIGMAMN2_HPP_
#define SIGMAMN2_HPP_

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
        const uint16_t sigmamn = 268; //!< Sigma makernote
    }

    //! Header of a Sigma Makernote
    class SigmaMnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        SigmaMnHeader();
        //! Virtual destructor.
        virtual ~SigmaMnHeader() {}
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
        DataBuf buf_;                    //!< Raw header data
        uint32_t start_;                 //!< Start of the mn IFD rel. to mn start
        static const byte signature1_[]; //!< Sigma makernote header signature 1
        static const byte signature2_[]; //!< Sigma makernote header signature 2
        static const uint32_t size_;     //!< Size of the signature

    }; // class SigmaMnHeader

    /*!
      @brief Sigma Makernote
     */
    class TiffSigmaMn : public TiffIfdMakernote {
    public:
        //! @name Creators
        //@{
        //! Default constructor.
        TiffSigmaMn(uint16_t tag, uint16_t group, uint16_t mnGroup)
            : TiffIfdMakernote(tag, group, mnGroup) {}
        //! Virtual destructor
        virtual ~TiffSigmaMn() {}
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
        SigmaMnHeader header_;                //!< Makernote header

    }; // class TiffSigmaMn

// *****************************************************************************
// template, inline and free functions

    //! Function to create a Sigma makernote
    TiffComponent* newSigmaMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    mnGroup,
                              const byte* pData,
                              uint32_t    size, 
                              ByteOrder   byteOrder);

}                                       // namespace Exiv2

#endif                                  // #ifndef SIGMAMN2_HPP_
