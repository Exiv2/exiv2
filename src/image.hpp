// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*!
  @file    image.hpp
  @brief   Class JpegImage to access JPEG images
  @version $Name:  $ $Revision: 1.3 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    09-Jan-04, ahu: created
           11-Feb-04, ahu: isolated as a component
 */
#ifndef IMAGE_HPP_
#define IMAGE_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <string>
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exif {

// *****************************************************************************
// class definitions

    /*! 
      @brief Helper class to access JPEG images
     */
    class JpegImage {
        //! @name Not implemented
        //@{
        //! Copying not allowed
        JpegImage(const JpegImage& rhs);
        //! Assignment not allowed
        JpegImage& operator=(const JpegImage& rhs);
        //@}

    public:
        //! @name Creators
        //@{
        //! Default constructor
        JpegImage();
        //! Destructor
        ~JpegImage();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the %Exif data from the file path into the internal 
                 data buffer.
          @param path Path to the file.
          @return 0 if successful;<BR>
                 -1 if the file cannot be opened; or<BR>
                    the return code of readExifData(std::istream& is)
                    if the call to this function fails.
         */
        int readExifData(const std::string& path);
        /*!
          @brief Read the %Exif data from the stream into the internal 
                 data buffer.
          @param is Input stream to read from.
          @return 0 if successful;<BR>
                  1 if reading from the stream failed (consult the stream state 
                    for more information);<BR>
                  2 if the stream does not contain a JPEG image;<BR>
                  3 if no %Exif APP1 segment was found after SOI at the 
                    beginning of the input stream.
         */
        int readExifData(std::istream& is);
        /*!
          @brief Set the %Exif data. The data is copied into the internal
                 data buffer.
          @param buf Pointer to the data buffer.
          @param size Number of characters in the data buffer.
         */
        void setExifData(const char* buf, long size);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Write the %Exif data to file path, which must contain a JPEG
                 image. If an %Exif APP1 section exists in the file, it is
                 replaced. Otherwise, an %Exif data section is created.
          @param path Path to the file.
          @return 0 if successful;<br>
                 -1 if the input file cannot be opened;<br>
                 -2 if the temporary file cannot be opened;<br>
                 -3 if renaming the temporary file fails; or<br>
                 the return code of 
                    writeExifData(std::ostream& os, std::istream& is) const
                    if the call to this function fails.

         */
        int writeExifData(const std::string& path) const;
        /*!
          @brief Copy %Exif data into the JPEG image is, write the resulting
                 image to the output stream os. If an %Exif APP1 section exists
                 in the input file, it is replaced. Otherwise, an %Exif data
                 section is created.
          @param os Output stream to write to (e.g., a temporary file).
          @param is Input stream with the JPEG image to which the %Exif data
                 should be copied.
          @return 0 if successful;<BR>
                  1 if reading from the input stream failed (consult the stream 
                    state for more information);<BR>
                  2 if the input stream does not contain a JPEG image;<BR>
                  3 if neither a JFIF APP0 segment nor a %Exif APP1 segment was
                    found after SOI at the beginning of the input stream;<BR>
                  4 if writing to the output stream failed (consult the stream 
                    state for more information).
         */
        int writeExifData(std::ostream& os, std::istream& is) const;
        //! Return the size of the %Exif data buffer
        long sizeExifData() const { return sizeExifData_; }
        //! Return a read-only pointer to the %Exif data buffer 
        const char* exifData() const { return pExifData_; }
        //@}

        /*!
          @brief Check if the stream is the beginning of a JPEG image.
          @param is Input stream to test.
          @return true if the input stream starts the JPEG SOI marker.
                  The stream is advanced by two characters in this case.<br>
                  false if the input stream does not begin with the JPEG SOI
                  marker. The stream is not advanced in this case.<br>
                  false if reading the first two bytes from the stream fails.
                  Consult the stream state for more information. In this case,
                  the stream may or may not have been advanced by 1 or 2 
                  characters.
         */
        static bool isJpeg(std::istream& is);

    private:
        // DATA
        static const uint16 soi_;               // SOI marker
        static const uint16 app0_;              // APP0 marker
        static const uint16 app1_;              // APP1 marker
        static const char exifId_[];            // Exif identifier
        static const char jfifId_[];            // JFIF identifier

        long sizeExifData_;                     // Size of the Exif data buffer
        char* pExifData_;                       // Exif data buffer

    }; // class JpegImage

    //! Helper class modelling the TIFF header structure.
    class TiffHeader {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Default constructor. Optionally sets the byte order 
                 (default: little endian).
         */
        explicit TiffHeader(ByteOrder byteOrder =littleEndian);
        //@}

        //! @name Manipulators
        //@{
        //! Read the TIFF header from a data buffer. Returns 0 if successful.
        int read(const char* buf);
        //@}

        //! @name Accessors
        //@{
        /*! 
          @brief Write a standard TIFF header into buf as a data string, return
                 number of bytes copied.

          Only the byte order of the TIFF header varies, the values written for
          offset and tag are constant, i.e., independent of the values possibly
          read before a call to this function. The value 0x00000008 is written
          for the offset, tag is set to 0x002a.

          @param buf The data buffer to write to.
          @return The number of bytes written.
         */
        long copy(char* buf) const;
        //! Return the size of the TIFF header in bytes.
        long size() const { return 8; }
        //! Return the byte order (little or big endian).
        ByteOrder byteOrder() const { return byteOrder_; }
        //! Return the tag value.
        uint16 tag() const { return tag_; }
        /*!
          @brief Return the offset to IFD0 from the start of the TIFF header.
                 The offset is 0x00000008 if IFD0 begins immediately after the 
                 TIFF header.
         */
        uint32 offset() const { return offset_; }
        //@}

    private:
        ByteOrder byteOrder_;
        uint16 tag_;
        uint32 offset_;

    }; // class TiffHeader
   
}                                       // namespace Exif

#endif                                  // #ifndef IMAGE_HPP_
