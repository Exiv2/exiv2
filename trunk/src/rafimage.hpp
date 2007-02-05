// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2007 Andreas Huggel <ahuggel@gmx.net>
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
  @file    rafimage.hpp
  @brief   Fujifilm RAW image
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    05-Feb-07, ahu: created
 */
#ifndef RAFIMAGE_HPP_
#define RAFIMAGE_HPP_

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

    // Add RAF to the supported image formats
    namespace ImageType {
        const int raf = 8;          //!< RAF image type (see class RafImage)
    }

    /*!
      @brief Class to access raw RAF images. Exif metadata is supported
             directly, IPTC is read from the Exif data, if present.
     */
    class RafImage : public Image {
        //! @name NOT Implemented
        //@{
        //! Copy constructor
        RafImage(const RafImage& rhs);
        //! Assignment operator
        RafImage& operator=(const RafImage& rhs);
        //@}

    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that can either open an existing RAF image or create
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
        RafImage(BasicIo::AutoPtr io, bool create);
        //! Destructor
        ~RafImage() {}
        //@}

        //! @name Manipulators
        //@{
        void            readMetadata();
        /*!
          @brief Todo: Write metadata back to the image. This method is not
              yet implemented. Calling it will throw an Error(31).
         */
        void            writeMetadata();
        void            setExifData(const ExifData& exifData);
        void            clearExifData();
        void            setIptcData(const IptcData& iptcData);
        void            clearIptcData();
        /*!
          @brief Not supported. RAF format does not contain a comment.
              Calling this function will throw an Error(32).
         */
        void            setComment(const std::string& comment);
        void            clearComment();
        void            setMetadata(const Image& image);
        void            clearMetadata();
        ExifData&       exifData()       { return exifData_; }
        IptcData&       iptcData()       { return iptcData_; }
        //@}

        //! @name Accessors
        //@{
        bool            good()     const;
        const ExifData& exifData() const { return exifData_; }
        const IptcData& iptcData() const { return iptcData_; }
        std::string     comment()  const { return comment_; }
        BasicIo&        io()       const { return *io_; }
        AccessMode      checkMode(MetadataId metadataId) const;
        //@}

    private:
        //! @name Accessors
        //@{
        /*!
          @brief Determine if the content of the BasicIo instance is a RAF image.

          The advance flag determines if the read position in the stream is
          moved (see below). This applies only if the type matches and the
          function returns true. If the type does not match, the stream
          position is not changed. However, if reading from the stream fails,
          the stream position is undefined. Consult the stream state to obtain
          more information in this case.

          @param iIo BasicIo instance to read from.
          @param advance Flag indicating whether the position of the io
              should be advanced by the number of characters read to
              analyse the data (true) or left at its original
              position (false). This applies only if the type matches.
          @return  true  if the data matches the type of this class;<BR>
                   false if the data does not match
         */
        bool isThisType(BasicIo& iIo, bool advance) const;
        /*!
          @brief Todo: Write RAF header. Not implemented yet.
         */
        int writeHeader(BasicIo& oIo) const;
        //@}

        // DATA
        BasicIo::AutoPtr  io_;                  //!< Image data io pointer
        ExifData          exifData_;            //!< Exif data container
        IptcData          iptcData_;            //!< IPTC data container
        std::string       comment_;             //!< User comment

    }; // class RafImage

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new RafImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    Image::AutoPtr newRafInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a RAF image.
    bool isRafType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef RAFIMAGE_HPP_
