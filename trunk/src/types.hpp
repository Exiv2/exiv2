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
  @file    types.hpp
  @brief   Type definitions for %Exiv2 and related functionality
  @version $Name:  $ $Revision: 1.11 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    09-Jan-04, ahu: created
           11-Feb-04, ahu: isolated as a component
 */
#ifndef TYPES_HPP_
#define TYPES_HPP_

// *****************************************************************************
// included header files

// + standard includes
#include <string>
#include <iosfwd>
#include <utility>
#include <sstream>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// type definitions

    //! 2 byte unsigned integer type.
    typedef unsigned short uint16;
    //! 4 byte unsigned integer type.
    typedef unsigned long  uint32;
    //! 2 byte signed integer type.
    typedef short          int16;
    //! 4 byte signed integer type.
    typedef long           int32;

    //! 8 byte unsigned rational type.
    typedef std::pair<uint32, uint32> URational;
    //! 8 byte signed rational type.
    typedef std::pair<int32, int32> Rational;

    //! Type to express the byte order (little or big endian)
    enum ByteOrder { invalidByteOrder, littleEndian, bigEndian };

    //! Type identifiers for IFD format types
    enum TypeId { invalidTypeId, unsignedByte, asciiString, unsignedShort, 
                  unsignedLong, unsignedRational, invalid6, undefined,
                  signedShort, signedLong, signedRational };

    //! Type to specify the IFD to which a metadata belongs
    enum IfdId { ifdIdNotSet, 
                 ifd0, exifIfd, gpsIfd, makerIfd, iopIfd, ifd1, 
                 lastIfdId};

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
        //! Return the size in bytes of one element of this type
        static long typeSize(TypeId typeId);

    private:
        static const TypeInfoTable typeInfoTable_[];
    };

    /*!
      @brief Utility class containing a character array. All it does is to take
             care of memory allocation and deletion. Its primary use is meant to
             be as a stack variable in functions that need a temporary data
             buffer. Todo: this should be some sort of smart pointer,
             essentially an std:auto_ptr for a character array. But it isn't.
     */
    class DataBuf {
        // Not implemented
        //! Copy constructor
        DataBuf(const DataBuf&);
        //! Assignment operator
        DataBuf& operator=(const DataBuf&);
    public:
        //! Default constructor
        DataBuf() : size_(0), pData_(0) {}
        //! Constructor with an initial buffer size 
        DataBuf(long size) : size_(size), pData_(new char[size]) {}
        //! Destructor, deletes the allocated buffer
        ~DataBuf() { delete[] pData_; }
        //! Allocate a data buffer of the given size
        void alloc(long size)
            { delete pData_; size_ = size; pData_ = new char[size]; }
        //! The current size of the buffer
        long size_; 
        //! Pointer to the buffer, 0 if none has been allocated
        char* pData_;
    }; // class DataBuf

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
      @brief Print len bytes from buf in hex and ASCII format to the given
             stream, prefixed with the position in the buffer adjusted by
             offset.
     */
    void hexdump(std::ostream& os, const char* buf, long len, long offset =0);

    /*!
      @brief Return the greatest common denominator of integers a and b.
             Both parameters must be greater than 0.
     */
    int gcd(int a, int b);

    /*!
      @brief Return the greatest common denominator of long values a and b. 
             Both parameters must be greater than 0.
     */
    long lgcd(long a, long b);
   
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

}                                       // namespace Exiv2

#endif                                  // #ifndef TYPES_HPP_
