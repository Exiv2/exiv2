// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006-2007 Andreas Huggel <ahuggel@gmx.net>
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
  @file    cr2image.hpp
  @brief   Class Cr2Image
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    22-Apr-06, ahu: created
 */
#ifndef CR2IMAGE_HPP_
#define CR2IMAGE_HPP_

// *****************************************************************************
// included header files
#include "image.hpp"
#include "basicio.hpp"
#include "tifffwd.hpp"
#include "types.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    // Add CR2 to the supported image formats
    namespace ImageType {
        const int cr2 = 7;          //!< CR2 image type (see class Cr2Image)
    }

    /*!
      @brief Class to access raw Canon CR2 images.  Exif metadata
          is supported directly, IPTC is read from the Exif data, if present.
     */
    class Cr2Image : public Image {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that can either open an existing CR2 image or create
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
        Cr2Image(BasicIo::AutoPtr io, bool create);
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
          @brief Todo: Not supported yet, requires writeMetadata(). Calling 
              this function will throw an Error(32).
         */
        void setExifData(const ExifData& exifData);
        /*!
          @brief Todo: Not supported yet, requires writeMetadata(). Calling 
              this function will throw an Error(32).
         */
        void setIptcData(const IptcData& iptcData);
        /*!
          @brief Not supported. CR2 format does not contain a comment.
              Calling this function will throw an Error(32).
         */
        void setComment(const std::string& comment);
        //@}

    private:
        //! @name NOT implemented
        //@{
        //! Copy constructor
        Cr2Image(const Cr2Image& rhs);
        //! Assignment operator
        Cr2Image& operator=(const Cr2Image& rhs);
        //@}

    }; // class Cr2Image

    /*!
      @brief Table of Cr2 decoding functions and find function. See
             TiffDecoder for details.
     */
    class Cr2Decoder {
    public:
        /*!
          @brief Find the decoder function for a key.

          If the returned pointer is 0, the tag should not be decoded,
          else the decoder function should be used.

          @param make Camera make
          @param extendedTag Extended tag
          @param group %Group

          @return Pointer to the decoder function
         */
        static const DecoderFct findDecoder(const std::string& make,
                                                  uint32_t     extendedTag,
                                                  uint16_t     group);

    private:
        static const TiffDecoderInfo cr2DecoderInfo_[]; //<! CR2 decoder table

    }; // class Cr2Decoder

    /*!
      @brief This class models a Canon CR2 header structure.
     */
    class Cr2Header {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        Cr2Header()
            : byteOrder_ (littleEndian),
              offset_    (0x00000010),
              offset2_   (0x00000000)
            {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the CR2 header from a data buffer. Return false if the
                 data buffer does not contain a CR2 header, else true.

          @param pData Pointer to the data buffer.
          @param size  Number of bytes in the data buffer.
         */
        bool read(const byte* pData, uint32_t size);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Write the CR2 header to the binary image \em blob.
                 This method appends to the blob.

          @param blob Binary image to add to.

          @throw Error If the header cannot be written.
         */
        void write(Blob& blob) const;
        /*!
          @brief Print debug info for the CR2 header to \em os.

          @param os Output stream to write to.
          @param prefix Prefix to be written before each line of output.
         */
        void print(std::ostream& os, const std::string& prefix ="") const;
        //! Return the byte order (little or big endian).
        ByteOrder byteOrder() const { return byteOrder_; }
        //! Return the offset to the start of the root directory
        uint32_t offset() const { return offset_; }
        //@}

    private:
        // DATA
        ByteOrder             byteOrder_; //!< Applicable byte order
        uint32_t              offset_;    //!< Offset to the start of the root dir
        uint32_t              offset2_;   //!< Bytes 12-15 from the header

        static const uint16_t tag_;       //!< 42, identifies the buffer as TIFF data
        static const char*    cr2sig_;    //!< Signature for CR2 type TIFF
    }; // class Cr2Header

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new Cr2Image instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    Image::AutoPtr newCr2Instance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a CR2 image.
    bool isCr2Type(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef CR2IMAGE_HPP_
