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
  @file    tiffimage.hpp
  @brief   Class TiffImage
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    15-Mar-06, ahu: created
 */
#ifndef TIFFIMAGE_HPP_
#define TIFFIMAGE_HPP_

// *****************************************************************************
// included header files
#include "image.hpp"
#include "basicio.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    // Add TIFF to the supported image formats
    namespace ImageType {
        const int tiff = 4;          //!< TIFF image type (see class TiffImage)
    }

    /*!
      @brief Class to access TIFF images. Exif metadata is
          supported directly, IPTC is read from the Exif data, if present.
     */
    class TiffImage : public Image {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that can either open an existing TIFF image or create
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
        TiffImage(BasicIo::AutoPtr io, bool create);
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
          @brief Not supported. TIFF format does not contain a comment.
              Calling this function will throw an Error(32).
         */
        void setComment(const std::string& comment);
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const { return "image/tiff"; }
        //@}

    private:
        //! @name NOT Implemented
        //@{
        //! Copy constructor
        TiffImage(const TiffImage& rhs);
        //! Assignment operator
        TiffImage& operator=(const TiffImage& rhs);
        //@}

    }; // class TiffImage

    /*!
      @brief Abstract base class defining the interface of an image header.
             Used internally by classes for TIFF-based images.  Default
             implementation is for the regular TIFF header.
     */
    class TiffHeaderBase {
    public:
        //! @name Constructors
        //@{
        //! Constructor taking \em tag, \em size and default \em byteOrder and \em offset.
        TiffHeaderBase(uint16_t  tag,
                       uint32_t  size,
                       ByteOrder byteOrder,
                       uint32_t  offset);
        //! Virtual destructor.
        virtual ~TiffHeaderBase() =0;
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the image header from a data buffer. Return false if the
                 data buffer does not contain an image header of the expected
                 format, else true.

          @param pData Pointer to the data buffer.
          @param size  Number of bytes in the data buffer.
         */
        virtual bool read(const byte* pData, uint32_t size);
        //! Set the byte order.
        virtual void setByteOrder(ByteOrder byteOrder);
        //! Set the offset to the start of the root directory.
        virtual void setOffset(uint32_t offset);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Write the image header to the binary image \em blob.
                 This method appends to the blob.

          @param blob Binary image to add to.

          @throw Error If the header cannot be written.
         */
        virtual void write(Blob& blob) const;
        /*!
          @brief Print debug info for the image header to \em os.

          @param os Output stream to write to.
          @param prefix Prefix to be written before each line of output.
         */
        virtual void print(std::ostream& os, const std::string& prefix ="") const;
        //! Return the byte order (little or big endian).
        virtual ByteOrder byteOrder() const;
        //! Return the offset to the start of the root directory.
        virtual uint32_t offset() const;
        //! Return the size (in bytes) of the image header.
        virtual uint32_t size() const;
        //! Return the tag value (magic number) which identifies the buffer as TIFF data
        virtual uint16_t tag() const;
        //@}

    private:
        // DATA
        const uint16_t tag_;       //!< Tag to identify the buffer as TIFF data
        const uint32_t size_;      //!< Size of the header
        ByteOrder      byteOrder_; //!< Applicable byte order
        uint32_t       offset_;    //!< Offset to the start of the root dir

    }; // class TiffHeaderBase

    /*!
      @brief Standard TIFF header structure.
     */
    class TiffHeade2 : public TiffHeaderBase {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffHeade2();
        //! Destructor
        ~TiffHeade2();
        //@}
    }; // class TiffHeade2

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new TiffImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    Image::AutoPtr newTiffInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a TIFF image.
    bool isTiffType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef TIFFIMAGE_HPP_
