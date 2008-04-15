// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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
  @file    jp2image.hpp
  @brief   JPEG-2000 image, implemented using the following references:
           <a href="http://jpeg.org/public/fcd15444-6.pdf">ISO/IEC JTC 1/SC 29/WG1 N2401: JPEG 2000 Part 6 FCD 15444-6</a><br>
  @version $Rev$
  @author  Marco Piovanelli, Ovolabs (marco)
           <a href="mailto:marco.piovanelli@pobox.com">marco.piovanelli@pobox.com</a>
  @date    12-Mar-2007, marco: created
 */
#ifndef JP2IMAGE_HPP_
#define JP2IMAGE_HPP_

// *****************************************************************************
// included header files
#include "exif.hpp"
#include "iptc.hpp"
#include "image.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    // Add JPEG-2000 to the supported image formats
    namespace ImageType {
        const int jp2 = 15;                     //!< JPEG-2000 image type
    }

    /*!
      @brief Class to access JPEG-2000 images.
     */
    class Jp2Image : public Image {
        //! @name NOT Implemented
        //@{
        //! Copy constructor
        Jp2Image(const Jp2Image& rhs);
        //! Assignment operator
        Jp2Image& operator=(const Jp2Image& rhs);
        //@}

    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor to open a JPEG-2000 image. Since the
              constructor can not return a result, callers should check the
              good() method after object construction to determine success
              or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method.  Use the Image::io()
              method to get a temporary reference.
         */
        Jp2Image(BasicIo::AutoPtr io);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata();
        /*!
          @brief Todo: Write metadata back to the image. This method is not
              yet implemented. Calling it will throw an Error(31).
         */
        void writeMetadata();
        /*!
          @brief Todo: Not supported yet. Calling this function will throw
              an instance of Error(32).
         */
        void setExifData(const ExifData& exifData);
        /*!
          @brief Todo: Not supported yet. Calling this function will throw
              an instance of Error(32).
         */
        void setIptcData(const IptcData& iptcData);
        /*!
          @brief Todo: Not supported yet(?). Calling this function will throw
              an instance of Error(32).
         */
        void setComment(const std::string& comment);
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const { return "image/jp2"; }
        //@}

    }; // class Jp2Image

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new Jp2Image instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    Image::AutoPtr newJp2Instance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a JPEG-2000 image.
    bool isJp2Type(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef JP2IMAGE_HPP_
