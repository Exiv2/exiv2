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
  @file    pngimage.hpp
  @brief   PNG image, implemented using the following references:
           <a href="http://www.w3.org/TR/PNG/">PNG specification</a> by W3C<br>
           <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/PNG.html">PNG tags list</a> by Phil Harvey<br>
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Gilles Caulier (cgilles)
           <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
  @date    12-Jun-06, gc: submitted
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
    // *****************************************************************************
    // class definitions

    /// @brief Class to access PNG images. Exif and IPTC metadata are supported directly.
    class EXIV2API PngImage : public Image
    {
    public:
        //! @name Creators
        //@{
        /// @brief Constructor that can either open an existing PNG image or create a new image from scratch.
        ///
        /// If a new image is to be created, any existing data is overwritten. Since the constructor can not return a
        /// result, callers should check the good() method after object construction to determine success or failure.
        /// @param io An auto-pointer that owns a BasicIo instance used for reading and writing image metadata.
        /// Use the Image::io() method to get a temporary reference to the BasicIo instance.
        /// @param create Indicates if a new file will be created, or if we are reading an existing file.
        PngImage(BasicIo::UniquePtr io, bool create);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata() override;
        void writeMetadata() override;
        void printStructure(std::ostream& out, PrintStructureOption option, int depth) override;
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const override;
        //@}

        PngImage& operator=(const PngImage& rhs) = delete;
        PngImage& operator=(const PngImage&& rhs) = delete;
        PngImage(const PngImage& rhs) = delete;
        PngImage(const PngImage&& rhs) = delete;

    private:
        /// @brief Provides the main implementation of writeMetadata() by writing all buffered metadata to the
        /// provided BasicIo.
        /// @throw Error on input-output errors or when the image data is not valid.
        /// @param oIo BasicIo instance to write to (a temporary location).
        void doWriteMetadata(BasicIo& outIo);
        //@}

        std::string profileName_;
    };

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /// @brief Create a new PngImage instance and return an auto-pointer to it.
    EXIV2API Image::UniquePtr newPngInstance(BasicIo::UniquePtr io, bool create);

    /// @brief Check if the file iIo is a PNG image.
    EXIV2API bool isPngType(BasicIo& iIo, bool advance);

}  // namespace Exiv2
