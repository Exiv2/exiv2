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
  @file    webpimage.hpp
  @brief   An Image subclass to support WEBP image files
  @author  Ben Touchette
           <a href="mailto:draekko.software+exiv2@gmail.com">draekko.software+exiv2@gmail.com</a>
  @date    29-Jul-16
 */
#ifndef WEBPIMAGE_HPP
#define WEBPIMAGE_HPP

// *****************************************************************************
// included header files
#include "exif.hpp"
#include "image.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    // Add WEBP to the supported image formats
    namespace ImageType {
        const int webp = 23; //!< Treating webp as an image type>
    }

    /*!
      @brief Class to access WEBP video files.
     */
    class EXIV2API WebPImage:public Image
    {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor for a WebP video. Since the constructor
              can not return a result, callers should check the good() method
              after object construction to determine success or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method. Use the Image::io()
              method to get a temporary reference.
         */
        WebPImage(BasicIo::AutoPtr io);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata();
        void writeMetadata();
        void printStructure(std::ostream& out, PrintStructureOption option,int depth);
        //@}

        /*!
          @brief Not supported. Calling this function will throw an Error(kerInvalidSettingForImage).
         */
        void setComment(const std::string& comment);
        void setIptcData(const IptcData& /*iptcData*/);

        //! @name Accessors
        //@{
        std::string mimeType() const;
        //@}

    private:
        void doWriteMetadata(BasicIo& outIo);
        //! @name NOT Implemented
        //@{
        long getHeaderOffset(byte *data, long data_size,
                             byte *header, long header_size);
        bool equalsWebPTag(Exiv2::DataBuf& buf ,const char* str);
        void debugPrintHex(byte *data, long size);
        void decodeChunks(uint64_t filesize);
        void inject_VP8X(BasicIo& iIo, bool has_xmp, bool has_exif,
                         bool has_alpha, bool has_icc, int width,
                         int height);

        //! Copy constructor
        WebPImage(const WebPImage& rhs);
        //! Assignment operator
        WebPImage& operator=(const WebPImage& rhs);
        //@}

    private:
        const static byte WEBP_PAD_ODD;
        const static int WEBP_TAG_SIZE;
        const static int WEBP_VP8X_ICC_BIT;
        const static int WEBP_VP8X_ALPHA_BIT;
        const static int WEBP_VP8X_EXIF_BIT;
        const static int WEBP_VP8X_XMP_BIT;
        const static char* WEBP_CHUNK_HEADER_VP8X;
        const static char* WEBP_CHUNK_HEADER_VP8L;
        const static char* WEBP_CHUNK_HEADER_VP8;
        const static char* WEBP_CHUNK_HEADER_ANMF;
        const static char* WEBP_CHUNK_HEADER_ANIM;
        const static char* WEBP_CHUNK_HEADER_ICCP;
        const static char* WEBP_CHUNK_HEADER_EXIF;
        const static char* WEBP_CHUNK_HEADER_XMP;


    }; //Class WebPImage

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new WebPImage instance and return an auto-pointer to it.
          Caller owns the returned object and the auto-pointer ensures that
          it will be deleted.
     */
    EXIV2API Image::AutoPtr newWebPInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a WebP Video.
    EXIV2API bool isWebPType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // WEBPIMAGE_HPP
