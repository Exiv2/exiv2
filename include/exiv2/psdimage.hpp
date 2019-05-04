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
  @file    psdimage.hpp
  @brief   Photoshop image, implemented using the following references:
           <a href="http://www.fine-view.com/jp/lab/doc/ps6ffspecsv2.pdf">Adobe Photoshop 6.0 File Format Specification</a> by Adobe<br>
  @author  Marco Piovanelli, Ovolab (marco)
           <a href="mailto:marco.piovanelli@pobox.com">marco.piovanelli@pobox.com</a>
  @author  Michael Ulbrich
           <a href="mailto:mul@rentapacs.de">mul@rentapacs.de</a>
  @date    05-Mar-2007, marco: created
 */
#pragma once

// *****************************************************************************
#include "exiv2lib_export.h"

// included header files
#include "image.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    /*!
      @brief Class to access raw Photoshop images.
     */
    class EXIV2API PsdImage : public Image {
    public:
        PsdImage& operator=(const PsdImage& rhs) = delete;
        PsdImage& operator=(const PsdImage&& rhs) = delete;
        PsdImage(const PsdImage& rhs) = delete;
        PsdImage(const PsdImage&& rhs) = delete;

        //! @name Creators
        //@{
        /*!
          @brief Constructor to open a Photoshop image. Since the
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
        explicit PsdImage(BasicIo::UniquePtr io);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata() override;
        void writeMetadata() override;
        /*!
          @brief Not supported. Calling this function will throw an Error(kerInvalidSettingForImage).
         */
        void setComment(const std::string& comment) override;
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Return the MIME type of the image.

          The MIME type returned for Photoshop images is "image/x-photoshop".

          @note This should really be "image/vnd.adobe.photoshop"
              (officially registered with IANA in December 2005 -- see
              http://www.iana.org/assignments/media-types/image/vnd.adobe.photoshop)
              but Apple, as of Tiger (10.4.8), maps this official MIME type to a
              dynamic UTI, rather than "com.adobe.photoshop-image" as it should.
         */
        std::string mimeType() const override;
        //@}

    private:
        //! @name Manipulators
        //@{
        void readResourceBlock(uint16_t resourceId, uint32_t resourceSize);
        /*!
          @brief Provides the main implementation of writeMetadata() by
                writing all buffered metadata to the provided BasicIo.
          @param oIo BasicIo instance to write to (a temporary location).

          @return 4 if opening or writing to the associated BasicIo fails
         */
        void doWriteMetadata(BasicIo& oIo);
        size_t writeExifData(const ExifData& exifData, BasicIo& out);
        //@}

        //! @name Accessors
        //@{
        size_t writeIptcData(const IptcData& iptcData, BasicIo& out) const;
        size_t writeXmpData(const XmpData& xmpData, BasicIo& out) const;
        //@}

    }; // class PsdImage

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new PsdImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::UniquePtr newPsdInstance(BasicIo::UniquePtr io, bool create);

    //! Check if the file iIo is a Photoshop image.
    EXIV2API bool isPsdType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2
