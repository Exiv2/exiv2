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
  @file    value.hpp
  @brief   Value interface and concrete subclasses
  @version $Name:  $ $Revision: 1.7 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    09-Jan-04, ahu: created
           11-Feb-04, ahu: isolated as a component
 */
#ifndef VALUE_HPP_
#define VALUE_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

// *****************************************************************************
// namespace extensions
namespace Exif {

// *****************************************************************************
// class definitions

    /*!
      @brief Common interface for all types of values used with metadata. 

      The interface provides a uniform way to access values independent from
      their actual C++ type for simple tasks like reading the values from a
      string or data buffer.  For other tasks, like modifying values you may
      need to downcast it to the actual subclass of %Value so that you can
      access the subclass specific interface.
     */
    class Value {
    public:
        //! @name Creators
        //@{
        //! Constructor, taking a type id to initialize the base class with
        explicit Value(TypeId typeId) : type_(typeId) {}
        //! Virtual destructor.
        virtual ~Value() {}
        //@}

        //! @name Manipulators
        //@{
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
        //@}

        //! @name Accessors
        //@{
        //! Return the type identifier (Exif data format type).
        TypeId typeId() const { return TypeId(type_); }
        /*!
          @brief Return the value as a string. Implemented in terms of
                 write(std::ostream& os) const of the concrete class. 
         */
        std::string toString() const;
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
        /*!
          @brief Convert the n-th component of the value to a float. The
                 behaviour of this method may be undefined if there is no
                 n-th component.

          @return The converted value. 
         */
        virtual float toFloat(long n =0) const =0;
        /*!
          @brief Convert the n-th component of the value to a Rational. The
                 behaviour of this method may be undefined if there is no
                 n-th component.

          @return The converted value. 
         */
        virtual Rational toRational(long n =0) const =0;
        //@}

        /*!
          @brief A (simple) factory to create a Value type.

          The following Value subclasses are created depending on typeId:<BR><BR>
          <TABLE>
          <TR><TD><B>typeId</B></TD><TD><B>%Value subclass</B></TD></TR>
          <TR><TD>invalidTypeId</TD><TD>%DataValue(invalidTypeId)</TD></TR>
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

    protected:
        /*!
          @brief Assignment operator. Protected so that it can only be used
                 by subclasses but not directly.
         */
        Value& operator=(const Value& rhs);

    private:
        uint16 type_;                           //!< Type of the data

    }; // class Value

    //! Output operator for Value types
    inline std::ostream& operator<<(std::ostream& os, const Value& value)
    {
        return value.write(os);
    }

    //! %Value for an undefined data type.
    class DataValue : public Value {
    public:
        //! @name Creators
        //@{
        //! Default constructor.
        DataValue(TypeId typeId =undefined) : Value(typeId) {}
        //! Virtual destructor.
        virtual ~DataValue() {}
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator.
        DataValue& operator=(const DataValue& rhs);
        /*!
          @brief Read the value from a character buffer. 

          @note The byte order is required by the interface but not 
                used by this method, so just use the default.

          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer 
          @param byteOrder Byte order. Not needed.
         */
        virtual void read(const char* buf,
                          long len, 
                          ByteOrder byteOrder =invalidByteOrder);
        //! Set the data from a string of integer values (e.g., "0 1 2 3")
        virtual void read(const std::string& buf);
        //@}

        //! @name Accessors
        //@{        
        /*!
          @brief Write value to a character data buffer. 

          @note The byte order is required by the interface but not used by this
                method, so just use the default.

          The user must ensure that the buffer has enough memory. Otherwise
          the call results in undefined behaviour.

          @param buf Data buffer to write to.
          @param byteOrder Byte order. Not needed.
          @return Number of characters written.
        */
        virtual long copy(char* buf, ByteOrder byteOrder =invalidByteOrder) const;
        virtual long count() const { return size(); }
        virtual long size() const;
        virtual Value* clone() const;
        virtual std::ostream& write(std::ostream& os) const;
        virtual long toLong(long n =0) const { return value_[n]; }
        virtual float toFloat(long n =0) const { return value_[n]; }
        virtual Rational toRational(long n =0) const
            { return Rational(value_[n], 1); }
        //@}

    private:
        std::string value_;

    }; // class DataValue

    //! %Value for an Ascii string type.
    class AsciiValue : public Value {
    public:
        //! @name Creators
        //@{
        //! Default constructor.
        AsciiValue() : Value(asciiString) {}
        //! Virtual destructor.
        virtual ~AsciiValue() {}
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator.
        AsciiValue& operator=(const AsciiValue& rhs);
        /*!
          @brief Read the value from a character buffer.

          @note The byte order is required by the interface but not used by this
                method, so just use the default.

          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer 
          @param byteOrder Byte order. Not needed.
         */
        virtual void read(const char* buf, 
                          long len, 
                          ByteOrder byteOrder =invalidByteOrder);
        /*!
          @brief Set the value to that of the string buf. A terminating
                 '\\0' character is appended to the value if buf doesn't 
                 end with '\\0'.
         */
        virtual void read(const std::string& buf);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Write value to a character data buffer.

          The user must ensure that the buffer has enough memory. Otherwise
          the call results in undefined behaviour.

          @note The byte order is required by the interface but not used by this
                method, so just use the default.

          @param buf Data buffer to write to.
          @param byteOrder Byte order. Not used.
          @return Number of characters written.
        */
        virtual long copy(char* buf, ByteOrder byteOrder =invalidByteOrder) const;
        virtual long count() const { return size(); }
        virtual long size() const;
        virtual Value* clone() const;
        /*! 
          @brief Write the value to an output stream. Any trailing '\\0'
                 characters of the ASCII value are stripped and not written to
                 the output stream.
        */
        virtual std::ostream& write(std::ostream& os) const;
        virtual long toLong(long n =0) const { return value_[n]; }
        virtual float toFloat(long n =0) const { return value_[n]; }
        virtual Rational toRational(long n =0) const
            { return Rational(value_[n], 1); }
        //@}

    private:
        std::string value_;

    }; // class AsciiValue

    //! Template to determine the TypeId for a type T
    template<typename T> TypeId getType();

    //! Specialization for an unsigned short
    template<> inline TypeId getType<uint16>() { return unsignedShort; }
    //! Specialization for an unsigned long
    template<> inline TypeId getType<uint32>() { return unsignedLong; }
    //! Specialization for an unsigned rational
    template<> inline TypeId getType<URational>() { return unsignedRational; }
    //! Specialization for a signed short
    template<> inline TypeId getType<int16>() { return signedShort; }
    //! Specialization for a signed long
    template<> inline TypeId getType<int32>() { return signedLong; }
    //! Specialization for a signed rational
    template<> inline TypeId getType<Rational>() { return signedRational; }

    // No default implementation: let the compiler/linker complain
//    template<typename T> inline TypeId getType() { return invalid; }

    /*!
      @brief Template for a %Value of a basic type. This is used for unsigned 
             and signed short, long and rationals.
     */    
    template<typename T>
    class ValueType : public Value {
    public:
        //! @name Creators
        //@{
        //! Default constructor.
        ValueType() : Value(getType<T>()) {}
        //! Virtual destructor.
        virtual ~ValueType() {}
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator.
        ValueType<T>& operator=(const ValueType<T>& rhs);
        virtual void read(const char* buf, long len, ByteOrder byteOrder);
        /*!
          @brief Set the data from a string of values of type T (e.g., 
                 "0 1 2 3" or "1/2 1/3 1/4" depending on what T is). 
                 Generally, the accepted input format is the same as that 
                 produced by the write() method.
         */
        virtual void read(const std::string& buf);
        //@}

        //! @name Accessors
        //@{
        virtual long copy(char* buf, ByteOrder byteOrder) const;
        virtual long count() const { return value_.size(); }
        virtual long size() const;
        virtual Value* clone() const;
        virtual std::ostream& write(std::ostream& os) const;
        virtual long toLong(long n =0) const;
        virtual float toFloat(long n =0) const;
        virtual Rational toRational(long n =0) const;
        //@}

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
    ValueType<T>& ValueType<T>::operator=(const ValueType<T>& rhs)
    {
        if (this == &rhs) return *this;
        Value::operator=(rhs);
        value_ = rhs.value_;
        return *this;
    }

    template<typename T>
    void ValueType<T>::read(const char* buf, long len, ByteOrder byteOrder)
    {
        value_.clear();
        for (long i = 0; i < len; i += TypeInfo::typeSize(typeId())) {
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
        return TypeInfo::typeSize(typeId()) * value_.size();
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
    // Default implementation
    template<typename T>
    inline float ValueType<T>::toFloat(long n) const
    { 
        return value_[n]; 
    }
    // Specialization for rational
    template<>
    inline float ValueType<Rational>::toFloat(long n) const 
    {
        return (float)value_[n].first / value_[n].second; 
    }
    // Specialization for unsigned rational
    template<>
    inline float ValueType<URational>::toFloat(long n) const 
    {
        return (float)value_[n].first / value_[n].second; 
    }
    // Default implementation
    template<typename T>
    inline Rational ValueType<T>::toRational(long n) const
    {
        return Rational(value_[n], 1);
    }
    // Specialization for rational
    template<>
    inline Rational ValueType<Rational>::toRational(long n) const 
    {
        return Rational(value_[n].first, value_[n].second);
    }
    // Specialization for unsigned rational
    template<>
    inline Rational ValueType<URational>::toRational(long n) const 
    {
        return Rational(value_[n].first, value_[n].second);
    }
   
}                                       // namespace Exif

#endif                                  // #ifndef VALUE_HPP_
