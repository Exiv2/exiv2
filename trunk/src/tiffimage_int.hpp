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
  @file    tiffimage_int.hpp
  @brief   Internal class TiffParserWorker to parse TIFF data.
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    23-Apr-08, ahu: created
 */
#ifndef TIFFIMAGE_INT_HPP_
#define TIFFIMAGE_INT_HPP_

// *****************************************************************************
// included header files
#include "tifffwd_int.hpp"
#include "image.hpp"
#include "types.hpp"

// + standard includes

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    /*!
      @brief Contains internal objects which are not published and are not part
             of the <b>libexiv2</b> API.
     */
    namespace Internal {

// *****************************************************************************
// class definitions

    /*!
      @brief Abstract base class defining the interface of an image header.
             Used internally by classes for TIFF-based images.  Default
             implementation is for the regular TIFF header.
     */
    class TiffHeaderBase {
    public:
        //! @name Creators
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
          @return True if the TIFF header was read successfully. False if the
                 data buffer does not contain a valid TIFF header.
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
          @return Number of bytes written.
         */
        virtual uint32_t write(Blob& blob) const;
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
        TiffHeade2(ByteOrder byteOrder =littleEndian);
        //! Destructor
        ~TiffHeade2();
        //@}
    }; // class TiffHeade2

    /*!
      @brief TIFF component factory for standard TIFF components.
     */
    class TiffCreator {
    public:
        /*!
          @brief Create the TiffComponent for TIFF entry \em extendedTag and
                 \em group based on the embedded lookup table. If the pointer
                 that is returned is 0, then the TIFF entry should be ignored.
        */
        static std::auto_ptr<TiffComponent> create(uint32_t extendedTag,
                                                   uint16_t group);
        /*!
          @brief Get the path, i.e., a list of TiffStructure pointers, from
                 the root TIFF element to the TIFF entry \em extendedTag and
                 \em group.
        */
        static void getPath(TiffPath& tiffPath,
                            uint32_t  extendedTag,
                            uint16_t  group);

    private:
        static const TiffStructure tiffStructure_[]; //<! TIFF structure
    }; // class TiffCreator

    /*!
      @brief Stateless parser class for data in TIFF format. Images use this
             class to decode and encode TIFF-based data.
     */
    class TiffParserWorker {
    public:
        /*!
          @brief Decode TIFF metadata from a data buffer \em pData of length
                 \em size into the provided metadata containers.

          This is the entry point to access image data in TIFF format. The
          parser uses classes TiffHeade2 and the TiffComponent and TiffVisitor
          hierarchies.

          @param exifData  Exif metadata container.
          @param iptcData  IPTC metadata container.
          @param xmpData   XMP metadata container.
          @param pData     Pointer to the data buffer. Must point to data
                           in TIFF format; no checks are performed.
          @param size      Length of the data buffer.
          @param createFct Factory function to create new TIFF components.
          @param findDecoderFct Function to access special decoding info.
          @param pHeader   Optional pointer to a TIFF header. If not provided,
                           a standard TIFF header is used.

          @return Byte order in which the data is encoded, invalidByteOrder if
                  decoding failed.
        */
        static ByteOrder decode(
                  ExifData&          exifData,
                  IptcData&          iptcData,
                  XmpData&           xmpData,
            const byte*              pData,
                  uint32_t           size,
                  TiffCompFactoryFct createFct,
                  FindDecoderFct     findDecoderFct,
                  TiffHeaderBase*    pHeader =0);
        /*!
          @brief Encode TIFF metadata from the metadata containers into a
                 memory block \em blob.
        */
        static WriteMethod encode(
                  Blob&              blob,
            const byte*              pData,
                  uint32_t           size,
            const ExifData&          exifData,
            const IptcData&          iptcData,
            const XmpData&           xmpData,
                  TiffCompFactoryFct createFct,
                  FindEncoderFct     findEncoderFct,
                  TiffHeaderBase*    pHeader
        );

    private:
        /*!
          @brief Parse TIFF metadata from a data buffer \em pData of length
                 \em size into a TIFF composite structure.

          @param pData     Pointer to the data buffer. Must point to data
                           in TIFF format; no checks are performed.
          @param size      Length of the data buffer.
          @return          An auto pointer with the root element of the TIFF
                           composite structure. If \em pData is 0 or \em size
                           is 0, the return value is a 0 pointer.
         */
        static std::auto_ptr<TiffComponent>
        parse(const byte*              pData,
                    uint32_t           size,
                    TiffCompFactoryFct createFct,
                    TiffHeaderBase*    pHeader);

    }; // class TiffParserWorker

    /*!
      @brief Table of TIFF decoding and encoding functions and find functions.
             This class is separated from the metadata decoder and encoder
             visitors so that the parser can be parametrized with a different
             table if needed. This is used, eg., for CR2 format, which uses a
             different decoder table.
     */
    class TiffMapping {
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
        static DecoderFct findDecoder(const std::string& make,
                                            uint32_t     extendedTag,
                                            uint16_t     group);
        /*!
          @brief Find special encoder function for a key.

          If the returned pointer is 0, the tag should be encoded with the
          encoder function of the TIFF component, else the encoder function
          should be used.

          @param make Camera make
          @param extendedTag Extended tag
          @param group %Group

          @return Pointer to the encoder function
         */
        static EncoderFct findEncoder(
            const std::string& make,
                  uint32_t     extendedTag,
                  uint16_t     group
        );

    private:
        static const TiffMappingInfo tiffMappingInfo_[]; //<! TIFF mapping table

    }; // class TiffMapping

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef TIFFIMAGE_INT_HPP_
