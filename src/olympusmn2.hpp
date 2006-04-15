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
  @file    olympusmn2.hpp
  @brief   TIFF Olympus makernote
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    15-Apr-06, ahu: created
 */
#ifndef OLYMPUSMN2_HPP_
#define OLYMPUSMN2_HPP_

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
        const uint16_t olympmn = 257; //!< Olympus makernote
    }

    //! Header of an Olympus Makernote
    class OlympusMnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        OlympusMnHeader();
        //! Virtual destructor.
        virtual ~OlympusMnHeader() {}
        //@}
        //! @name Manipulators
        //@{
        virtual bool     read(const byte* pData, 
                              uint32_t    size);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size()      const { return header_.size_; }
        virtual uint32_t ifdOffset() const { return size_; }
        virtual bool     check()     const;
        //@}

    private:
        DataBuf header_;               //!< Data buffer for the makernote header
        static const char* signature_; //!< Olympus makernote header signature
        static const uint32_t size_;   //!< Size of the signature

    }; // class OlympusMnHeader

    /*!
      @brief Olympus Makernote
     */
    class TiffOlympusMn : public TiffIfdMakernote {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffOlympusMn(uint16_t tag, uint16_t group, uint16_t mnGroup)
            : TiffIfdMakernote(tag, group, mnGroup) {}
        //! Virtual destructor
        virtual ~TiffOlympusMn() {}
        //@}

    private:
        //! @name Manipulators
        //@{
        virtual bool doReadHeader(const byte* pData, uint32_t size);
        //@}

        //! @name Accessors
        //@{
        virtual bool doCheckHeader() const;
        virtual uint32_t doIfdOffset() const;
        //@}

    private:
        // DATA
        OlympusMnHeader header_;                //!< Makernote header

    }; // TiffOlympusMn

// *****************************************************************************
// template, inline and free functions

    //! Function to create an Olympus makernote
    TiffComponent* newOlympusMn(uint16_t    tag,
                                uint16_t    group,
                                uint16_t    mnGroup,
                                const byte* pData,
                                uint32_t    size, 
                                ByteOrder   byteOrder);

}                                       // namespace Exiv2

#endif                                  // #ifndef OLYMPUSMN2_HPP_
