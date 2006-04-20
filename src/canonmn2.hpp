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
  @file    canonmn2.hpp
  @brief   TIFF Canon makernote
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    18-Apr-06, ahu: created
 */
#ifndef CANONMN2_HPP_
#define CANONMN2_HPP_

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
        const uint16_t canonmn = 259; //!< Canon makernote
        const uint16_t canoncs = 260; //!< Canon camera settings
        const uint16_t canonsi = 261; //!< Canon shot info
        const uint16_t canoncf = 262; //!< Canon customer functions
    }

    /*!
      @brief Canon Makernote
     */
    class TiffCanonMn : public TiffIfdMakernote {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffCanonMn(uint16_t tag, uint16_t group, uint16_t mnGroup)
            : TiffIfdMakernote(tag, group, mnGroup) {}
        //! Virtual destructor
        virtual ~TiffCanonMn() {}
        //@}
    private:
        //! @name Accessors
        //@{
        virtual TiffRwState::AutoPtr doGetState(uint32_t  mnOffset,
                                                ByteOrder byteOrder) const;
        //@}

    }; // class TiffCanonMn

    /*!
      @brief TIFF component factory for Canon TIFF components.
     */
    class TiffCanonCreator {
    public:
        /*!
          @brief Create the TiffComponent for TIFF entry \em extendedTag and 
                 \em group based on the embedded lookup table.

          If a tag and group combination is not found in the table, a TiffEntry
          is created.  If the pointer that is returned is 0, then the TIFF entry
          should be ignored.
        */
        static TiffComponent::AutoPtr create(uint32_t extendedTag,
                                             uint16_t group);

    private:
        static const TiffStructure tiffStructure_[]; //<! TIFF structure
    }; // class TiffCanonCreator

// *****************************************************************************
// template, inline and free functions

    //! Function to create a Canon makernote
    TiffComponent* newCanonMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    mnGroup,
                              const byte* pData,
                              uint32_t    size, 
                              ByteOrder   byteOrder);

}                                       // namespace Exiv2

#endif                                  // #ifndef CANONMN2_HPP_
