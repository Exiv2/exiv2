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
  @file    jpgimage.hpp
  @brief   Class JpegImage to access JPEG images
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Brad Schick (brad) 
           <a href="mailto:brad@robotbattle.com">brad@robotbattle.com</a>
  @date    15-Jan-05, brad: split out from image.cpp
 */
#ifndef JPGIMAGE_HPP_
#define JPGIMAGE_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "image.hpp"
#include "basicio.hpp"
#include "exif.hpp"
#include "iptc.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {


// *****************************************************************************
// class definitions

    /*! 
      @brief Abstract helper base class to access JPEG images.
     */
    class JpegBase : public Image {
    public:
        //! @name Creators
        //@{
        //! Virtual destructor.
        virtual ~JpegBase() {}
        //@}
        //! @name Manipulators
        //@{
        /*!
          @brief Read all metadata from the image. Before this method
              is called, the various metadata types (Iptc, Exif) will be empty.
              
          This method returns success even when no metadata is found in
          the image. Callers must therefore check the size of indivdual
          metadata types before accessing the data.
          
          @return 0 if successful;<BR>
                  1 if reading from the file failed 
                    (could be caused by invalid image);<BR>
                  2 if the file does not contain a valid image;<BR>
         */
        int readMetadata();
        /*!
          @brief Write metadata back to the image. 

          All existing metadata sections in the image are either created,
          replaced, or erased. If values for a given metadata type have been
          assigned, a section for that metadata type will either be created or
          replaced. If no values have been assigned to a given metadata type,
          any exists section for that metadata type will be removed from the
          image.
          
          @return 0 if successful;<br>
                  1 if reading from the file failed;<BR>
                  2 if the file does not contain a valid image;<BR>
                  4 if the temporary output file can not be written to;<BR>
                  -1 if the newly created file could not be reopened;<BR>
                  -3 if the temporary output file can not be opened;<BR>
                  -4 if renaming the temporary file fails;<br>
         */
        int writeMetadata();
        /*!
          @brief Assign new exif data. The new exif data is not written
             to the image until the writeMetadata() method is called.
          @param exifData An ExifData instance holding exif data to be copied

          @throw Error ("Exif data too large") if the exif data is larger than
                 65535 bytes (the maximum size of JPEG APP segments)
         */
        void setExifData(const ExifData& exifData);
        void clearExifData();
        void setIptcData(const IptcData& iptcData);
        void clearIptcData();
        void setComment(const std::string& comment);
        void clearComment();
        void setMetadata(const Image& image);
        void clearMetadata();
        //@}

        //! @name Accessors
        //@{
        bool good() const;
        const ExifData& exifData() const { return exifData_; }
        ExifData& exifData() { return exifData_; }
        const IptcData& iptcData() const { return iptcData_; }
        IptcData& iptcData() { return iptcData_; }
        std::string comment() const { return comment_; }
        BasicIo& io() const { return *io_; }
        //@}        
    protected:
        //! @name Creators
        //@{
        /*! 
          @brief Constructor that can either open an existing image or create
              a new image from scratch. If a new image is to be created, any
              existing data is overwritten.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method.  Use the Image::io()
              method to get a temporary reference.
          @param create Specifies if an existing image should be read (false)
              or if a new image should be created (true).
          @param initData Data to initialize newly created images. Only used
              when \em create is true. Should contain data for the smallest
              valid image of the calling subclass.
          @param dataSize Size of initData in bytes.
         */
        JpegBase(BasicIo::AutoPtr io, bool create,
                 const byte initData[], long dataSize);
        //@}
        //! @name Manipulators
        //@{
        /*!
          @brief Writes the image header (aka signature) to the BasicIo instance.
          @param oIo BasicIo instance that the header is written to.
          @return 0 if successful;<BR>
                 4 if the output file can not be written to;<BR>
         */
        virtual int writeHeader(BasicIo& oIo) const =0;
        //@}
        //! @name Accessors
        //@{
        /*!
          @brief Determine if the content of the BasicIo instance is of the
              type supported by this class.

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
                   false if the data does not match;<BR>
         */
        virtual bool isThisType(BasicIo& iIo, bool advance) const =0;
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
        BasicIo::AutoPtr io_;                   //!< Image data io pointer
        ExifData exifData_;                     //!< Exif data container
        IptcData iptcData_;                     //!< Iptc data container
        std::string comment_;                   //!< JPEG comment

        // METHODS
        /*!
          @brief Advances associated io instance to one byte past the next
              Jpeg marker and returns the marker. This method should be called
              when the BasicIo instance is positioned one byte past the end of a
              Jpeg segment.
          @return the next Jpeg segment marker if successful;<BR>
                 -1 if a maker was not found before EOF;<BR>
         */
        int advanceToMarker() const;
        /*!
          @brief Locates Photoshop formated Iptc data in a memory buffer.
              Operates on raw data to simplify reuse.
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
          @brief Initialize the image with the provided data.
          @param initData Data to be written to the associated BasicIo
          @param dataSize Size in bytes of data to be written
          @return 0 if successful;<BR>
                  4 if the output file can not be written to;<BR>
         */
        int initImage(const byte initData[], long dataSize);
        /*!
          @brief Provides the main implementation of writeMetadata() by 
                writing all buffered metadata to the provided BasicIo. 
          @param oIo BasicIo instance to write to (a temporary location).
          @return 0 if successful;<br>
                  1 if reading from input file failed;<BR>
                  2 if the input file does not contain a valid image;<BR>
                  4 if the output file can not be written to;<BR>
         */
        int doWriteMetadata(BasicIo& oIo);

        // NOT Implemented
        //! Default constructor.
        JpegBase();
        //! Copy constructor
        JpegBase(const JpegBase& rhs);
        //! Assignment operator
        JpegBase& operator=(const JpegBase& rhs);
    }; // class JpegBase

    /*! 
      @brief Class to access JPEG images
     */
    class JpegImage : public JpegBase {
        friend bool isJpegType(BasicIo& iIo, bool advance);
    public:
        //! @name Creators
        //@{
        /*! 
          @brief Constructor that can either open an existing Jpeg image or create
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
        JpegImage(BasicIo::AutoPtr io, bool create);
        //! Destructor
        ~JpegImage() {}
        //@}
        
        //! @cond IGNORE
        // Public only so that we can create a static instance
        struct JpegRegister{
            JpegRegister();
        };
        //! @endcond
    protected:
        //! @name Accessors
        //@{
        /*!
          @brief Determine if the content of the BasicIo instance is a Jpeg image.
              See base class for more details.
          @param iIo BasicIo instance to read from.
          @param advance Flag indicating whether the position of the io
              should be advanced by the number of characters read to
              analyse the data (true) or left at its original
              position (false). This applies only if the type matches.
          @return  true  if the data matches a Jpeg image;<BR>
                   false if the data does not match;<BR>
         */
        bool isThisType(BasicIo& iIo, bool advance) const;
        //@}
        //! @name Manipulators
        //@{
        /*!
          @brief Writes a Jpeg header (aka signature) to the BasicIo instance.
          @param oIo BasicIo instance that the header is written to.
          @return 0 if successful;<BR>
                 2 if the input image is invalid or can not be read;<BR>
                 4 if the temporary image can not be written to;<BR>
                -3 other temporary errors;<BR>
         */
        int writeHeader(BasicIo& oIo) const;
        //@}
    private:
        // Constant data
        static const byte soi_;          // SOI marker
        static const byte blank_[];      // Minimal Jpeg image

        // NOT Implemented
        //! Default constructor
        JpegImage();
        //! Copy constructor
        JpegImage(const JpegImage& rhs);
        //! Assignment operator
        JpegImage& operator=(const JpegImage& rhs);
    }; // class JpegImage

    static JpegImage::JpegRegister jpegReg;

    //! Helper class to access %Exiv2 files
    class ExvImage : public JpegBase {
        friend bool isExvType(BasicIo& iIo, bool advance);
    public:
        //! @name Creators
        //@{
        /*! 
          @brief Constructor that can either open an existing Exv image or create
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
        ExvImage(BasicIo::AutoPtr io, bool create);
        //! Destructor
        ~ExvImage() {}
        //@}
        
        //! @cond IGNORE
        // Public only so that we can create a static instance
        struct ExvRegister{
            ExvRegister();
        };
        //! @endcond
    protected:
        //! @name Accessors
        //@{
        /*!
          @brief Determine if the content of the BasicIo instance is an Exv
              image. See base class for more details.
          @param iIo BasicIo instance to read from.
          @param advance Flag indicating whether the position of the io
              should be advanced by the number of characters read to
              analyse the data (true) or left at its original
              position (false). This applies only if the type matches.
          @return  true  if the data matches a Jpeg image;<BR>
                   false if the data does not match;<BR>
         */
        virtual bool isThisType(BasicIo& iIo, bool advance) const;
        //@}
        //! @name Manipulators
        //@{
        /*!
          @brief Writes an Exv header (aka signature) to the BasicIo instance.
          @param oIo BasicIo instance that the header is written to.
          @return 0 if successful;<BR>
                  4 if the output file can not be written to;<BR>
         */
        int writeHeader(BasicIo& oIo) const;
        //@}
    private:
        // Constant data
        static const char exiv2Id_[];    // Exv identifier
        static const byte blank_[];      // Minimal exiv file

        // NOT Implemented
        //! Default constructor
        ExvImage();
        //! Copy constructor
        ExvImage(const ExvImage& rhs);
        //! Assignment operator
        ExvImage& operator=(const ExvImage& rhs);
    }; // class ExvImage

    static ExvImage::ExvRegister exvReg;
}                                       // namespace Exiv2


#endif                                  // #ifndef JPGIMAGE_HPP_
