// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005 Andreas Huggel <ahuggel@gmx.net>
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
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Brad Schick (brad) 
           <a href="mailto:brad@robotbattle.com">brad@robotbattle.com</a>
  @date    09-Jan-04, ahu: created<BR>
           11-Feb-04, ahu: isolated as a component<BR>
           19-Jul-04, brad: revamped to be more flexible and support Iptc
           15-Jan-05, brad: inside-out design changes
 */
#ifndef IMAGE_HPP_
#define IMAGE_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "basicio.hpp"

// + standard includes
#include <string>
#include <map>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class ExifData;
    class IptcData;

// *****************************************************************************
// class definitions
    /*!
      @brief Abstract base class defining the interface for an image. This is
         the top-level interface to the Exiv2 library.

      Most client apps will obtain an Image instance by calling a static
      ImageFactory method. The Image class can then be used to to
      read, write, and save metadata.      
     */
    class Image {
    public:
        //! Supported image formats
        enum Type { none, jpeg, exv };
     
        //! Image auto_ptr type
        typedef std::auto_ptr<Image> AutoPtr;

        //! @name Creators
        //@{
        //! Virtual Destructor
        virtual ~Image() {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read metadata from assigned image. Before this method
              is called, the various metadata types (Iptc, Exif) will be empty.
          @throw Error In case of failure.
         */
        virtual void readMetadata() =0;
        /*!
          @brief Write metadata back to the image. 

          All existing metadata sections in the image are either created,
          replaced, or erased. If values for a given metadata type have been
          assigned, a section for that metadata type will either be created or
          replaced. If no values have been assigned to a given metadata type,
          any exists section for that metadata type will be removed from the
          image.
          
          @throw Error if the operation fails
         */
        virtual void writeMetadata() =0;
        /*!
          @brief Assign new exif data. The new exif data is not written
              to the image until the writeMetadata() method is called.
          @param exifData An ExifData instance holding exif data to be copied
         */
        virtual void setExifData(const ExifData& exifData) =0;
        /*!
          @brief Erase any buffered Exif data. Exif data is not removed from
              the actual image until the writeMetadata() method is called.
         */
        virtual void clearExifData() =0;
        /*!
          @brief Assign new iptc data. The new iptc data is not written
              to the image until the writeMetadata() method is called.
          @param iptcData An IptcData instance holding iptc data to be copied
         */
        virtual void setIptcData(const IptcData& iptcData) =0;
        /*!
          @brief Erase any buffered Iptc data. Iptc data is not removed from
              the actual image until the writeMetadata() method is called.
         */
        virtual void clearIptcData() =0;
        /*!
          @brief Set the image comment. The new comment is not written
              to the image until the writeMetadata() method is called.
          @param comment String containing comment.
         */
        virtual void setComment(const std::string& comment) =0;
        /*!
          @brief Erase any buffered comment. Comment is not removed
              from the actual image until the writeMetadata() method is called.
         */
        virtual void clearComment() =0;
        /*!
          @brief Copy all existing metadata from source Image. The data is
              copied into internal buffers and is not written to the image
              until the writeMetadata() method is called.
          @param image Metadata source. All metadata types are copied.
         */
        virtual void setMetadata(const Image& image) =0;
        /*!
          @brief Erase all buffered metadata. Metadata is not removed
              from the actual image until the writeMetadata() method is called.
         */
        virtual void clearMetadata() =0;
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Check if the Image instance is valid. Use after object 
                 construction.
          @return true if the Image is in a valid state.
         */
        virtual bool good() const =0;
        /*!
          @brief Returns an ExifData instance containing currently buffered
              exif data.

          The exif data may have been read from the image by
          a previous call to readMetadata() or added directly. The exif
          data in the returned instance will be written to the image when
          writeMetadata() is called.
          
          @return read only ExifData instance containing exif values
         */
        virtual const ExifData& exifData() const =0;
        /*!
          @brief Returns an ExifData instance containing currently buffered
              exif data.

          The contained exif data may have been read from the image by
          a previous call to readMetadata() or added directly. The exif
          data in the returned instance will be written to the image when
          writeMetadata() is called.
          
          @return modifiable ExifData instance containing exif values
         */
        virtual ExifData& exifData() =0;
        /*!
          @brief Returns an IptcData instance containing currently buffered
              iptc data.

          The contained iptc data may have been read from the image by
          a previous call to readMetadata() or added directly. The iptc
          data in the returned instance will be written to the image when
          writeMetadata() is called.
          
          @return modifiable IptcData instance containing iptc values
         */
        virtual const IptcData& iptcData() const =0;
        /*!
          @brief Returns an ExifData instance containing currently buffered
              exif data.

          The contained iptc data may have been read from the image by
          a previous call to readMetadata() or added directly. The iptc
          data in the returned instance will be written to the image when
          writeMetadata() is called.
          
          @return modifiable IptcData instance containing iptc values
         */
        virtual IptcData& iptcData() =0;
        /*!
          @brief Return a copy of the image comment. May be an empty string.
         */
        virtual std::string comment() const =0;
        /*!
          @brief Return a reference to the BasicIo instance being used for Io.
          
          This refence is particularly useful to reading the results of
          operations on a MemIo instance. For example after metadata has
          been modified and the writeMetadata() method has been called,
          this method can be used to get access to the modified image. 
          
          @return BasicIo instance that can be used to read or write image
             data directly.
          @note If the returned BasicIo is used to write to the image, the
             Image class will not see those changes until the readMetadata()
             method is called.
         */
        virtual BasicIo& io() const = 0;
        //@}

    protected:
        //! @name Creators
        //@{
        //! Default Constructor
        Image() {}
        //@}

    private:
        // NOT Implemented
        //! Copy constructor
        Image(const Image& rhs);
        //! Assignment operator
        Image& operator=(const Image& rhs);

    }; // class Image

    //! Type for function pointer that creates new Image instances
    typedef Image::AutoPtr (*NewInstanceFct)(BasicIo::AutoPtr io, bool create);
    //! Type for function pointer that checks image types
    typedef bool (*IsThisTypeFct)(BasicIo& iIo, bool advance);

    /*!
      @brief Returns an Image instance of the specified type.

      The factory is implemented as a singleton, which can be accessed
      through static member functions.
    */
    class ImageFactory {
    public:
        //! @name Manipulators
        //@{
        //! Destructor.
        static void cleanup();
        /*!
          @brief Register image type together with its function pointers.

          The image factory creates new images by calling their associated
          function pointer. Additional images can be added by registering
          new type and function pointers. If called for a type that already
          exists in the list, the corresponding functions are replaced.

          @param type Image type.
          @param newInst Function pointer for creating image instances.
          @param isType Function pointer to test for matching image types.
        */
        static void registerImage(Image::Type type, 
                                  NewInstanceFct newInst, 
                                  IsThisTypeFct isType);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Create an Image subclass of the appropriate type by reading
              the specified file. %Image type is derived from the file
              contents. 
          @param  path %Image file. The contents of the file are tested to
              determine the image type. File extension is ignored.
          @return An auto-pointer that owns an Image instance whose type 
              matches that of the file. 
          @throw Error If opening the file fails or it contains data of an
              unknown image type.
         */
        static Image::AutoPtr open(const std::string& path);
        /*!
          @brief Create an Image subclass of the appropriate type by reading
              the provided memory. %Image type is derived from the memory
              contents. 
          @param data Pointer to a data buffer containing an image. The contents
              of the memory are tested to determine the image type.
          @param size Number of bytes pointed to by \em data.
          @return An auto-pointer that owns an Image instance whose type 
              matches that of the data buffer.
          @throw Error If the memory contains data of an unknown image type.
         */
        static Image::AutoPtr open(const byte* data, long size);
        /*!
          @brief Create an Image subclass of the appropriate type by reading
              the provided BasicIo instance. %Image type is derived from the
              data provided by \em io. The passed in \em io instance is
              (re)opened by this method. 
          @param io An auto-pointer that owns a BasicIo instance that provides
              image data. The contents of the image data are tested to determine
              the type. 
          @note This method takes ownership of the passed
              in BasicIo instance through the auto-pointer. Callers should not
              continue to use the BasicIo instance after it is passed to this method.
              Use the Image::io() method to get a temporary reference.
          @return An auto-pointer that owns an Image instance whose type 
              matches that of the \em io data. If no image type could be
              determined, the pointer is 0.
          @throw Error If opening the BasicIo fails
         */
        static Image::AutoPtr open(BasicIo::AutoPtr io);
        /*!
          @brief Create an Image subclass of the requested type by creating a
              new image file. If the file already exists, it will be overwritten.
          @param type Type of the image to be created.
          @param path %Image file to create. File extension is ignored.
          @return An auto-pointer that owns an Image instance of the requested
              type. 
          @throw Error If the image type is not supported.
         */
        static Image::AutoPtr create(Image::Type type, const std::string& path);
        /*!
          @brief Create an Image subclass of the requested type by creating a
              new image in memory.
          @param type Type of the image to be created.
          @return An auto-pointer that owns an Image instance of the requested
              type. 
          @throw Error If the image type is not supported
         */
        static Image::AutoPtr create(Image::Type type);
        /*!
          @brief Create an Image subclass of the requested type by writing a
              new image to a BasicIo instance. If the BasicIo instance already
              contains data, it will be overwritten.
          @param type Type of the image to be created.
          @param io An auto-pointer that owns a BasicIo instance that will
              be written to when creating a new image. 
          @note This method takes ownership of the passed in BasicIo instance
              through the auto-pointer. Callers should not continue to use the 
              BasicIo instance after it is passed to this method.  Use the 
              Image::io() method to get a temporary reference.
          @return An auto-pointer that owns an Image instance of the requested
              type. If the image type is not supported, the pointer is 0.
         */
        static Image::AutoPtr create(Image::Type type, BasicIo::AutoPtr io);
        /*!
          @brief Returns the image type of the provided file. 
          @param path %Image file. The contents of the file are tested to
              determine the image type. File extension is ignored.
          @return %Image type or Image::none if the type is not recognized.
         */
        static Image::Type getType(const std::string& path);
        /*!
          @brief Returns the image type of the provided data buffer. 
          @param data Pointer to a data buffer containing an image. The contents
              of the memory are tested to determine the image type.
          @param size Number of bytes pointed to by \em data.
          @return %Image type or Image::none if the type is not recognized.
         */
        static Image::Type getType(const byte* data, long size);
        /*!
          @brief Returns the image type of data provided by a BasicIo instance.
              The passed in \em io instance is (re)opened by this method.
          @param io A BasicIo instance that provides image data. The contents
              of the image data are tested to determine the type.
          @return %Image type or Image::none if the type is not recognized.
         */
        static Image::Type getType(BasicIo& io);
        //@}

        /*!
          @brief Class Init is used to execute initialisation and termination 
                 code exactly once, at the begin and end of the program.

          See Bjarne Stroustrup, 'The C++ Programming Language 3rd
          Edition', section 21.5.2 for details about this pattern.
        */
        class Init {
            static int count;           //!< Counts calls to constructor
        public:
            //! @name Creators
            //@{                            
            //! Perform one-time initialisations.
            Init();
            //! Perform one-time cleanup operations.
            ~Init();
            //@}
        };

    private:
        //! @name Creators
        //@{
        //! Prevent construction other than through instance().
        ImageFactory();
        //! Prevent copy construction: not implemented.
        ImageFactory(const ImageFactory& rhs);
        //! Creates the private static instance
        static void init();
        //@}

        //! Struct for storing image function pointers.
        struct ImageFcts
        {
            NewInstanceFct newInstance;
            IsThisTypeFct isThisType;
            ImageFcts(NewInstanceFct newInst, IsThisTypeFct isType) 
                : newInstance(newInst), isThisType(isType) {}
            ImageFcts() : newInstance(0), isThisType(0) {}
        };

        // DATA
        //! Type used to store Image creation functions
        typedef std::map<Image::Type, ImageFcts> Registry;
        //! List of image types and corresponding creation functions.
        static Registry* registry_;
    }; // class ImageFactory


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
        int read(const byte* buf);
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
        long copy(byte* buf) const;
        //! Return the size of the TIFF header in bytes.
        long size() const { return 8; }
        //! Return the byte order (little or big endian).
        ByteOrder byteOrder() const { return byteOrder_; }
        //! Return the tag value.
        uint16_t tag() const { return tag_; }
        /*!
          @brief Return the offset to IFD0 from the start of the TIFF header.
                 The offset is 0x00000008 if IFD0 begins immediately after the 
                 TIFF header.
         */
        uint32_t offset() const { return offset_; }
        //@}

    private:
        ByteOrder byteOrder_;
        uint16_t tag_;
        uint32_t offset_;

    }; // class TiffHeader   

}                                       // namespace Exiv2

namespace {
    /*!
      Each translation unit that includes image.hpp declares its own
      Init object. The destructor ensures that the factory is properly
      freed exactly once.

      See Bjarne Stroustrup, 'The C++ Programming Language 3rd
      Edition', section 21.5.2 for details about this pattern.
    */
    Exiv2::ImageFactory::Init imageFactoryInit;
}

#endif                                  // #ifndef IMAGE_HPP_
