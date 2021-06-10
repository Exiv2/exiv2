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
#ifndef PNGIMAGE_HPP_
#define PNGIMAGE_HPP_

// *****************************************************************************
#include "exiv2lib_export.h"

// included header files
#include "image.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2
{

// *****************************************************************************
// class definitions

    // Add PNG to the supported image formats
    namespace ImageType
    {
        const int png = 6;          //!< PNG image type (see class PngImage)
    }

    /*!
      @brief Class to access PNG images. Exif and IPTC metadata are supported
          directly.
     */
    class EXIV2API PngImage : public Image {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that can either open an existing PNG image or create
              a new image from scratch. If a new image is to be created, any
              existing data is overwritten. Since the constructor can not return
              a result, callers should check the good() method after object
              construction to determine success or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method.  Use the Image::io()
              method to get a temporary reference.
          @param create Specifies if an existing image should be read (false)
              or if a new file should be created (true).
         */
        PngImage(BasicIo::UniquePtr io, bool create);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata();
        void writeMetadata();

        /*!
          @brief Print out the structure of image file.
          @throw Error if reading of the file fails or the image data is
                not valid (does not look like data of the specific image type).
          @warning This function is not thread safe and intended for exiv2 -pS for debugging.
         */
        void printStructure(std::ostream& out, PrintStructureOption option,int depth);
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const;
        //@}

        //! @name NOT implemented
        //@{
        //! Copy constructor
        PngImage(const PngImage& rhs) = delete;
        //! Assignment operator
        PngImage& operator=(const PngImage& rhs) = delete;

    private:
        /*!
          @brief Provides the main implementation of writeMetadata() by
                writing all buffered metadata to the provided BasicIo.
          @throw Error on input-output errors or when the image data is not valid.
          @param oIo BasicIo instance to write to (a temporary location).

         */
        void doWriteMetadata(BasicIo& outIo);
        //@}

        std::string profileName_;

    }; // class PngImage

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new PngImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::UniquePtr newPngInstance(BasicIo::UniquePtr io, bool create);

    //! Check if the file iIo is a PNG image.
    EXIV2API bool isPngType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef PNGIMAGE_HPP_
