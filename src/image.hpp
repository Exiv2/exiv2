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
  @version $Name:  $ $Revision: 1.22 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Brad Schick (brad) 
           <a href="mailto:schick@robotbattle.com">schick@robotbattle.com</a>
  @date    09-Jan-04, ahu: created<BR>
           11-Feb-04, ahu: isolated as a component<BR>
           19-Jul-04, brad: revamped to be more flexible and support Iptc
 */
#ifndef IMAGE_HPP_
#define IMAGE_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <string>
#include <map>
#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
 
// *****************************************************************************
// class definitions

    /*!
      @brief Abstract base class defining the interface for an image.
     */
    class Image {
    public:
        //! Image auto_ptr type
        typedef std::auto_ptr<Image> AutoPtr;

        //! Supported image formats
        enum Type { none, jpeg, exv };

        //! @name Creators
        //@{
        //! Virtual Destructor
        virtual ~Image() {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read metadata from assigned image file into internal 
                 buffers.
          @return 0 if successful.
         */
        virtual int readMetadata() =0;
        /*!
          @brief Write metadata from internal buffers into to the image fle.
          @return 0 if successful.
         */
        virtual int writeMetadata() =0;
        /*!
          @brief Set the Exif data. The data is copied into an internal data
                 buffer and is not written until writeMetadata is called.
          @param buf Pointer to the new Exif data.
          @param size Size in bytes of new Exif data.
         */
        virtual void setExifData(const byte* buf, long size) =0;
        /*!
          @brief Erase any buffered Exif data. Exif data is not removed
                from the actual file until writeMetadata is called.
         */
        virtual void clearExifData() =0;
        /*!
          @brief Set the Iptc data. The data is copied into an internal data
                 buffer and is not written until writeMetadata is called.
          @param buf Pointer to the new Iptc data.
          @param size Size in bytes of new Iptc data.
         */
        virtual void setIptcData(const byte* buf, long size) =0;
        /*!
          @brief Erase any buffered Iptc data. Iptc data is not removed
                from the actual file until writeMetadata is called.
         */
        virtual void clearIptcData() =0;
        /*!
          @brief Set the image comment. The data is copied into an internal data
                 buffer and is not written until writeMetadata is called.
          @param comment String containing comment.
         */
        virtual void setComment(const std::string& comment) =0;
        /*!
          @brief Erase any buffered comment. Comment is not removed
                 from the actual file until writeMetadata is called.
         */
        virtual void clearComment() =0;
        /*!
          @brief Copy all existing metadata from source %Image. The data is
                 copied into internal buffers and is not written until
                 writeMetadata is called.
          @param image Metadata source. All metadata types are copied.
         */
        virtual void setMetadata(const Image& image) =0;
        /*!
          @brief Erase all buffered metadata. Metadata is not removed
                 from the actual file until writeMetadata is called.
         */
        virtual void clearMetadata() =0;
        /*!
          @brief Close associated image file but preserve buffered metadata.
          @return 0 if successful.         
         */
        virtual int detach() =0;
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Check if the %Image instance is valid. Use after object 
                 construction.
          @return true if the %Image is in a valid state.
         */
        virtual bool good() const =0;
        //! Return the size of the Exif data in bytes.
        virtual long sizeExifData() const =0;
        /*!
          @brief Return a read-only pointer to an Exif data buffer. Do not
                 attempt to write to this buffer.
         */
        virtual const byte* exifData() const =0;
        //! Return the size of the Iptc data in bytes.
        virtual long sizeIptcData() const =0;
        /*!
          @brief Return a read-only pointer to an Iptc data buffer. Do not
                 attempt to write to this buffer.
         */
        virtual const byte* iptcData() const =0;
        /*!
          @brief Return a copy of the image comment. May be an empty string.
         */
        virtual std::string comment() const =0;
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
    typedef Image::AutoPtr (*NewInstanceFct)(const std::string& path, 
                                                   FILE* ifp);
    //! Type for function pointer that checks image types
    typedef bool (*IsThisTypeFct)(FILE* ifp, bool advance);

    /*!
      @brief Image factory.

      Creates an instance of the image of the requested type.  The factory is
      implemented as a singleton, which can be accessed only through the static
      member function instance().
    */
    class ImageFactory {
    public:
        //! @name Manipulators
        //@{
        /*!
          @brief Register image type together with its function pointers.

          The image factory creates new images calling their associated
          function pointer. Additional images can be added by registering
          new type and function pointers. If called for a type that already
          exists in the list, the corresponding prototype is replaced.

          @param type Image type.
          @param newInst Function pointer for creating image instances.
          @param isType Function pointer to test for matching image types.
        */
        void registerImage(Image::Type type, 
                           NewInstanceFct newInst, 
                           IsThisTypeFct isType);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief  Create an %Image of the appropriate type by opening the
                  specified file. File type is derived from the contents of the
                  file.
          @param  path %Image file. The contents of the file are tested to
                  determine the image type to open. File extension is ignored.
          @return An auto-pointer that owns an %Image of the type derived from
                  the file. If no image type could be determined, the pointer is 0.
         */
        Image::AutoPtr open(const std::string& path) const;
        /*!
          @brief  Create an %Image of the requested type by creating a new
                  file. If the file already exists, it will be overwritten.
          @param  type Type of the image to be created.
          @param  path %Image file. The contents of the file are tested to
                  determine the image type to open. File extension is ignored.
          @return An auto-pointer that owns an %Image of the requested type. 
                  If the image type is not supported, the pointer is 0.
         */
        Image::AutoPtr create(Image::Type type, const std::string& path) const;
        /*!
          @brief  Returns the image type of the provided file. 
          @param  path %Image file. The contents of the file are tested to
                  determine the image type. File extension is ignored.
          @return %Image type of Image::none if the type is not recognized.
         */
        Image::Type getType(const std::string& path) const;
        //@}

        /*!
          @brief Get access to the image factory.

          Clients access the image factory exclusively through
          this method.
        */
        static ImageFactory& instance();

    private:
        //! @name Creators
        //@{
        //! Prevent construction other than through instance().
        ImageFactory();
        //! Prevent copy construction: not implemented.
        ImageFactory(const ImageFactory& rhs);
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
        //! Pointer to the one and only instance of this class.
        static ImageFactory* pInstance_;
        //! Type used to store Image creation functions
        typedef std::map<Image::Type, ImageFcts> Registry;
        //! List of image types and corresponding creation functions.
        Registry registry_;

    }; // class ImageFactory

    /*! 
      @brief Abstract helper base class to access JPEG images
     */
    class JpegBase : public Image {
    public:
        //! @name Creators
        //@{
        //! Virtual destructor.
        virtual ~JpegBase();
        //@}
        //! @name Manipulators
        //@{
        /*!
          @brief Read all metadata from the file into the internal 
                 data buffers. This method returns success even when
                 no metadata is found in the image. Callers must therefore
                 check the size of indivdual metadata types before
                 accessing the data.
          @return 0 if successful;<BR>
                  1 if reading from the file failed 
                    (could be caused by invalid image);<BR>
                  2 if the file does not contain a valid image;<BR>
         */
        int readMetadata();
        /*!
          @brief Write all buffered metadata to associated file. All existing
                metadata sections in the file are either replaced or erased.
                If data for a given metadata type has not been assigned,
                then that metadata type will be erased from the file.
          @return 0 if successful;<br>
                  1 if reading from the file failed;<BR>
                  2 if the file does not contain a valid image;<BR>
                  4 if the temporary output file can not be written to;<BR>
                  -1 if the newly created file could not be reopened;<BR>
                  -3 if the temporary output file can not be opened;<BR>
                  -4 if renaming the temporary file fails;<br>
         */
        int writeMetadata();
        void setExifData(const byte* buf, long size);
        void clearExifData();
        void setIptcData(const byte* buf, long size);
        void clearIptcData();
        void setComment(const std::string& comment);
        void clearComment();
        void setMetadata(const Image& image);
        void clearMetadata();
        int detach();
        //@}

        //! @name Accessors
        //@{
        bool good() const;
        long sizeExifData() const { return sizeExifData_; }
        const byte* exifData() const { return pExifData_; }
        long sizeIptcData() const { return sizeIptcData_; }
        const byte* iptcData() const { return pIptcData_; }
        std::string comment() const { return comment_; }
        //@}

    protected:
        //! @name Creators
        //@{
        /*! 
          @brief Constructor for subclasses that have already opened a
                 file stream on the specified path.
          @param path Full path to image file.
          @param fp File pointer to open file.
         */
        JpegBase(const std::string& path, FILE* fp);
        /*! 
          @brief Constructor that can either open an existing image or create
                 a new image from scratch. If a new image is to be created, any
                 existing file is overwritten
          @param path Full path to image file.
          @param create Specifies if an existing file should be opened (false)
                 or if a new file should be created (true).
          @param initData Data to initialize newly created files. Only used
                 when %create is true. Should contain the data for the smallest
                 valid image of the calling subclass.
          @param dataSize Size of initData in bytes.
         */
        JpegBase(const std::string& path, bool create,
                 const byte initData[], size_t dataSize);
        //@}
        //! @name Accessors
        //@{
        /*!
          @brief Writes the image header (aka signature) to the file stream.
          @param ofp File stream that the header is written to.
          @return 0 if successful;<BR>
                 4 if the output file can not be written to;<BR>
         */
        virtual int writeHeader(FILE* ofp) const =0;
        /*!
          @brief Determine if the content of the stream is of the type of this
                 class.

          The advance flag determines if the read position in the stream is
          moved (see below). This applies only if the type matches and the
          function returns true. If the type does not match, the stream
          position is not changed. However, if reading from the stream fails,
          the stream position is undefined. Consult the stream state to obtain 
          more information in this case.
          
          @param ifp Input file stream.
          @param advance Flag indicating whether the read position in the stream
                         should be advanced by the number of characters read to
                         analyse the stream (true) or left at its original
                         position (false). This applies only if the type matches.
          @return  true  if the stream data matches the type of this class;<BR>
                   false if the stream data does not match;<BR>
         */
        virtual bool isThisType(FILE* ifp, bool advance) const =0;
        //@}

        // Constant Data
        static const byte sos_;                 //!< JPEG SOS marker
        static const byte eoi_;                 //!< JPEG EOI marker
        static const byte app0_;                //!< JPEG APP0 marker
        static const byte app1_;                //!< JPEG APP1 marker
        static const byte app13_;               //!< JPEG APP13 marker
        static const byte com_;                 //!< JPEG Comment marker
        static const char exifId_[];            //!< Exif identifier
        static const char jfifId_[];            //!< JFIF identifier
        static const char ps3Id_[];             //!< Photoshop marker
        static const char bimId_[];             //!< Photoshop marker
        static const uint16_t iptc_;              //!< Photoshop Iptc marker

    private:
        // DATA
        FILE* fp_;                              //!< Image file (read write)
        const std::string path_;                //!< Image file name
        long sizeExifData_;                     //!< Size of the Exif data buffer
        byte* pExifData_;                       //!< Exif data buffer
        long sizeIptcData_;                     //!< Size of the Iptc data buffer
        byte* pIptcData_;                       //!< Iptc data buffer
        std::string comment_;                   //!< JPEG comment

        // METHODS
        /*!
          @brief Advances file stream to one byte past the next Jpeg marker
                 and returns the marker. This method should be called when the
                 file stream is positioned one byte past the end of a Jpeg segment.
          @return the next Jpeg segment marker if successful;<BR>
                 -1 if a maker was not found before EOF;<BR>
         */
        int advanceToMarker() const;
        /*!
          @brief Locates Photoshop formated Iptc data in a memory buffer.
                 Operates on raw data (rather than file streams) to simplify reuse.
          @param pPsData Pointer to buffer containing entire payload of 
                 Photoshop formated APP13 Jpeg segment.
          @param sizePsData Size in bytes of pPsData.
          @param record Output value that is set to the start of the Iptc
                 data block within pPsData (may not be null).
          @param sizeHdr Output value that is set to the size of the header
                 within the Iptc data block pointed to by record (may not
                 be null).
          @param sizeIptc Output value that is set to the size of the actual
                 Iptc data within the Iptc data block pointed to by record
                 (may not be null).
          @return 0 if successful;<BR>
                  3 if no Iptc data was found in pPsData;<BR>
                  -2 if the pPsData buffer does not contain valid data;<BR>
         */
        int locateIptcData(const byte *pPsData, 
                           long sizePsData,
                           const byte **record, 
                           uint16_t *const sizeHdr,
                           uint16_t *const sizeIptc) const;
        /*!
          @brief Write to the associated file stream with the provided data.
          @param initData Data to be written to the associated file
          @param dataSize Size in bytes of data to be written
          @return 0 if successful;<BR>
                  4 if the output file can not be written to;<BR>
         */
        int initFile(const byte initData[], size_t dataSize);
        /*!
          @brief Provides the main implementation of writeMetadata by 
                writing all buffered metadata to associated file. 
          @param os Output stream to write to (e.g., a temporary file).
          @return 0 if successful;<br>
                  1 if reading from associated file failed;<BR>
                  2 if the file does not contain a valid image;<BR>
                  4 if the temporary output file can not be written to;<BR>
         */
        int doWriteMetadata(FILE* ofp) const;

        // NOT Implemented
        //! Default constructor.
        JpegBase();
        //! Copy constructor
        JpegBase(const JpegBase& rhs);
        //! Assignment operator
        JpegBase& operator=(const JpegBase& rhs);
    }; // class JpegBase

    /*! 
      @brief Helper class to access JPEG images
     */
    class JpegImage : public JpegBase {
        friend Image::AutoPtr newJpegInstance(const std::string& path, FILE* fp);
        friend bool isJpegType(FILE* ifp, bool advance);
    public:
        //! @name Creators
        //@{
        /*! 
          @brief Constructor that can either open an existing Jpeg image or create
                 a new image from scratch. If a new image is to be created, any
                 existing file is overwritten. Since the constructor can not return
                 a result, callers should check the %good method after object
                 construction to determine success or failure.
          @param path Full path to image file.
          @param create Specifies if an existing file should be opened (false)
                 or if a new file should be created (true).
         */
        JpegImage(const std::string& path, bool create);
        //! Destructor
        ~JpegImage() {}
        //@}
    protected:
        //! @name Accessors
        //@{
        /*!
          @brief Writes a Jpeg header (aka signature) to the file stream.
          @param ofp File stream that the header is written to.
          @return 0 if successful;<BR>
                 4 if the output file can not be written to;<BR>
         */
        int writeHeader(FILE* ofp) const;
        /*!
          @brief Determine if the content of the file stream is a Jpeg image.
                 See base class for more details.
          @param ifp Input file stream.
          @param advance Flag indicating whether the read position in the stream
                         should be advanced by the number of characters read to
                         analyse the stream (true) or left at its original
                         position (false). This applies only if the type matches.
          @return  true  if the file stream data matches a Jpeg image;<BR>
                   false if the stream data does not match;<BR>
         */
        bool isThisType(FILE* ifp, bool advance) const;
        //@}
    private:
        // Constant data
        static const byte soi_;          // SOI marker
        static const byte blank_[];      // Minimal Jpeg image

        //! @name Creators
        //@{
        /*! 
          @brief Constructor to be used when a Jpeg file has already
                 been opened. Meant for internal factory use.
          @param path Full path to opened image file.
          @param fp File pointer to open file.
         */
        JpegImage(const std::string& path, FILE* fp) : JpegBase(path, fp) {}
        //@}

        // NOT Implemented
        //! Default constructor
        JpegImage();
        //! Copy constructor
        JpegImage(const JpegImage& rhs);
        //! Assignment operator
        JpegImage& operator=(const JpegImage& rhs);
    }; // class JpegImage

    //! Helper class to access %Exiv2 files
    class ExvImage : public JpegBase {
        friend Image::AutoPtr newExvInstance(const std::string& path, FILE* fp);
        friend bool isExvType(FILE* ifp, bool advance);
    public:
        //! @name Creators
        //@{
        /*! 
          @brief Constructor that can either open an existing Exv image or create
                 a new image from scratch. If a new image is to be created, any
                 existing file is overwritten. Since the constructor can not return
                 a result, callers should check the %good method after object
                 construction to determine success or failure.
          @param path Full path to image file.
          @param create Specifies if an existing file should be opened (false)
                 or if a new file should be created (true).
         */
        ExvImage(const std::string& path, bool create);
        //! Destructor
        ~ExvImage() {}
        //@}
    protected:
        //! @name Accessors
        //@{
        /*!
          @brief Writes an Exv header (aka signature) to the file stream.
          @param ofp File stream that the header is written to.
          @return 0 if successful;<BR>
                  4 if the output file can not be written to;<BR>
         */
        int writeHeader(FILE* ofp) const;
        /*!
          @brief Determine if the content of the file stream is a Exv image.
                 See base class for more details.
          @param ifp Input file stream.
          @param advance Flag indicating whether the read position in the stream
                         should be advanced by the number of characters read to
                         analyse the stream (true) or left at its original
                         position (false). This applies only if the type matches.
          @return  true  if the file stream data matches a Exv image;<BR>
                   false if the stream data does not match;<BR>
         */
        virtual bool isThisType(FILE* ifp, bool advance) const;
        //@}
    private:
        // Constant data
        static const char exiv2Id_[];    // Exv identifier
        static const byte blank_[];      // Minimal exiv file

        //! @name Creators
        //@{
        /*! 
          @brief Constructor to be used when an Exv file has already
                 been opened. Meant for internal factory use.
          @param path Full path to opened image file.
          @param fp File pointer to open file.
         */
        ExvImage(const std::string& path, FILE* fp) : JpegBase(path, fp) {}
        //@}

        // NOT Implemented
        //! Default constructor
        ExvImage();
        //! Copy constructor
        ExvImage(const ExvImage& rhs);
        //! Assignment operator
        ExvImage& operator=(const ExvImage& rhs);
    }; // class ExvImage

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

// *********************************************************************
// free functions

    /*!
      @brief Test if a file exists.
  
      @param  path Name of file to verify.
      @param  ct   Flag to check if <i>path</i> is a regular file.
      @return true if <i>path</i> exists and, if <i>ct</i> is set,
      is a regular file, else false.
  
      @note The function calls <b>stat()</b> test for <i>path</i>
      and its type, see stat(2). <b>errno</b> is left unchanged 
      in case of an error.
     */
    bool fileExists(const std::string& path, bool ct =false);

}                                       // namespace Exiv2

#endif                                  // #ifndef IMAGE_HPP_
