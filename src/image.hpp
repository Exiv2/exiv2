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
  @version $Name:  $ $Revision: 1.4 $
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
#include <map>

// *****************************************************************************
// namespace extensions
namespace Exif {

// *****************************************************************************
// class definitions

    /*!
      @brief Abstract base class defining the interface for an image.
     */
    class Image {
    public:
        //! Supported image formats
        enum Type { none, jpeg, exiv2 };

        //! @name Creators
        //@{
        //! Default Constructor
        Image() {}
        //! Virtual Destructor
        virtual ~Image() {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the %Exif data from the file path into the internal 
                 data buffer.
          @param path Path to the file.
          @return 0 if successful.         
         */
        virtual int readExifData(const std::string& path) =0;
        /*!
          @brief Read the %Exif data from the stream into the internal 
                 data buffer.
          @param is Input stream to read from.
          @return 0 if successful.
         */
        virtual int readExifData(std::istream& is) =0;
        /*!
          @brief Read the %Exif data from the buffer buf which has size bytes.
          @param buf Pointer to the data buffer.
          @param size Number of characters in the data buffer.
         */
        virtual void setExifData(const char* buf, long size) =0;
        //@}

        //! @name Accessors
        //@{
        //! Virtual copy construction
        virtual Image* clone() const =0;
        /*!
          @brief Determine if the file path is of this type of image.
          @return  0 if the type of the image matches that of this;<BR>
                   1 if the type of the image is not that of this;<BR>
                  -1 if the file cannot be opened.
         */
        virtual int isThisType(const std::string& path) const =0;
        /*!
          @brief Write the %Exif data to file path.
          @param path Path to the file.
          @return 0 if successful.
         */
        virtual int writeExifData(const std::string& path) const =0;
        /*!
          @brief Read from the image input stream is, add %Exif data to the
                 image, replacing existing %Exif data, if there is any) and
                 write the resulting image to the output stream os.
          @param os Output stream to write to (e.g., a temporary file).
          @param is Input stream with the image to which the %Exif data
                 should be copied.
          @return 0 if successful.
         */
        virtual int writeExifData(std::ostream& os, std::istream& is) const =0;
        //! Return the size of the %Exif data in bytes.
        virtual long sizeExifData() const =0;
        /*!
          @brief Return a read-only pointer to an %Exif data buffer. Do not
                 attempt to write to this buffer.
         */
        virtual const char* exifData() const =0;
        //@}

    protected:
        /*!
          @brief Assignment operator. Protected so that it can only be used
                 by subclasses but not directly.
         */
        Image& operator=(const Image& rhs) { return *this; }

    }; // class Image

    /*!
      @brief Image factory.

      Creates an instance of the image of the requested type.  The factory is
      implemented as a singleton, which can be accessed only through the static
      member function instance().
    */
    class ImageFactory {
    public:
        /*!
          @brief Get access to the image factory.

          Clients access the image factory exclusively through
          this method.
        */
        static ImageFactory& instance();

        /*!
          @brief  Create an %Image of the appropriate type, derived from path.

          @param  path Path to an image file (which may or may not exist). The
                  path is used to determine the image type to create.
          @return A pointer that owns an %Image of the type derived from path. 
                  If no image type could be determined, the pointer is 0.
         */
        Image* create(const std::string& path);

        /*!
          @brief  Create an %Image of the requested type.

          @param  type Type of the image to be created.
          @return A pointer that owns an %Image of the requested type.
                  If the image type is not supported, the pointer is 0.
         */
        Image* create(Image::Type type);

        /*!
          @brief Register an image prototype together with its type.

          The image factory creates new images by cloning their associated
          prototypes. Additional images can be added by registering a prototype
          and its type. If called for a type which already exists in the list,
          the corresponding prototype is replaced.

          @param type Image type.
          @param pImage Pointer to the prototype. Ownership is transfered to the
                 factory.
        */
        void registerImage(Image::Type type, Image* pImage);

    private:
        //! Prevent construction other than through instance().
        ImageFactory();
        //! Prevent copy construction: not implemented.
        ImageFactory(const ImageFactory& rhs);

        //! Pointer to the one and only instance of this class.
        static ImageFactory* pInstance_;
        //! Type used to store Image prototype classes
        typedef std::map<Image::Type, Image*> Registry;
        //! List of image types and corresponding prototypes.
        Registry registry_;

    }; // class ImageFactory

    /*! 
      @brief Helper class to access JPEG images
     */
    class JpegImage : public Image {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        JpegImage();
        //! Copy constructor
        JpegImage(const JpegImage& rhs);
        //! Destructor
        ~JpegImage();
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator
        JpegImage& operator=(const JpegImage& rhs);
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
        //! Virtual copy construction
        Image* clone() const;
        /*!
          @brief Determine if the file path contains a JPEG file. Return true if it
                 does, false if not or if path cannot be opened or accessed.
         */
        int isThisType(const std::string& path) const;
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
