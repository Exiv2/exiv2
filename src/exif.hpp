// ***************************************************************** -*- C++ -*-
/*
 * Copyright (c) 2004 Andreas Huggel. All rights reserved.
 * 
 * Todo: Insert license blabla here
 *
 */
/*!
  @file    exif.hpp
  @brief   Encoding and decoding of %Exif data
  @version $Name:  $ $Revision: 1.2 $
  @author  Andreas Huggel (ahu)
  @date    09-Jan-03, ahu: created
 */
#ifndef _EXIF_HPP_
#define _EXIF_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"

// + standard includes
#include <string>
#include <vector>
#include <iosfwd>

// *****************************************************************************
// namespace extensions
//! Provides classes and functions to encode and decode %Exif data.
namespace Exif {

// *****************************************************************************
// type definitions

// *****************************************************************************
// class definitions

    /*! 
      @brief Helper class to access JPEG images

      Todo: define and implement write support
     */
    class JpegImage {
        // Copying not allowed
        JpegImage(const JpegImage& rhs);
        // Assignment not allowed
        JpegImage& operator=(const JpegImage& rhs);

    public:
        //! Default constructor
        JpegImage();
        //! Default destructor
        ~JpegImage();

        /*!
          @brief Checks if the stream is the beginning of a JPEG image
          @param is Input stream to test
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
        /*!
          @brief Reads the %Exif data from the file path into the internal 
                 data buffer.
          @param path Path to the file
          @return 0 if successful<br>
                 -1 if the file cannot be opened or<br>
                  the return code of readExifData(std::istream& is)
                    if the call to this function fails
         */
        int readExifData(const std::string& path);
        /*!
          @brief Reads the %Exif data from the stream into the internal 
                 data buffer.
          @param is Input stream to read from
          @return 0 if successful<br>
                  1 if reading from the stream failed. Consult the stream state 
                    for more information.<br>
                  2 if the stream does not contain a JPEG image<br>
                  3 if the APP1 marker was not found<br>
                  4 if the APP1 field does not contain %Exif data
         */
        int readExifData(std::istream& is);
        //! @name Accessors
        //@{
        //! Returns the size of the %Exif data buffer
        long sizeExifData() const { return sizeExifData_; }
        //! Returns the offset of the %Exif data buffer from SOI
        long offsetExifData() const { return offsetExifData_; }
        //! Returns a read-only pointer to the %Exif data buffer 
        const char* exifData() const { return exifData_; }
        //@}

    private:
        static const uint16 soi_;               // SOI marker
        static const uint16 app1_;              // APP1 marker
        static const char exifId_[];            // Exif identifier

        long sizeExifData_;                     // Size of the Exif data buffer
        long offsetExifData_;                   // Offset from SOI
        char* exifData_;                        // Exif data buffer

    }; // class JpegImage

    //! Type to express the byte order (little or big endian)
    enum ByteOrder { littleEndian, bigEndian };

    //! Helper class modelling the Tiff header structure.
    class TiffHeader {
    public:
        //! Default constructor.
        TiffHeader();
        //! Reads the Tiff header from a data buffer. Returns 0 if successful.
        int read(const char* buf);
        //! Writes the Tiff header into buf as a data string.
        void data(char* buf) const;
        //! Returns the lengths of the Tiff header in bytes.
        long size() const { return 8; }
        //! @name Accessors
        //@{
        //! Returns the byte order (little or big endian). 
        ByteOrder byteOrder() const { return byteOrder_; }
        //! Returns the tag value.
        uint16 tag() const { return tag_; }
        /*!
          @brief Returns the offset to IFD0 from the start of the Tiff header.
                 The offset is 0x00000008 if IFD0 begins immediately after the 
                 Tiff header.
         */
        uint32 offset() const { return offset_; }
        //@}

    private:
        ByteOrder byteOrder_;
        uint16 tag_;
        uint32 offset_;
    }; // class TiffHeader

    /*!
      @brief Information related to one %Exif tag.

     */
    class Metadatum {
    public:
        Metadatum();                   //!< Constructor
        ~Metadatum();                  //!< Destructor 
        Metadatum(const Metadatum& rhs); //!< Copy constructor
        Metadatum& operator=(const Metadatum& rhs); //!< Assignment operator

        //! Returns the name of the type
        const char* tagName() const { return ExifTags::tagName(tag_, ifdId_); }
        //! Returns the name of the type
        const char* typeName() const { return ExifTags::typeName(type_); }
        //! Returns the size in bytes of one element of this type
        long typeSize() const { return ExifTags::typeSize(type_); }
        //! Returns the name of the IFD
        const char* ifdName() const { return ExifTags::ifdName(ifdId_); }
        //! Returns the related image item (image or thumbnail)
        const char* ifdItem() const { return ExifTags::ifdItem(ifdId_); }
        //! Returns the name of the section
        const char* sectionName() const 
            { return ExifTags::sectionName(tag_, ifdId_); }
        //! Returns a unique key of the tag (ifdItem.sectionName.tagName)
        std::string key() const;

    public:
        uint16 tag_;                   //!< Tag value
        uint16 type_;                  //!< Type of the data
        uint32 count_;                 //!< Number of components
        uint32 offset_;                //!< Offset of the data from start of IFD
        long size_;                    //!< Size of the data in bytes

        IfdId ifdId_;                  //!< The IFD associated with this tag
        int   ifdIdx_;                 //!< Position in the IFD (-1: not set)

        char* data_;                   //!< Pointer to the data

    }; // struct Metadatum

    //! Container type to hold all metadata
    typedef std::vector<Metadatum> Metadata;

    //! Unary predicate that matches a Metadatum with a given tag
    class FindMetadatumByTag {
    public:
        //! Constructor, initializes the object with the tag to look for
        FindMetadatumByTag(uint16 tag) : tag_(tag) {}
        /*!
          @brief Returns true if the tag of the argument metadatum is equal
                 to that of the object.
         */
        bool operator()(const Metadatum& metadatum) const
            { return tag_ == metadatum.tag_; } 

    private:
        uint16 tag_;
    };

    /*!
      @brief Models an IFD (Image File Directory)

      Todo:
      - make the data handling more intelligent
      - should we return the size and do away with size() ?
    */
    class Ifd {
    public:
        //! Constructor. Allows to set the IFD identifier.
        explicit Ifd(IfdId ifdId =IfdIdNotSet);
        /*!
          @brief Read a complete IFD and its data from a data buffer

          @param buf Pointer to the data to decode. The buffer must start with the 
                     IFD data (as opposed to readSubIfd).
          @param byteOrder Applicable byte order (little or big endian).
          @param offset (Optional) offset of the IFD from the start of the Tiff
                 header, if known. If not given, the offset will be guessed
                 using the assumption that the smallest offset of all IFD
                 directory entries points to a data buffer immediately follwing
                 the IFD.

          @return 0 if successful
         */
        int read(const char* buf, ByteOrder byteOrder, long offset =0);
        /*!
          @brief Read a sub-IFD from the location pointed to by the directory entry 
                 with the given tag.

          @param dest References the destination IFD.
          @param buf The data buffer to read from. The buffer must contain all Exif 
                     data starting from the Tiff header (as opposed to read).
          @param byteOrder Applicable byte order (little or big endian).
          @param tag Tag to look for.

          @return 0 if successful
        */
        int readSubIfd(
            Ifd& dest, const char* buf, ByteOrder byteOrder, uint16 tag
        ) const;
        /*!
          @brief Convert the IFD to a data array, returns a reference to the
                 data buffer. The pointer to the next IFD will be adjusted to an
                 offset from the start of the Tiff header to the position
                 immediately following the converted IFD.

          @param buf    Pointer to the data buffer.
          @param byteOrder Applicable byte order (little or big endian).
          @param offset Target offset from the start of the Tiff header of the
                        data array. The IFD offsets will be adjusted as
                        necessary. If not given, then it is assumed that the IFD
                        will remain at its original position.
          @return       Returns buf
         */
        char* data(char* buf, ByteOrder byteOrder, long offset =0) const;
        /*!
          @brief Print the IFD in human readable format to the given stream;
                 begin each line with prefix.
         */
        void print(std::ostream& os, const std::string& prefix ="") const;
        //! @name Accessors
        //@{
        //! Offset of the IFD from SOI
        long offset() const { return offset_; }
        //! Get the IFD entries
        const Metadata& entries() const { return entries_; }
        //! Get the offset to the next IFD from the start of the Tiff header
        long next() const { return next_; }
        //! Get the size of this IFD in bytes (IFD only, without data)
        long size() const { return size_; }
        //@}

    private:
        IfdId ifdId_;                    // IFD Id
        long offset_;                    // offset of the IFD from the start of 
                                         // Tiff header
        Metadata entries_;               // IFD metadata entries
        long next_;                      // offset of next IFD from the start of 
                                         // the Tiff header
        long size_;                      // size of the IFD in bytes
    }; // class Ifd

    /*!
      @brief A container for %Exif data

      Contains the %Exif data of a JPEG image
      - read and write access to all tags and data
      - iterators to access the %Exif data
      - decoding and encoding through the stream interface
      - human readable 
      - XML input and output

      Todo:
      - A constructor which creates a minimal valid set of %Exif data
      - Support to add, delete, edit, read data (maybe also in Metadata)
    */
    class ExifData {
    public:
        /*!
          @brief Read the %Exif data from file path.
          @param path Path to the file
          @return 0 if successful<br>
                  the return code of readExifData(std::istream& is)
                    if the call to this function fails<br>
                  the return code of read(const char* buf, long len)
                    if the call to this function fails<br>
         */
        int read(const std::string& path);
        /*!
          @brief Read the %Exif data from a character buffer. Assumes that 
                 the original abs offset of the Exif data is set.
          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer 
          @return 0 if successful
         */
        int read(const char* buf, long len);
        //! Write %Exif data to a data buffer
        void data(char* buf) const;
        //! Returns the size of all %Exif data (Tiff Header plus metadata)
        long size() const;

        //! Add all entries of src to the Exif metadata
        void add(const Metadata& src);
        //! Add Metadatum src to the Exif metadata
        void add(const Metadatum& src);

        //! Metadata iterator type (const)
        typedef Metadata::const_iterator const_iterator;
        //! Begin of the metadata
        const_iterator begin() const { return metadata_.begin(); }
        //! End of the metadata
        const_iterator end() const { return metadata_.end(); }

    private:
        long offset_;                   // Original abs offset of the Exif data
        TiffHeader tiffHeader_;
        Metadata metadata_;
    }; // class ExifData
   
// *****************************************************************************
// free functions

    //! Read a 2 byte unsigned short value from the data buffer
    uint16 getUShort(const char* buf, ByteOrder byteOrder);

    //! Read a 4 byte unsigned long value from the data buffer
    uint32 getULong(const char* buf, ByteOrder byteOrder);

    //! Convert len bytes from the data buffer into a string
    std::string getString(const char* buf, long len);

    //! Write an unsigned short to the data buffer
    char* us2Data(char* buf, uint16 s, ByteOrder byteOrder);

    //! Convert an unsigned long to data, write the data to the buffer
    char* ul2Data(char* buf, uint32 l, ByteOrder byteOrder);

    //! Print len bytes from buf in hex and ASCII format to the given stream
    void hexdump(std::ostream& os, const char* buf, long len);

}                                       // namespace Exif

#endif                                  // #ifndef _EXIF_HPP_
