// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2021 Exiv2 authors
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

#pragma once

// *****************************************************************************
#include "exiv2lib_export.h"

// included header files
#include "image.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2
{
    EXIV2API bool enableISOBMFF(bool enable = true);

// *****************************************************************************
// class definitions

    // Add ISO Base Media File Format to the supported image formats
    namespace ImageType {
        const int bmff = 15; //!< ISO BMFF (bmff) image type (see class ISOBMFF)
    }

    /*!
      @brief Class to access ISO BMFF images.
     */
    class EXIV2API BmffImage : public Image {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor to open a ISO/IEC BMFF image. Since the
              constructor can not return a result, callers should check the
              good() method after object construction to determine success
              or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method.  Use the Image::io()
              method to get a temporary reference.
          @param create Specifies if an existing image should be read (false)
              or if a new file should be created (true).
         */
        BmffImage(BasicIo::AutoPtr io, bool create);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata() /* override */ ;
        void writeMetadata() /* override */ ;

        /*!
          @brief Print out the structure of image file.
          @throw Error if reading of the file fails or the image data is
                not valid (does not look like data of the specific image type).
          @warning This function is not thread safe and intended for exiv2 -pS for debugging.
         */
        void printStructure(std::ostream& out, PrintStructureOption option,int depth) /* override */ ;

        /*!
          @brief Todo: Not supported yet(?). Calling this function will throw
              an instance of Error(kerInvalidSettingForImage).
         */
        void setComment(const std::string& comment) /* override */ ;
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const /* override */ ;
        //@}
#if 0
        BmffImage& operator=(const BmffImage& rhs) /* = delete*/ ;
        BmffImage& operator=(const BmffImage&& rhs) /* = delete */ ;
        BmffImage(const BmffImage& rhs) /* = delete */;
        BmffImage(const BmffImage&& rhs) /* = delete */;
#endif

    private:
        uint32_t fileType;

        /*!
          @brief Provides the main implementation of writeMetadata() by
                writing all buffered metadata to the provided BasicIo.
          @param oIo BasicIo instance to write to (a temporary location).

          @return 4 if opening or writing to the associated BasicIo fails
         */
        void doWriteMetadata(BasicIo& outIo);
        //@}

    }; // class BmffImage

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new ISO BMFF instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::AutoPtr newBmffInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a ISO BMFF image.
    EXIV2API bool isBmffType(BasicIo& iIo, bool advance);
}                                       // namespace Exiv2
