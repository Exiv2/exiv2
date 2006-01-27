// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
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
  @file    types.hpp
  @brief   Type definitions for %Exiv2 and related functionality
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    09-Jan-04, ahu: created<BR>
           11-Feb-04, ahu: isolated as a component
           31-Jul-04, brad: added Time, Data and String values
 */
#ifndef TYPES_HPP_
#define TYPES_HPP_

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

// + standard includes
#include <string>
#include <iosfwd>
#include <utility>
#include <sstream>
#ifdef EXV_HAVE_STDINT_H
# include <stdint.h>
#endif

// MSVC doesn't provide C99 types, but it has MS specific variants
#ifdef _MSC_VER
typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef __int16          int16_t;
typedef __int32          int32_t;
#endif

// *****************************************************************************
// forward declarations
struct tm;

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// type definitions

    //! 1 byte unsigned integer type.
    typedef uint8_t byte;

    //! 8 byte unsigned rational type.
    typedef std::pair<uint32_t, uint32_t> URational;
    //! 8 byte signed rational type.
    typedef std::pair<int32_t, int32_t> Rational;

    //! Type to express the byte order (little or big endian)
    enum ByteOrder { invalidByteOrder, littleEndian, bigEndian };

    //! Type identifiers for IFD format types
    enum TypeId { invalidTypeId, unsignedByte, asciiString, unsignedShort,
                  unsignedLong, unsignedRational, invalid6, undefined,
                  signedShort, signedLong, signedRational,
                  string, date, time,
                  comment,
                  directory,
                  lastTypeId };

    // Todo: decentralize IfdId, so that new ids can be defined elsewhere
    //! Type to specify the IFD to which a metadata belongs
    enum IfdId { ifdIdNotSet,
                 ifd0Id, exifIfdId, gpsIfdId, iopIfdId, ifd1Id,
                 canonIfdId, canonCs1IfdId, canonCs2IfdId, canonCfIfdId,
                 fujiIfdId, nikon1IfdId, nikon2IfdId, nikon3IfdId,
                 olympusIfdId, panasonicIfdId, sigmaIfdId, sonyIfdId,
                 lastIfdId };

    //! Type to identify where the data is stored in a directory
    enum DataLocId { invalidDataLocId,
                     valueData, directoryData,
                     lastDataLocId };

// *****************************************************************************
// class definitions

    //! Information pertaining to the defined types
    struct TypeInfoTable {
        //! Constructor
        TypeInfoTable(TypeId typeId, const char* name, long size);
        TypeId typeId_;                         //!< Type id
        const char* name_;                      //!< Name of the type
        long size_;                             //!< Bytes per data entry
    }; // struct TypeInfoTable

    //! Type information lookup functions. Implemented as a static class.
    class TypeInfo {
        //! Prevent construction: not implemented.
        TypeInfo() {}
        //! Prevent copy-construction: not implemented.
        TypeInfo(const TypeInfo& rhs);
        //! Prevent assignment: not implemented.
        TypeInfo& operator=(const TypeInfo& rhs);

    public:
        //! Return the name of the type
        static const char* typeName(TypeId typeId);
        //! Return the type id for a type name
        static TypeId typeId(const std::string& typeName);
        //! Return the size in bytes of one element of this type
        static long typeSize(TypeId typeId);

    private:
        static const TypeInfoTable typeInfoTable_[];
    };

    /*!
      @brief Auxiliary type to enable copies and assignments, similar to
             std::auto_ptr_ref. See http://www.josuttis.com/libbook/auto_ptr.html
             for a discussion.
     */
    struct DataBufRef {
        //! Constructor
        DataBufRef(std::pair<byte*, long> rhs) : p(rhs) {}
        //! Pointer to a byte array and its size
        std::pair<byte*, long> p;
    };

    /*!
      @brief Utility class containing a character array. All it does is to take
             care of memory allocation and deletion. Its primary use is meant to
             be as a stack variable in functions that need a temporary data
             buffer. Todo: this should be some sort of smart pointer,
             essentially an std::auto_ptr for a character array. But it isn't...
     */
    class DataBuf {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        DataBuf() : pData_(0), size_(0) {}
        //! Constructor with an initial buffer size
        explicit DataBuf(long size) : pData_(new byte[size]), size_(size) {}
        //! Constructor, copies an existing buffer
        DataBuf(const byte* pData, long size);
        /*!
          @brief Copy constructor. Transfers the buffer to the newly created
                 object similar to std::auto_ptr, i.e., the original object is
                 modified.
         */
        DataBuf(DataBuf& rhs);
        //! Destructor, deletes the allocated buffer
        ~DataBuf() { delete[] pData_; }
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Assignment operator. Transfers the buffer and releases the
                 buffer at the original object similar to std::auto_ptr, i.e.,
                 the original object is modified.
         */
        DataBuf& operator=(DataBuf& rhs);
        //! Allocate a data buffer of the given size
        void alloc(long size);
        /*!
          @brief Release ownership of the buffer to the caller. Returns the
                 buffer as a data pointer and size pair, resets the internal
                 buffer.
         */
        std::pair<byte*, long> release();
        //! Reset value
        void reset(std::pair<byte*, long> =std::make_pair((byte*)(0),long(0)));
        //@}

        /*!
          @name Conversions

          Special conversions with auxiliary type to enable copies
          and assignments, similar to those used for std::auto_ptr.
          See http://www.josuttis.com/libbook/auto_ptr.html for a discussion.
         */
        //@{
        DataBuf(DataBufRef rhs) : pData_(rhs.p.first), size_(rhs.p.second) {}
        DataBuf& operator=(DataBufRef rhs) { reset(rhs.p); return *this; }
        operator DataBufRef() { return DataBufRef(release()); }
        //@}

        // DATA
        //! Pointer to the buffer, 0 if none has been allocated
        byte* pData_;
        //! The current size of the buffer
        long size_;
    }; // class DataBuf


// *****************************************************************************
// free functions

    //! Read a 2 byte unsigned short value from the data buffer
    uint16_t getUShort(const byte* buf, ByteOrder byteOrder);
    //! Read a 4 byte unsigned long value from the data buffer
    uint32_t getULong(const byte* buf, ByteOrder byteOrder);
    //! Read an 8 byte unsigned rational value from the data buffer
    URational getURational(const byte* buf, ByteOrder byteOrder);
    //! Read a 2 byte signed short value from the data buffer
    int16_t getShort(const byte* buf, ByteOrder byteOrder);
    //! Read a 4 byte signed long value from the data buffer
    int32_t getLong(const byte* buf, ByteOrder byteOrder);
    //! Read an 8 byte signed rational value from the data buffer
    Rational getRational(const byte* buf, ByteOrder byteOrder);

    //! Output operator for our fake rational
    std::ostream& operator<<(std::ostream& os, const Rational& r);
    //! Input operator for our fake rational
    std::istream& operator>>(std::istream& is, Rational& r);
    //! Output operator for our fake unsigned rational
    std::ostream& operator<<(std::ostream& os, const URational& r);
    //! Input operator for our fake unsigned rational
    std::istream& operator>>(std::istream& is, URational& r);

    /*!
      @brief Convert an unsigned short to data, write the data to the buffer,
             return number of bytes written.
     */
    long us2Data(byte* buf, uint16_t s, ByteOrder byteOrder);
    /*!
      @brief Convert an unsigned long to data, write the data to the buffer,
             return number of bytes written.
     */
    long ul2Data(byte* buf, uint32_t l, ByteOrder byteOrder);
    /*!
      @brief Convert an unsigned rational to data, write the data to the buffer,
             return number of bytes written.
     */
    long ur2Data(byte* buf, URational l, ByteOrder byteOrder);
    /*!
      @brief Convert a signed short to data, write the data to the buffer,
             return number of bytes written.
     */
    long s2Data(byte* buf, int16_t s, ByteOrder byteOrder);
    /*!
      @brief Convert a signed long to data, write the data to the buffer,
             return number of bytes written.
     */
    long l2Data(byte* buf, int32_t l, ByteOrder byteOrder);
    /*!
      @brief Convert a signed rational to data, write the data to the buffer,
             return number of bytes written.
     */
    long r2Data(byte* buf, Rational l, ByteOrder byteOrder);

    /*!
      @brief Print len bytes from buf in hex and ASCII format to the given
             stream, prefixed with the position in the buffer adjusted by
             offset.
     */
    void hexdump(std::ostream& os, const byte* buf, long len, long offset =0);

    /*!
      @brief Return true if str is a hex number starting with prefix followed
             by size hex digits, false otherwise. If size is 0, any number of
             digits is allowed and all are checked.
     */
    bool isHex(const std::string& str,
               size_t size =0,
               const std::string& prefix ="");

    /*!
      @brief Converts a string in the form "%Y:%m:%d %H:%M:%S", e.g.,
             "2007:05:24 12:31:55" to broken down time format, 
             returns 0 if successful, else 1.
     */
    int exifTime(const char* buf, struct tm* tm);

// *****************************************************************************
// template and inline definitions

    //! Utility function to convert the argument of any type to a string
    template<typename T>
    std::string toString(const T& arg)
    {
        std::ostringstream os;
        os << arg;
        return os.str();
    }

    /*!
      @brief Return the greatest common denominator of n and m.
             (implementation from Boost rational.hpp)

      @note We use n and m as temporaries in this function, so there is no
            value in using const IntType& as we would only need to make a copy
            anyway...
     */
    template <typename IntType>
    IntType gcd(IntType n, IntType m)
    {
        // Avoid repeated construction
        IntType zero(0);

        // This is abs() - given the existence of broken compilers with Koenig
        // lookup issues and other problems, I code this explicitly. (Remember,
        // IntType may be a user-defined type).
#ifdef _MSC_VER
#pragma warning( disable : 4146 )
#endif
        if (n < zero)
            n = -n;
        if (m < zero)
            m = -m;
#ifdef _MSC_VER
#pragma warning( default : 4146 )
#endif

        // As n and m are now positive, we can be sure that %= returns a
        // positive value (the standard guarantees this for built-in types,
        // and we require it of user-defined types).
        for(;;) {
            if(m == zero)
                return n;
            n %= m;
            if(n == zero)
                return m;
            m %= n;
        }
    }

}                                       // namespace Exiv2

#endif                                  // #ifndef TYPES_HPP_
