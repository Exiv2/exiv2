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
  @file    exif.hpp
  @brief   Encoding and decoding of %Exif data
  @version $Name:  $ $Revision: 1.14 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    09-Jan-04, ahu: created
 */
#ifndef EXIF_HPP_
#define EXIF_HPP_

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
// class declarations
    class ExifData;
    class Entry;

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

    //! Helper class modelling the TIFF header structure.
    class TiffHeader {
    public:
        /*!
          @brief Default constructor. Optionally sets the byte order 
                 (default: little endian).
         */
        explicit TiffHeader(ByteOrder byteOrder =littleEndian);
        //! Read the TIFF header from a data buffer. Returns 0 if successful.
        int read(const char* buf);
        /*!
          @brief Write the TIFF header into buf as a data string, return number 
                 of bytes copied.
         */
        long copy(char* buf) const;
        //! Return the lengths of the TIFF header in bytes.
        long size() const { return 8; }
        //! @name Accessors
        //@{
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

    /*!
      @brief Common interface for all values. The interface provides a uniform
             way to access values independent from their actual C++ type for
             simple tasks like reading the values from a string or data buffer.
             For other tasks, like modifying values you may need to downcast it
             to the actual subclass of %Value so that you can access the
             subclass specific interface.
     */
    class Value {
    public:
        //! Constructor, taking a type id to initialize the base class with
        explicit Value(TypeId typeId) : type_(typeId) {}
        //! Virtual destructor.
        virtual ~Value() {}
        /*!
          @brief Read the value from a character buffer.

          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer 
          @param byteOrder Applicable byte order (little or big endian).
         */
        virtual void read(const char* buf, long len, ByteOrder byteOrder) =0;
        /*! 
          @brief Set the value from a string buffer. The format of the string
                 corresponds to that of the write() method, i.e., a string
                 obtained through the write() method can be read by this 
                 function.

          @param buf The string to read from.
         */
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
        //! Return the number of components of the value
        virtual long count() const =0;
        //! Return the size of the value in bytes
        virtual long size() const =0;
        /*!
          @brief Return a pointer to a copy of itself (deep copy).
                 The caller owns this copy and is responsible to delete it!
         */
        virtual Value* clone() const =0;
        /*! 
          @brief Write the value to an output stream. You do not usually have
                 to use this function; it is used for the implementation of 
                 the output operator for %Value, 
                 operator<<(std::ostream &os, const Value &value).
        */
        virtual std::ostream& write(std::ostream& os) const =0;
        /*!
          @brief Convert the n-th component of the value to a long. The
                 behaviour of this method may be undefined if there is no
                 n-th component.

          @return The converted value. 
         */
        virtual long toLong(long n =0) const =0;
        //! Return the value as a string
        std::string toString() const;

        //! Return the type identifier (Exif data format type).
        TypeId typeId() const { return TypeId(type_); }

        /*!
          @brief A (simple) factory to create a Value type.

          The following Value subclasses are created depending on typeId:<BR><BR>
          <TABLE>
          <TR><TD><B>typeId</B></TD><TD><B>%Value subclass</B></TD></TR>
          <TR><TD>invalid</TD><TD>%DataValue(invalid)</TD></TR>
          <TR><TD>unsignedByte</TD><TD>%DataValue(unsignedByte)</TD></TR>
          <TR><TD>asciiString</TD><TD>%AsciiValue</TD></TR>
          <TR><TD>unsignedShort</TD><TD>%ValueType &lt; uint16 &gt;</TD></TR>
          <TR><TD>unsignedLong</TD><TD>%ValueType &lt; uint32 &gt;</TD></TR>
          <TR><TD>unsignedRational</TD><TD>%ValueType &lt; URational &gt;</TD></TR>
          <TR><TD>invalid6</TD><TD>%DataValue(invalid6)</TD></TR>
          <TR><TD>undefined</TD><TD>%DataValue</TD></TR>
          <TR><TD>signedShort</TD><TD>%ValueType &lt; int16 &gt;</TD></TR>
          <TR><TD>signedLong</TD><TD>%ValueType &lt; int32 &gt;</TD></TR>
          <TR><TD>signedRational</TD><TD>%ValueType &lt; Rational &gt;</TD></TR>
          <TR><TD><EM>default:</EM></TD><TD>%DataValue(typeId)</TD></TR>
          </TABLE>

          @param typeId Type of the value.
          @return Pointer to the newly created Value.
                  The caller owns this copy and is responsible to delete it!
         */
        static Value* create(TypeId typeId);

    private:
        const uint16 type_;                     //!< Type of the data

    }; // class Value

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
        /*!
          @brief Read the value from a character buffer. The byte order is
                 required by the interface but not used by this method.

          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer 
          @param byteOrder Byte order. Not used.
         */
        virtual void read(const char* buf, long len, ByteOrder byteOrder);
        //! Set the data from a string of integer values (e.g., "0 1 2 3")
        virtual void read(const std::string& buf);
        /*!
          @brief Write value to a character data buffer. The byte order is
                 required by the interface but not used by this method.

          The user must ensure that the buffer has enough memory. Otherwise
          the call results in undefined behaviour.

          @param buf Data buffer to write to.
          @param byteOrder Byte order. Not used.
          @return Number of characters written.
        */
        virtual long copy(char* buf, ByteOrder byteOrder) const;
        virtual long count() const { return size(); }
        virtual long size() const;
        virtual Value* clone() const;
        virtual std::ostream& write(std::ostream& os) const;
        virtual long toLong(long n =0) const { return value_[n]; }

    private:
        std::string value_;

    }; // class DataValue

    //! %Value for an Ascii string type.
    class AsciiValue : public Value {
    public:
        //! Default constructor.
        AsciiValue() : Value(asciiString) {}
        /*!
          @brief Read the value from a character buffer. The byte order is
                 required by the interface but not used by this method.

          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer 
          @param byteOrder Byte order. Not used.
         */
        virtual void read(const char* buf, long len, ByteOrder byteOrder);
        /*!
          @brief Set the value to that of the string buf. A terminating
                 '\\0' character is appended to the value if buf doesn't 
                 end with '\\0'.
         */
        virtual void read(const std::string& buf);
        /*!
          @brief Write value to a character data buffer. The byte order is
                 required by the interface but not used by this method.

          The user must ensure that the buffer has enough memory. Otherwise
          the call results in undefined behaviour.

          @param buf Data buffer to write to.
          @param byteOrder Byte order. Not used.
          @return Number of characters written.
        */
        virtual long copy(char* buf, ByteOrder byteOrder) const;
        virtual long count() const { return size(); }
        virtual long size() const;
        virtual Value* clone() const;
        virtual std::ostream& write(std::ostream& os) const;
        virtual long toLong(long n =0) const { return value_[n]; }

    private:
        std::string value_;

    }; // class AsciiValue

    /*!
      @brief Template for a %Value of a basic type. This is used for unsigned 
             and signed short, long and rationals.
     */    
    template<typename T>
    class ValueType : public Value {
    public:
        //! Default constructor.
        ValueType() : Value(getType<T>()) {}
        virtual void read(const char* buf, long len, ByteOrder byteOrder);
        /*!
          @brief Set the data from a string of values of type T (e.g., 
                 "0 1 2 3" or "1/2 1/3 1/4" depending on what T is). 
                 Generally, the accepted input format is the same as that 
                 produced by the write() method.
         */
        virtual void read(const std::string& buf);
        virtual long copy(char* buf, ByteOrder byteOrder) const;
        virtual long count() const { return value_.size(); }
        virtual long size() const;
        virtual Value* clone() const;
        virtual std::ostream& write(std::ostream& os) const;
        virtual long toLong(long n =0) const;

        //! Container for values 
        typedef std::vector<T> ValueList;
        //! Iterator type defined for convenience.
        typedef typename std::vector<T>::iterator iterator;
        //! Const iterator type defined for convenience.
        typedef typename std::vector<T>::const_iterator const_iterator;

        /*!
          @brief The container for all values. In your application, if you know 
                 what subclass of Value you're dealing with (and possibly the T)
                 then you can access this STL container through the usual 
                 standard library functions.
         */
        ValueList value_;

    }; // class ValueType

    //! Unsigned short value type
    typedef ValueType<uint16> UShortValue;
    //! Unsigned long value type
    typedef ValueType<uint32> ULongValue;
    //! Unsigned rational value type
    typedef ValueType<URational> URationalValue;
    //! Signed short value type
    typedef ValueType<int16> ShortValue;
    //! Signed long value type
    typedef ValueType<int32> LongValue;
    //! Signed rational value type
    typedef ValueType<Rational> RationalValue;

    /*!
      @brief Information related to one %Exif tag.
     */
    class Metadatum {
    public:
        /*!
          @brief Constructor to build a metadatum from an IFD entry.
         */
        Metadatum(const Entry& e, ByteOrder byteOrder);
        /*!
          @brief Constructor for new tags created by an application, which only
                 needs to provide a key / value pair. %Metadatum copies (clones)
                 the value if one is provided. Alternatively, a program can
                 create an 'empty' metadatum with only a key and set the value
                 later, using setValue().

          @param key The key of the metadatum.
          @param value Pointer to a metadatum value.
          @throw Error ("Invalid key") if the key cannot be parsed and converted
                 to a tag number and an IFD id or the section name does not match.
         */
        explicit Metadatum(const std::string& key, Value* value =0);
        //! Destructor
        ~Metadatum();
        //! Copy constructor
        Metadatum(const Metadatum& rhs);
        //! Assignment operator
        Metadatum& operator=(const Metadatum& rhs);
        /*!
          @brief Set the value. This method copies (clones) the value.
         */
        void setValue(const Value* value);
        /*!
          @brief Set the value to the string buf. 
                 Uses Value::read(const std::string& buf). If the metadatum does
                 not have a value yet, then an AsciiValue is created.
         */
        void setValue(const std::string& buf);
        /*!
          @brief Return a pointer to a copy (clone) of the value. The caller
                 is responsible to delete this copy when it's no longer needed.

          This method is provided for users who need full control over the 
          value. A caller may, e.g., downcast the pointer to the appropriate
          subclass of Value to make use of the interface of the subclass to set
          or modify its contents.
          
          @return A pointer to a copy (clone) of the value, 0 if the value is 
                  not set.
         */
        Value* getValue() const { return value_ == 0 ? 0 : value_->clone(); }
        //! Return the name of the tag
        const char* tagName() const { return ExifTags::tagName(tag_, ifdId_); }
        //! Return the name of the type
        const char* typeName() const { return ExifTags::typeName(typeId()); }
        //! Return the size in bytes of one component of this type
        long typeSize() const { return ExifTags::typeSize(typeId()); }
        //! Return the name of the IFD
        const char* ifdName() const { return ExifTags::ifdName(ifdId_); }
        //! Return the related image item (image or thumbnail)
        const char* ifdItem() const { return ExifTags::ifdItem(ifdId_); }
        //! Return the name of the section
        const char* sectionName() const 
            { return ExifTags::sectionName(tag_, ifdId_); }
        //! @name Accessors
        //@{
        //! Return the tag
        uint16 tag() const { return tag_; }
        //! Return the type id.
        TypeId typeId() const { return value_ == 0 ? invalid : value_->typeId(); }
        //! Return the number of components in the value
        long count() const { return value_ == 0 ? 0 : value_->count(); }
        //! Return the size of the value in bytes
        long size() const { return value_ == 0 ? 0 : value_->size(); }
        /*!
          @brief Return the n-th component of the value. The return value is 
                 -1 if the value of the Metadatum is not set and the behaviour
                 of the method is undefined if there is no n-th component.
         */
        long toLong(long n =0) const 
            { return value_ == 0 ? -1 : value_->toLong(n); }
        //! Return the value as a string.
        std::string toString() const 
            { return value_ == 0 ? "" : value_->toString(); }
        /*!
          @brief Write value to a character data buffer and return the number
                 of characters (bytes) written.

          The user must ensure that the buffer has enough memory. Otherwise
          the call results in undefined behaviour.

          @param buf Data buffer to write to.
          @param byteOrder Applicable byte order (little or big endian).
          @return Number of characters written.
        */
        long copy(char* buf, ByteOrder byteOrder) const 
            { return value_ == 0 ? 0 : value_->copy(buf, byteOrder); }
        //! Return the IFD id
        IfdId ifdId() const { return ifdId_; }
        //! Return the position in the IFD (-1: not set)
        int ifdIdx() const { return ifdIdx_; }
        /*!
          @brief Return a constant reference to the value. 

          This method is provided mostly for convenient and versatile output of
          the value which can (to some extent) be formatted through standard
          stream manipulators.  Do not attempt to write to the value through
          this reference. 

          <b>Example:</b> <br>
          @code
          ExifData::const_iterator i = exifData.findKey(key);
          if (i != exifData.end()) {
              std::cout << i->key() << " " << std::hex << i->value() << "\n";
          }
          @endcode

          @return A constant reference to the value.
          @throw Error ("Value not set") if the value is not set.
         */
        const Value& value() const 
            { if (value_) return *value_; throw Error("Value not set"); }
        /*!
          @brief Return a unique key for the tag. The key is of the form
                 'ifdItem.sectionName.tagName'.
         */
        std::string key() const { return key_; }
        //@}

    private:
        uint16 tag_;                   //!< Tag value
        IfdId ifdId_;                  //!< The IFD associated with this tag
        int   ifdIdx_;                 //!< Position in the IFD (-1: not set)
        Value* value_;                 //!< Pointer to the value
        std::string key_;              //!< Unique key

    }; // class Metadatum

    //! Container type to hold all metadata
    typedef std::vector<Metadatum> Metadata;

    //! Unary predicate that matches a Metadatum with a given key
    class FindMetadatumByKey {
    public:
        //! Constructor, initializes the object with the tag to look for
        FindMetadatumByKey(const std::string& key) : key_(key) {}
        /*!
          @brief Returns true if the key of the argument metadatum is equal
          to that of the object.
        */
        bool operator()(const Metadatum& metadatum) const
            { return key_ == metadatum.key(); }

    private:
        std::string key_;
        
    }; // class FindMetadatumByTag

    /*!
      @brief Simple structure for 'raw' IFD directory entries (without any data
             greater than four bytes)
     */
    struct RawEntry {
        //! Default constructor
        RawEntry();
        //! The IFD id 
        IfdId ifdId_;
        //! Position in the IFD
        int ifdIdx_;
        //! Tag
        uint16 tag_;
        //! Type
        uint16 type_;
        //! Number of components
        uint32 count_;
        //! Offset, unprocessed
        uint32 offset_;
        //! Data from the IFD offset field if size is less or equal to four
        char offsetData_[4];
        //! Size of the data in bytes
        long size_;
    }; // struct RawEntry

    //! Container type to hold 'raw' IFD directory entries
    typedef std::vector<RawEntry> RawEntries;

    /*!
      @brief Data structure for one IFD directory entry. See the description of
             class Ifd for an explanation of the supported modes for memory
             allocation.
    */
    class Entry {
    public:
        /*! 
          @brief The status of an Entry for entries without memory management.

          <TABLE BORDER=0>
          <TR><TD>valid:</TD>
          <TD>The entry is valid, including in particular, the data 
          pointed to is valid and up to date.</TD></TR>
          <TR><TD>invalid:</TD>
          <TD>The entry is not valid, it has been invalidated by a 
          modification of the corresponding metadata, which did not 
          fit into the original data buffer.</TD></TR>
          <TR><TD>erased:</TD>
          <TD>The entry has been deleted.</TD></TR>
          </TABLE>
        */
        enum Status { valid, invalid, erased };

        /*!
          @brief Default constructor. The entry allocates memory for its 
          data if alloc is true (the default), otherwise it remembers
          just the pointers into a read and writeable data buffer which
          it doesn't allocate or delete.
        */ 
        explicit Entry(bool alloc =true);
        /*!
          @brief Constructor to create an %Entry from a raw entry structure
          and a data buffer. 

          @param e 'Raw' entry structure filled with the relevant data. The 
          offset_ field will only be used if size_ is greater than four.
          @param buf Character buffer with the data of the tag. If size_ is 
          less or equal four, the data from the original IFD offset 
          field must be available in the field offsetData_. The buf is
          not needed in this case and can be 0.
          @param alloc Determines if memory management is required. If alloc
          is true, a data buffer will be allocated to store data
          of more than four bytes, else only the pointer will be 
          stored. Data less or equal than four bytes is stored
          locally in the %Entry.
        */
        Entry(const RawEntry& e, const char* buf, bool alloc =true);
        //! Destructor
        ~Entry();
        //! Copy constructor
        Entry(const Entry& rhs);
        //! Assignment operator
        Entry& operator=(const Entry& rhs);
        //! @name Accessors
        //@{
        //! Return the status
        Status status() const { return status_; }
        //! Return the IFD id
        IfdId ifdId() const { return ifdId_; }
        //! Return the index in the IFD
        int ifdIdx() const { return ifdIdx_; }
        //! Return the tag
        uint16 tag() const { return tag_; }
        //! Return the type id.
        uint16 type() const { return type_; }
        //! Return the number of components in the value
        uint32 count() const { return count_; }
        //! Return the offset from the start of the IFD
        uint32 offset() const { return offset_; }
        //! Return the size of the value in bytes
        long size() const { return size_; }
        /*!
          @brief Return a pointer to the data area. Do not attempt to write
          to this pointer.
        */
        const char* data() const;
        //@}

        //! Return the size in bytes of one element of this type
        long typeSize() const
            { return ExifTags::typeSize(TypeId(type_)); }
        //! Return the name of the type
        const char* typeName() const 
            { return ExifTags::typeName(TypeId(type_)); }

        /*!
          @brief Set the offset. If the size of the data is not greater than
          four, the offset is written into the offset field as an
          unsigned long using the byte order given to encode it.
        */
        void setOffset(uint32 offset, ByteOrder byteOrder);
        //! Set the status 
        void setStatus(Status status) { status_ = status; }

    private:
        /*!
          @brief True:  requires memory allocation and deallocation,<BR>
          False: no memory management needed.
        */
        bool alloc_;
        Status status_;      // Status of this entry
        IfdId ifdId_;        // Redundant IFD id (it is also at the IFD)
        int ifdIdx_;         // Position in the IFD
        uint16 tag_;         // Tag
        uint16 type_;        // Type
        uint32 count_;       // Number of components
        uint32 offset_;      // Offset from the start of the IFD,
        // 0 if size <=4, i.e., if there is no offset
        char offsetData_[4]; // Data from the offset field if size <= 4
        long size_;          // Size of the data in bytes
        char* data_;         // Pointer to the data buffer
    }; // class Entry

    //! Container type to hold all IFD directory entries
    typedef std::vector<Entry> Entries;


    /*!
      @brief Models an IFD (Image File Directory)

      This class operates in two modes, one that allocates and deallocates the
      memory required to store the data, and one that doesn't perform such
      memory management and which is suitable in the case where a global data
      buffer is available and only pointers into this buffer need to be
      remembered.  Note that the different modes imply completely different copy
      and assignment behaviours, with the first resulting in entirely separate
      classes and the second mode resulting in multiple classes using one and
      the same data buffer. Use the default mode (with memory management) if
      possible. <BR>
      The mode without memory management is used to make "non-intrusive write
      support" possible. This allows writing to %Exif data of an image without
      changing the data layout of the %Exif data, to maximize chances that tag
      data, which the %Exif reader may not understand (e.g., the Makernote)
      remains valid. Permitted "non-intrusive write operations" include the
      modification of tag data without increasing the data size and deletion of
      entries from an IFD.
    */
    class Ifd {
    public:
        /*!
          @brief Constructor. Allows to set the IFD identifier and choose
                 whether or not memory management is required for the Entries.
         */
        explicit Ifd(IfdId ifdId =ifdIdNotSet, uint32 offset =0, bool alloc =true);

        //! Entries const iterator type
        typedef Entries::const_iterator const_iterator;
        //! Entries iterator type
        typedef Entries::iterator iterator;
        //! The first entry
        const_iterator begin() const { return entries_.begin(); }
        //! End of the entries
        const_iterator end() const { return entries_.end(); }
        //! The first entry
        iterator begin() { return entries_.begin(); }
        //! End of the entries
        iterator end() { return entries_.end(); }
        //! Find an IFD entry by tag, return a const iterator into the entries list
        const_iterator findTag(uint16 tag) const;
        //! Find an IFD entry by tag, return an iterator into the entries list
        iterator findTag(uint16 tag);
        //! Sort the IFD entries by tag
        void sortByTag();
        //! Delete the directory entry with the given tag
        void erase(uint16 tag);
        //! Delete the directory entry at iterator position pos
        void erase(iterator pos);
        /*!
          @brief Set the offset of the entry identified by tag. If no entry with
                 this tag exists, an entry of type unsigned long with one
                 component is created. If the size of the data is greater than
                 four, the offset of the entry is set to the value provided in
                 offset, else it is written to the offset field of the entry as
                 an unsigned long, encoded according to the byte order.
         */
        void setOffset(uint16 tag, uint32 offset, ByteOrder byteOrder);
        //! Set the offset of the next IFD
        void setNext(uint32 next) { next_ = next; }
        /*!
          @brief Add all metadata in the range from iterator position begin to
                 iterator position end, which have an IFD id matching that of
                 this IFD to the list of directory entries. Checks for
                 duplicates: if an entry with the same tag already exists, the
                 entry is overwritten.
         */
        void add(Metadata::const_iterator begin, 
                 Metadata::const_iterator end,
                 ByteOrder byteOrder);
        /*!
          @brief Add the metadatum to the list of directory entries. Does not
                 check for a matching IFD id.  Checks for duplicates: if an
                 entry with the same tag already exists, the entry is
                 overwritten.
         */
        void add(const Metadatum& metadatum, ByteOrder byteOrder);

        //! Unary predicate that matches an Entry with a given tag
        class FindEntryByTag {
        public:
            //! Constructor, initializes the object with the tag to look for
            FindEntryByTag(uint16 tag) : tag_(tag) {}
            /*!
              @brief Returns true if the tag of the argument entry is equal
              to that of the object.
            */
            bool operator()(const Entry& entry) const
                { return tag_ == entry.tag(); }
        private:
            uint16 tag_;
            
        }; // class FindEntryByTag

        /*!
          @brief Read a complete IFD and its data from a data buffer

          @param buf Pointer to the data to decode. The buffer must start with the 
                 IFD data (unlike the readSubIfd() method).
          @param byteOrder Applicable byte order (little or big endian).
          @param offset (Optional) offset of the IFD from the start of the TIFF
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
                     data starting from the TIFF header (unlike the read() method).
          @param byteOrder Applicable byte order (little or big endian).
          @param tag Tag to look for.

          @return 0 if successful
        */
        int readSubIfd(
            Ifd& dest, char* buf, ByteOrder byteOrder, uint16 tag
        ) const;
        /*!
          @brief Copy the IFD to a data array, return the number of bytes
                 written. 

                 First the number of IFD entries is written (2 bytes), followed
                 by all directory entries: tag (2), type (2), number of data
                 components (4) and offset to the data or the data, if it
                 occupies not more than four bytes (4). The directory entries
                 are followed by the offset of the next IFD (4). All these
                 fields are encoded according to the byte order argument. Data
                 that doesn't fit into the offset fields follows immediately
                 after the IFD entries. The offsets in the IFD are set to
                 correctly point to the data fields, using the offset parameter
                 or the offset of the IFD.

          @param buf Pointer to the data buffer. The user must ensure that the
                 buffer has enough memory. Otherwise the call results in
                 undefined behaviour.
          @param byteOrder Applicable byte order (little or big endian).
          @param offset Target offset from the start of the TIFF header of the
                 data array. The IFD offsets will be adjusted as necessary. If
                 not given, then it is assumed that the IFD will remain at its
                 original position, i.e., the offset of the IFD will be used.
          @return       Returns the number of characters written.
         */
        long copy(char* buf, ByteOrder byteOrder, long offset =0) const;
        //! @name Accessors
        //@{
        //! Ifd id of the IFD
        IfdId ifdId() const { return ifdId_; }
        //! Offset of the IFD from SOI
        long offset() const { return offset_; }
        //! Get the offset to the next IFD from the start of the TIFF header
        long next() const { return next_; }
        //@}
        //! Get the size of this IFD in bytes (IFD only, without data)
        long size() const;
        /*!
          @brief Return the total size of the data of this IFD in bytes,
                 sums the size of all directory entries where size is greater
                 than four (i.e., only data that requires memory outside the 
                 IFD directory entries is counted).
         */
        long dataSize() const;
        /*!
          @brief Print the IFD in human readable format to the given stream;
                 begin each line with prefix.
         */
        void print(std::ostream& os, const std::string& prefix ="") const;

    private:
        const bool alloc_; // True:  requires memory allocation and deallocation,
                           // False: no memory management needed.
        Entries entries_;  // IFD entries
        IfdId ifdId_;      // IFD Id
        long offset_;      // offset of the IFD from the start of TIFF header
        long next_;        // offset of next IFD from the start of the TIFF header

    }; // class Ifd

    //! %Thumbnail data Todo: add, create, rotate, delete
    class Thumbnail {
    public:
        //! %Thumbnail image types
        enum Type { JPEG, TIFF };
        /*!
          @brief Read the thumbnail from the data buffer buf, using %Exif
                 metadata exifData. Return 0 if successful. 

          @param buf Data buffer containing the thumbnail data. The buffer must
                 start with the TIFF header.  
          @param exifData %Exif data corresponding to the data buffer.
          @param byteOrder The byte order used for the encoding of TIFF
                 thumbnails. It determines the byte order of the resulting
                 thumbnail image, if it is in TIFF format. For JPEG thumbnails
                 the byte order is not used.
          @return 0 if successful<br>
                 -1 if there is no thumbnail image according to the %Exif data<br>
                  1 in case of inconsistent JPEG thumbnail %Exif data<br>
                  2 in case of inconsistent TIFF thumbnail %Exif data<br>
         */
        int read(const char* buf, 
                 const ExifData& exifData,
                 ByteOrder byteOrder =littleEndian);
        //! Write thumbnail to file path, return 0 if successful
        int write(const std::string& path) const;
        /*!
          @brief Copy the thumbnail image data (without the IFD, if any) to the
                 data buffer buf. The user must ensure that the buffer has
                 enough memory. Otherwise the call results in undefined
                 behaviour. Return the number of characters written.
         */
        long copy(char* buf) const;
        /*!
          @brief Update the %Exif data according to the actual thumbnail image.
          
          If the type of the thumbnail image is JPEG, JPEGInterchangeFormat is
          set to 0. If the type is TIFF, StripOffsets are set to the offsets of
          the IFD of the thumbnail image itself.
         */
        void update(ExifData& exifData) const;
        /*!
          @brief Update the thumbnail data offsets in IFD1 assuming the
                 thumbnail data follows immediately after IFD1.  

          If the type of the thumbnail image is JPEG, JPEGInterchangeFormat is
          set to point directly behind the data area of IFD1. If the type is
          TIFF, StripOffsets from the thumbnail image are adjusted to point to
          the strips, which have to follow immediately after IFD1. Use copy() to
          write the thumbnail image data. The offset of IFD1 must be set
          correctly. Changing the size of IFD1 invalidates the thumbnail data
          offsets set by this method.
         */
        void setOffsets(Ifd& ifd1, ByteOrder byteOrder) const;

    private:

        //! Read a compressed (JPEG) thumbnail image from the data buffer
        int readJpegImage(const char* buf, const ExifData& exifData);
        //! Read an uncompressed (TIFF) thumbnail image from the data buffer
        int readTiffImage(const char* buf,
                          const ExifData& exifData,
                          ByteOrder byteOrder);
        //! Update the Exif data according to the actual JPEG thumbnail image
        void updateJpegImage(ExifData& exifData) const;
        //! Update the Exif data according to the actual TIFF thumbnail image
        void updateTiffImage(ExifData& exifData) const;
        //! Copy the JPEG thumbnail image data to the data buffer buf
        long copyJpegImage(char* buf) const;
        //! Copy the TIFF thumbnail image data to the data buffer buf
        long copyTiffImage(char* buf) const;
        //! Update the offsets to the JPEG thumbnail image in the IFD
        void setJpegImageOffsets(Ifd& ifd1, ByteOrder byteOrder) const;
        //! Update the offsets to the TIFF thumbnail image in the IFD
        void setTiffImageOffsets(Ifd& ifd1, ByteOrder byteOrder) const;

        std::string image_;
        Type type_;
    }; // class Thumbnail

    /*!
      @brief A container for %Exif data

      Contains the %Exif data of a JPEG image
      - read and write access to all tags and data
      - iterators to access the %Exif data
      - decoding and encoding through the stream interface
      - human readable output
      - XML input and output
      - access to thumbnail (write, delete, re-calculate)

      Todo:
      - A constructor which creates a minimal valid set of %Exif data
    */
    class ExifData {
    public:
        //! Default constructor
        ExifData();
        //! Destructor
        ~ExifData();
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
          @brief Read the %Exif data from a character buffer. The data buffer
                 must start with the TIFF header.
          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer 
          @return 0 if successful
         */
        int read(const char* buf, long len);
        /*!
          @brief Write %Exif data to a data buffer, return number of bytes 
                 written. Updates %Exif data with the metadata from the actual
                 thumbnail image (overriding existing metadata).
         */ 
        long copy(char* buf);
        //! Returns the size of all %Exif data (TIFF header plus metadata)
        long size() const;
        //! Returns the byte order as specified in the TIFF header
        ByteOrder byteOrder() const { return tiffHeader_.byteOrder(); }
        /*!
          @brief Add all IFD entries in the range from iterator position begin
                 to iterator position end to the %Exif metadata. Checks for
                 duplicates: if a metadatum already exists, its value is
                 overwritten.
         */
        void add(Ifd::const_iterator begin, 
                 Ifd::const_iterator end,
                 ByteOrder byteOrder);
        /*!
          @brief Add a metadatum from the supplied key and value pair.
                 This method copies (clones) the value. If a metadatum with the
                 given key already exists, its value is overwritten and no new
                 metadatum is added.
         */
        void add(const std::string& key, Value* value);
        /*! 
          @brief Add a copy of the metadatum to the %Exif metadata. If a
                 metadatum with the given key already exists, its value is
                 overwritten and no new metadatum is added.
         */
        void add(const Metadatum& metadatum);

        //! Metadata iterator type
        typedef Metadata::iterator iterator;
        //! Metadata const iterator type
        typedef Metadata::const_iterator const_iterator;
        //! Begin of the metadata
        const_iterator begin() const { return metadata_.begin(); }
        //! End of the metadata
        const_iterator end() const { return metadata_.end(); }
        //! Begin of the metadata
        iterator begin() { return metadata_.begin(); }
        //! End of the metadata
        iterator end() { return metadata_.end(); }
        //! Find a metadatum by its key, return an iterator to it
        iterator findKey(const std::string& key);
        //! Find a metadatum by its key, return a const iterator to it
        const_iterator findKey(const std::string& key) const;
        //! Delete the metadatum with a given key
        void erase(const std::string& key);
        //! Delete the metadatum at iterator position pos
        void erase(iterator pos);

        /*!
          @brief Write the thumbnail image to a file. The filename extension
                 will be set according to the image type of the thumbnail, so
                 the path should not include an extension.
         */
        int writeThumbnail(const std::string& path) const 
            { return thumbnail_.write(path); }

    private:
        TiffHeader tiffHeader_;
        Metadata metadata_;
        Thumbnail thumbnail_;

        Ifd ifd0_;
        Ifd exifIfd_;
        Ifd iopIfd_;
        Ifd gpsIfd_;
        Ifd ifd1_;

        bool valid_;                      // Flag if data buffer is valid
        long size_;                       // Size of the Exif raw data in bytes
        char* data_;                      // Exif raw data buffer

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

    //! Print len bytes from buf in hex and ASCII format to the given stream
    void hexdump(std::ostream& os, const char* buf, long len);

    /*! 
      @brief Compare two 'raw' IFD entries by offset, taking care of special
             cases where one or both of the entries don't have an offset.
             Return true if the offset of entry lhs is less than that of rhs,
             else false. By definition, entries without an offset are greater
             than those with an offset.
     */
    bool cmpOffset(const RawEntry& lhs, const RawEntry& rhs);        
    /*!
      @brief Compare two IFD entries by tag. Return true if the tag of entry
             lhs is less than that of rhs.
     */
    bool cmpTag(const Entry& lhs, const Entry& rhs);

// *****************************************************************************
// template and inline definitions

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
    template<> 
    inline uint16 getValue(const char* buf, ByteOrder byteOrder)
    {
        return getUShort(buf, byteOrder);
    }
    // Specialization for a 4 byte unsigned long value.
    template<> 
    inline uint32 getValue(const char* buf, ByteOrder byteOrder)
    {
        return getULong(buf, byteOrder);
    }
    // Specialization for an 8 byte unsigned rational value.
    template<> 
    inline URational getValue(const char* buf, ByteOrder byteOrder)
    {
        return getURational(buf, byteOrder);
    }
    // Specialization for a 2 byte signed short value.
    template<> 
    inline int16 getValue(const char* buf, ByteOrder byteOrder)
    {
        return getShort(buf, byteOrder);
    }
    // Specialization for a 4 byte signed long value.
    template<> 
    inline int32 getValue(const char* buf, ByteOrder byteOrder)
    {
        return getLong(buf, byteOrder);
    }
    // Specialization for an 8 byte signed rational value.
    template<> 
    inline Rational getValue(const char* buf, ByteOrder byteOrder)
    {
        return getRational(buf, byteOrder);
    }

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
    template<> 
    inline long toData(char* buf, uint16 t, ByteOrder byteOrder)
    {
        return us2Data(buf, t, byteOrder);
    }
    /*! 
      @brief Specialization to write an unsigned long to the data buffer.
             Return the number of bytes written.
     */
    template<> 
    inline long toData(char* buf, uint32 t, ByteOrder byteOrder)
    {
        return ul2Data(buf, t, byteOrder);
    }
    /*!
      @brief Specialization to write an unsigned rational to the data buffer.
             Return the number of bytes written.
     */
    template<> 
    inline long toData(char* buf, URational t, ByteOrder byteOrder)
    {
        return ur2Data(buf, t, byteOrder);
    }
    /*! 
      @brief Specialization to write a signed short to the data buffer.
             Return the number of bytes written.
     */
    template<> 
    inline long toData(char* buf, int16 t, ByteOrder byteOrder)
    {
        return s2Data(buf, t, byteOrder);
    }
    /*! 
      @brief Specialization to write a signed long to the data buffer.
             Return the number of bytes written.
     */
    template<> 
    inline long toData(char* buf, int32 t, ByteOrder byteOrder)
    {
        return l2Data(buf, t, byteOrder);
    }
    /*!
      @brief Specialization to write a signed rational to the data buffer.
             Return the number of bytes written.
     */
    template<> 
    inline long toData(char* buf, Rational t, ByteOrder byteOrder)
    {
        return r2Data(buf, t, byteOrder);
    }

    template<typename T>
    void ValueType<T>::read(const char* buf, long len, ByteOrder byteOrder)
    {
        value_.clear();
        for (long i = 0; i < len; i += ExifTags::typeSize(typeId())) {
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
        return ExifTags::typeSize(typeId()) * value_.size();
    }

    template<typename T>
    Value* ValueType<T>::clone() const
    {
        return new ValueType<T>(*this);
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
    // Default implementation
    template<typename T>
    inline long ValueType<T>::toLong(long n) const
    { 
        return value_[n]; 
    }
    // Specialization for rational
    template<>
    inline long ValueType<Rational>::toLong(long n) const 
    {
        return value_[n].first / value_[n].second; 
    }
    // Specialization for unsigned rational
    template<>
    inline long ValueType<URational>::toLong(long n) const 
    {
        return value_[n].first / value_[n].second; 
    }
   
}                                       // namespace Exif

#endif                                  // #ifndef EXIF_HPP_
