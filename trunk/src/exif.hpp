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
  @version $Name:  $ $Revision: 1.4 $
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
#include <iostream>
#include <sstream>

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
        //! Read the Tiff header from a data buffer. Returns 0 if successful.
        int read(const char* buf);
        /*!
          @brief Write the Tiff header into buf as a data string, return number 
                 of bytes copied.
         */
        long copy(char* buf) const;
        //! Return the lengths of the Tiff header in bytes.
        long size() const { return 8; }
        //! @name Accessors
        //@{
        //! Return the byte order (little or big endian). 
        ByteOrder byteOrder() const { return byteOrder_; }
        //! Return the tag value.
        uint16 tag() const { return tag_; }
        /*!
          @brief Return the offset to IFD0 from the start of the Tiff header.
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
      @brief Common interface for all values. The interface provides a uniform
             way to access values independent from their actual C++ type for 
             simple tasks like reading the values. For other tasks, like modifying
             values you need to downcast it to the actual subclass of Value so
             that you can access the subclass specific interface (e.g., assignment
             operator for a vector of unsigned longs). 
     */
    class Value {
    public:
        //! Constructor, taking a type id to initialize the base class with
        explicit Value(TypeId typeId) : typeId_(typeId) {}
        //! Virtual destructor.
        virtual ~Value() {}
        /*!
          @brief Read the value from a character buffer.

          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer 
          @param byteOrder Applicable byte order (little or big endian).
         */
        virtual void read(const char* buf, long len, ByteOrder byteOrder) =0;
        //! Set the value from a string buffer
        virtual void read(const std::string& buf) =0;
        /*!
          @brief Write value to a character data buffer.

          The user must ensure that the buffer has enough memory. Otherwise
          the call results in undefined behaviour.

          @param buf Data buffer to write to.
          @param byteOrder Applicable byte order (little or big endian).
          @return Number of characters written.
        */
        virtual long copy(char* buf, ByteOrder byteOrder) const =0;
        //! Return the size of the value in bytes
        virtual long size() const =0;
        /*!
          @brief Return a pointer to a copy of itself (deep copy).
                 The caller owns this copy and is responsible to delete it!
         */
        virtual Value* clone() const =0;
        //! Write the value to an output stream, return the stream
        virtual std::ostream& write(std::ostream& os) const =0;

        /*!
          @brief A (simple) factory to create a Value type.

          @param typeId Type of the value.
          @return Pointer to the newly created Value.
                  The caller owns this copy and is responsible to delete it!
         */
        static Value* create(TypeId typeId);

    protected:
        const TypeId typeId_;                   //!< Format type identifier

    };

    //! Output operator for Value types
    inline std::ostream& operator<<(std::ostream& os, const Value& value)
    {
        return value.write(os);
    }

    //! %Value for an undefined data type.
    class DataValue : public Value {
    public:
        //! Default constructor.
        DataValue(TypeId typeId =undefined) : Value(typeId) {}
        virtual void read(const char* buf, long len, ByteOrder byteOrder);
        virtual void read(const std::string& buf);
        virtual long copy(char* buf, ByteOrder byteOrder) const;
        virtual long size() const;
        virtual Value* clone() const;
        virtual std::ostream& write(std::ostream& os) const;

    private:
        std::string value_;

    };

    //! %Value for an Ascii string type.
    class AsciiValue : public Value {
    public:
        //! Default constructor.
        AsciiValue() : Value(asciiString) {}
        virtual void read(const char* buf, long len, ByteOrder byteOrder);
        virtual void read(const std::string& buf);
        virtual long copy(char* buf, ByteOrder byteOrder) const;
        virtual long size() const;
        virtual Value* clone() const;
        virtual std::ostream& write(std::ostream& os) const;

    private:
        std::string value_;

    };

    /*!
      @brief Template for a %Value for a basic Type. This is used for unsigned 
             and signed short, long and rational.
     */    
    template<typename T>
    class ValueType : public Value {
    public:
        //! Default constructor.
        ValueType() : Value(getType<T>()) {}
        virtual void read(const char* buf, long len, ByteOrder byteOrder);
        virtual void read(const std::string& buf);
        virtual long copy(char* buf, ByteOrder byteOrder) const;
        virtual long size() const;
        virtual Value* clone() const;
        virtual std::ostream& write(std::ostream& os) const;

    private:
        typedef std::vector<T> ValueList;
        ValueList value_;

    };

    /*!
      @brief Information related to one %Exif tag.
     */
    class Metadatum {
    public:
        Metadatum();                   //!< Constructor
        ~Metadatum();                  //!< Destructor 
        Metadatum(const Metadatum& rhs); //!< Copy constructor
        Metadatum& operator=(const Metadatum& rhs); //!< Assignment operator

        //! Return the name of the type
        const char* tagName() const { return ExifTags::tagName(tag_, ifdId_); }
        //! Return the name of the type
        const char* typeName() const { return ExifTags::typeName(TypeId(type_)); }
        //! Return the size in bytes of one element of this type
        long typeSize() const { return ExifTags::typeSize(TypeId(type_)); }
        //! Return the name of the IFD
        const char* ifdName() const { return ExifTags::ifdName(ifdId_); }
        //! Return the related image item (image or thumbnail)
        const char* ifdItem() const { return ExifTags::ifdItem(ifdId_); }
        //! Return the name of the section
        const char* sectionName() const 
            { return ExifTags::sectionName(tag_, ifdId_); }
        //! Return a unique key of the tag (ifdItem.sectionName.tagName)
        std::string key() const;
        /*!
          @brief Return a reference the the value. The behaviour is undefined
                 if the value has not been initialized.
                 Todo: should we make sure there is a value?
         */
        const Value& value() const { return *value_; }

    public:
        uint16 tag_;                   //!< Tag value
        uint16 type_;                  //!< Type of the data Todo: change to TypeId?
        uint32 count_;                 //!< Number of components
        uint32 offset_;                //!< Offset of the data from start of IFD
        long size_;                    //!< Size of the data in bytes

        IfdId ifdId_;                  //!< The IFD associated with this tag
        int   ifdIdx_;                 //!< Position in the IFD (-1: not set)

        Value* value_;                 //!< Pointer to the value

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
          @brief Copy the IFD to a data array, returns a reference to the
                 data buffer. The pointer to the next IFD will be adjusted to an
                 offset from the start of the Tiff header to the position
                 immediately following the converted IFD.

          @param buf    Pointer to the data buffer.
          @param byteOrder Applicable byte order (little or big endian).
          @param offset Target offset from the start of the Tiff header of the
                        data array. The IFD offsets will be adjusted as
                        necessary. If not given, then it is assumed that the IFD
                        will remain at its original position.
          @return       Returns the number of characters written.
         */
        long copy(char* buf, ByteOrder byteOrder, long offset =0) const;
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
        //! Write %Exif data to a data buffer, return number of bytes written
        long copy(char* buf) const;
        //! Returns the size of all %Exif data (Tiff header plus metadata)
        long size() const;
        //! Returns the byte order as specified in the Tiff header
        ByteOrder byteOrder() const { return tiffHeader_.byteOrder(); }

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
    //! Read an 8 byte unsigned rational value from the data buffer
    URational getURational(const char* buf, ByteOrder byteOrder);
    //! Read a 2 byte signed short value from the data buffer
    int16 getShort(const char* buf, ByteOrder byteOrder);
    //! Read a 4 byte signed long value from the data buffer
    int32 getLong(const char* buf, ByteOrder byteOrder);
    //! Read an 8 byte signed rational value from the data buffer
    Rational getRational(const char* buf, ByteOrder byteOrder);

    /*!
      @brief Read a value of type T from the data buffer.

      We need this template function for the ValueType template classes. 
      There are only specializations of this function available; no default
      implementation is provided.

      @param buf Pointer to the data buffer to read from.
      @param byteOrder Applicable byte order (little or big endian).
      @return A value of type T.
     */
    template<typename T> T getValue(const char* buf, ByteOrder byteOrder);
    // Specialization for a 2 byte unsigned short value.
    template<> inline uint16 getValue(const char* buf, ByteOrder byteOrder)
    {
        return getUShort(buf, byteOrder);
    }
    // Specialization for a 4 byte unsigned long value.
    template<> inline uint32 getValue(const char* buf, ByteOrder byteOrder)
    {
        return getULong(buf, byteOrder);
    }
    // Specialization for an 8 byte unsigned rational value.
    template<> inline URational getValue(const char* buf, ByteOrder byteOrder)
    {
        return getURational(buf, byteOrder);
    }
    // Specialization for a 2 byte signed short value.
    template<> inline int16 getValue(const char* buf, ByteOrder byteOrder)
    {
        return getShort(buf, byteOrder);
    }
    // Specialization for a 4 byte signed long value.
    template<> inline int32 getValue(const char* buf, ByteOrder byteOrder)
    {
        return getLong(buf, byteOrder);
    }
    // Specialization for an 8 byte signed rational value.
    template<> inline Rational getValue(const char* buf, ByteOrder byteOrder)
    {
        return getRational(buf, byteOrder);
    }

    /*!
      @brief Convert an unsigned short to data, write the data to the buffer, 
             return number of bytes written.
     */
    long us2Data(char* buf, uint16 s, ByteOrder byteOrder);
    /*!
      @brief Convert an unsigned long to data, write the data to the buffer,
             return number of bytes written.
     */
    long ul2Data(char* buf, uint32 l, ByteOrder byteOrder);
    /*!
      @brief Convert an unsigned rational to data, write the data to the buffer,
             return number of bytes written.
     */
    long ur2Data(char* buf, URational l, ByteOrder byteOrder);
    /*!
      @brief Convert a signed short to data, write the data to the buffer, 
             return number of bytes written.
     */
    long s2Data(char* buf, int16 s, ByteOrder byteOrder);
    /*!
      @brief Convert a signed long to data, write the data to the buffer,
             return number of bytes written.
     */
    long l2Data(char* buf, int32 l, ByteOrder byteOrder);
    /*!
      @brief Convert a signed rational to data, write the data to the buffer,
             return number of bytes written.
     */
    long r2Data(char* buf, Rational l, ByteOrder byteOrder);

    /*!
      @brief Convert a value of type T to data, write the data to the data buffer.

      We need this template function for the ValueType template classes. 
      There are only specializations of this function available; no default
      implementation is provided.

      @param buf Pointer to the data buffer to write to.
      @param t Value to be converted.
      @param byteOrder Applicable byte order (little or big endian).
      @return The number of bytes written to the buffer.
     */
    template<typename T> long toData(char* buf, T t, ByteOrder byteOrder);
    /*! 
      @brief Specialization to write an unsigned short to the data buffer.
             Return the number of bytes written.
     */
    template<> inline long toData(char* buf, uint16 t, ByteOrder byteOrder)
    {
        return us2Data(buf, t, byteOrder);
    }
    /*! 
      @brief Specialization to write an unsigned long to the data buffer.
             Return the number of bytes written.
     */
    template<> inline long toData(char* buf, uint32 t, ByteOrder byteOrder)
    {
        return ul2Data(buf, t, byteOrder);
    }
    /*!
      @brief Specialization to write an unsigned rational to the data buffer.
             Return the number of bytes written.
     */
    template<> inline long toData(char* buf, URational t, ByteOrder byteOrder)
    {
        return ur2Data(buf, t, byteOrder);
    }
    /*! 
      @brief Specialization to write a signed short to the data buffer.
             Return the number of bytes written.
     */
    template<> inline long toData(char* buf, int16 t, ByteOrder byteOrder)
    {
        return s2Data(buf, t, byteOrder);
    }
    /*! 
      @brief Specialization to write a signed long to the data buffer.
             Return the number of bytes written.
     */
    template<> inline long toData(char* buf, int32 t, ByteOrder byteOrder)
    {
        return l2Data(buf, t, byteOrder);
    }
    /*!
      @brief Specialization to write a signed rational to the data buffer.
             Return the number of bytes written.
     */
    template<> inline long toData(char* buf, Rational t, ByteOrder byteOrder)
    {
        return r2Data(buf, t, byteOrder);
    }

    //! Print len bytes from buf in hex and ASCII format to the given stream
    void hexdump(std::ostream& os, const char* buf, long len);

// *****************************************************************************
// template definitions

    template<typename T>
    void ValueType<T>::read(const char* buf, long len, ByteOrder byteOrder)
    {
        value_.clear();
        for (long i = 0; i < len; i += ExifTags::typeSize(typeId_)) {
            value_.push_back(getValue<T>(buf + i, byteOrder));
        }
    }

    template<typename T>
    void ValueType<T>::read(const std::string& buf)
    {
        std::istringstream is(buf);
        T tmp;
        value_.clear();
        while (is >> tmp) {
            value_.push_back(tmp);
        }
    }

    template<typename T>
    long ValueType<T>::copy(char* buf, ByteOrder byteOrder) const
    {
        long offset = 0;
        typename ValueList::const_iterator end = value_.end();
        for (typename ValueList::const_iterator i = value_.begin(); i != end; ++i) {
            offset += toData(buf + offset, *i, byteOrder);
        }
        return offset;
    }

    template<typename T>
    long ValueType<T>::size() const
    {
        return ExifTags::typeSize(typeId_) * value_.size();
    }

    template<typename T>
    Value* ValueType<T>::clone() const
    {
        return new ValueType(*this);
    }

    template<typename T>
    std::ostream& ValueType<T>::write(std::ostream& os) const
    {
        typename ValueList::const_iterator end = value_.end();
        typename ValueList::const_iterator i = value_.begin();
        while (i != end) {
            os << *i;
            if (++i != end) os << " ";
        }
        return os;
    }
   
}                                       // namespace Exif

#endif                                  // #ifndef _EXIF_HPP_
