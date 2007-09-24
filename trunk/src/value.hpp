// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2007 Andreas Huggel <ahuggel@gmx.net>
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
  @file    value.hpp
  @brief   Value interface and concrete subclasses
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    09-Jan-04, ahu: created
           11-Feb-04, ahu: isolated as a component
           31-Jul-04, brad: added Time, Data and String values
 */
#ifndef VALUE_HPP_
#define VALUE_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <memory>
#include <cstring>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    /*!
      @brief Common interface for all types of values used with metadata.

      The interface provides a uniform way to access values independent of
      their actual C++ type for simple tasks like reading the values from a
      string or data buffer.  For other tasks, like modifying values you may
      need to downcast it to a specific subclass to access its interface.
     */
    class Value {
    public:
        //! Shortcut for a %Value auto pointer.
        typedef std::auto_ptr<Value> AutoPtr;

        //! @name Creators
        //@{
        //! Constructor, taking a type id to initialize the base class with
        explicit Value(TypeId typeId);
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

          @return 0 if successful.
         */
        virtual int read(const byte* buf, long len, ByteOrder byteOrder) =0;
        /*!
          @brief Set the value from a string buffer. The format of the string
                 corresponds to that of the write() method, i.e., a string
                 obtained through the write() method can be read by this
                 function.

          @param buf The string to read from.

          @return 0 if successful.
         */
        virtual int read(const std::string& buf) =0;
        /*!
          @brief Set the data area, if the value has one by copying (cloning)
                 the buffer pointed to by buf.

          Values may have a data area, which can contain additional
          information besides the actual value. This method is used to set such
          a data area.

          @param buf Pointer to the source data area
          @param len Size of the data area
          @return Return -1 if the value has no data area, else 0.
         */
        virtual int setDataArea(const byte* buf, long len);
        //@}

        //! @name Accessors
        //@{
        //! Return the type identifier (Exif data format type).
        TypeId typeId() const { return type_; }
        /*!
          @brief Return an auto-pointer to a copy of itself (deep copy).
                 The caller owns this copy and the auto-pointer ensures that
                 it will be deleted.
         */
        AutoPtr clone() const { return AutoPtr(clone_()); }
        /*!
          @brief Write value to a data buffer.

          The user must ensure that the buffer has enough memory. Otherwise
          the call results in undefined behaviour.

          @param buf Data buffer to write to.
          @param byteOrder Applicable byte order (little or big endian).
          @return Number of bytes written.
        */
        virtual long copy(byte* buf, ByteOrder byteOrder) const =0;
        //! Return the number of components of the value
        virtual long count() const =0;
        //! Return the size of the value in bytes
        virtual long size() const =0;
        /*!
          @brief Write the value to an output stream. You do not usually have
                 to use this function; it is used for the implementation of
                 the output operator for %Value,
                 operator<<(std::ostream &os, const Value &value).
        */
        virtual std::ostream& write(std::ostream& os) const =0;
        /*!
          @brief Return the value as a string. Implemented in terms of
                 write(std::ostream& os) const of the concrete class.
         */
        std::string toString() const;
        /*!
          @brief Return the <EM>n</EM>-th component of the value as a string.
                 The default implementation returns toString(). The behaviour
                 of this method may be undefined if there is no <EM>n</EM>-th
                 component.
         */
        virtual std::string toString(long n) const;
        /*!
          @brief Convert the <EM>n</EM>-th component of the value to a long.
                 The behaviour of this method may be undefined if there is no
                 <EM>n</EM>-th component.

          @return The converted value.
         */
        virtual long toLong(long n =0) const =0;
        /*!
          @brief Convert the <EM>n</EM>-th component of the value to a float.
                 The behaviour of this method may be undefined if there is no
                 <EM>n</EM>-th component.

          @return The converted value.
         */
        virtual float toFloat(long n =0) const =0;
        /*!
          @brief Convert the <EM>n</EM>-th component of the value to a Rational.
                 The behaviour of this method may be undefined if there is no
                 <EM>n</EM>-th component.

          @return The converted value.
         */
        virtual Rational toRational(long n =0) const =0;
        //! Return the size of the data area, 0 if there is none.
        virtual long sizeDataArea() const { return 0; }
        /*!
          @brief Return a copy of the data area if the value has one. The
                 caller owns this copy and DataBuf ensures that it will be
                 deleted.

          Values may have a data area, which can contain additional
          information besides the actual value. This method is used to access
          such a data area.

          @return A DataBuf containing a copy of the data area or an empty
                  DataBuf if the value does not have a data area assigned.
         */
        virtual DataBuf dataArea() const { return DataBuf(0, 0); };
        /*!
          @brief Check the \em ok status indicator. After a to<Type> conversion,
                 this indicator shows whether the conversion was successful.
         */
        bool ok() const { return ok_; }
        //@}

        /*!
          @brief A (simple) factory to create a Value type.

          The following Value subclasses are created depending on typeId:<BR><BR>
          <TABLE>
          <TR><TD class="indexkey"><B>typeId</B></TD><TD class="indexvalue"><B>%Value subclass</B></TD></TR>
          <TR><TD class="indexkey">invalidTypeId</TD><TD class="indexvalue">%DataValue(invalidTypeId)</TD></TR>
          <TR><TD class="indexkey">unsignedByte</TD><TD class="indexvalue">%DataValue(unsignedByte)</TD></TR>
          <TR><TD class="indexkey">asciiString</TD><TD class="indexvalue">%AsciiValue</TD></TR>
          <TR><TD class="indexkey">string</TD><TD class="indexvalue">%StringValue</TD></TR>
          <TR><TD class="indexkey">unsignedShort</TD><TD class="indexvalue">%ValueType &lt; uint16_t &gt;</TD></TR>
          <TR><TD class="indexkey">unsignedLong</TD><TD class="indexvalue">%ValueType &lt; uint32_t &gt;</TD></TR>
          <TR><TD class="indexkey">unsignedRational</TD><TD class="indexvalue">%ValueType &lt; URational &gt;</TD></TR>
          <TR><TD class="indexkey">invalid6</TD><TD class="indexvalue">%DataValue(invalid6)</TD></TR>
          <TR><TD class="indexkey">undefined</TD><TD class="indexvalue">%DataValue</TD></TR>
          <TR><TD class="indexkey">signedShort</TD><TD class="indexvalue">%ValueType &lt; int16_t &gt;</TD></TR>
          <TR><TD class="indexkey">signedLong</TD><TD class="indexvalue">%ValueType &lt; int32_t &gt;</TD></TR>
          <TR><TD class="indexkey">signedRational</TD><TD class="indexvalue">%ValueType &lt; Rational &gt;</TD></TR>
          <TR><TD class="indexkey">date</TD><TD class="indexvalue">%DateValue</TD></TR>
          <TR><TD class="indexkey">time</TD><TD class="indexvalue">%TimeValue</TD></TR>
          <TR><TD class="indexkey">comment</TD><TD class="indexvalue">%CommentValue</TD></TR>
          <TR><TD class="indexkey">xmpText</TD><TD class="indexvalue">%XmpTextValue</TD></TR>
          <TR><TD class="indexkey">xmpBag</TD><TD class="indexvalue">%XmpArrayValue</TD></TR>
          <TR><TD class="indexkey">xmpSeq</TD><TD class="indexvalue">%XmpArrayValue</TD></TR>
          <TR><TD class="indexkey">xmpAlt</TD><TD class="indexvalue">%XmpArrayValue</TD></TR>
          <TR><TD class="indexkey">langAlt</TD><TD class="indexvalue">%LangAltValue</TD></TR>
          <TR><TD class="indexkey"><EM>default:</EM></TD><TD class="indexvalue">%DataValue(typeId)</TD></TR>
          </TABLE>

          @param typeId Type of the value.
          @return Auto-pointer to the newly created Value. The caller owns this
                  copy and the auto-pointer ensures that it will be deleted.
         */
        static AutoPtr create(TypeId typeId);

    protected:
        /*!
          @brief Assignment operator. Protected so that it can only be used
                 by subclasses but not directly.
         */
        Value& operator=(const Value& rhs);
        // DATA
        mutable bool ok_;                //!< Indicates the status of the previous to<Type> conversion

    private:
        //! Internal virtual copy constructor.
        virtual Value* clone_() const =0;
        // DATA
        TypeId type_;                    //!< Type of the data

    }; // class Value

    //! Output operator for Value types
    inline std::ostream& operator<<(std::ostream& os, const Value& value)
    {
        return value.write(os);
    }

    //! %Value for an undefined data type.
    class DataValue : public Value {
    public:
        //! Shortcut for a %DataValue auto pointer.
        typedef std::auto_ptr<DataValue> AutoPtr;

        //! @name Creators
        //@{
        //! Default constructor.
        explicit DataValue(TypeId typeId =undefined) : Value(typeId) {}
        //! Constructor
        DataValue(const byte* buf,
                  long len, ByteOrder byteOrder =invalidByteOrder,
                  TypeId typeId =undefined)
            : Value(typeId) { read(buf, len, byteOrder); }
        //! Virtual destructor.
        virtual ~DataValue() {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the value from a character buffer.

          @note The byte order is required by the interface but not
                used by this method, so just use the default.

          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer
          @param byteOrder Byte order. Not needed.

          @return 0 if successful.
         */
        virtual int read(const byte* buf,
                          long len,
                          ByteOrder byteOrder =invalidByteOrder);
        //! Set the data from a string of integer values (e.g., "0 1 2 3")
        virtual int read(const std::string& buf);
        //@}

        //! @name Accessors
        //@{
        AutoPtr clone() const { return AutoPtr(clone_()); }
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
        virtual long copy(byte* buf, ByteOrder byteOrder =invalidByteOrder) const;
        virtual long count() const { return size(); }
        virtual long size() const;
        virtual std::ostream& write(std::ostream& os) const;
        /*!
          @brief Return the <EM>n</EM>-th component of the value as a string.
                 The behaviour of this method may be undefined if there is no
                 <EM>n</EM>-th component.
         */
        virtual std::string toString(long n) const;
        virtual long toLong(long n =0) const;
        virtual float toFloat(long n =0) const;
        virtual Rational toRational(long n =0) const;
        //@}

    private:
        //! Internal virtual copy constructor.
        virtual DataValue* clone_() const;

    public:
        //! Type used to store the data.
        typedef std::vector<byte> ValueType;
        // DATA
        ValueType value_;                       //!< Stores the data value 

    }; // class DataValue

    /*!
      @brief Abstract base class for a string based %Value type.

      Uses a std::string to store the value and implements defaults for
      most operations.
     */
    class StringValueBase : public Value {
    public:
        //! Shortcut for a %StringValueBase auto pointer.
        typedef std::auto_ptr<StringValueBase> AutoPtr;

        //! @name Creators
        //@{
        //! Constructor for subclasses
        explicit StringValueBase(TypeId typeId)
            : Value(typeId) {}
        //! Constructor for subclasses
        StringValueBase(TypeId typeId, const std::string& buf)
            : Value(typeId) { read(buf); }
        //! Copy constructor
        StringValueBase(const StringValueBase& rhs)
            : Value(rhs), value_(rhs.value_) {}

        //! Virtual destructor.
        virtual ~StringValueBase() {}
        //@}

        //! @name Manipulators
        //@{
        //! Read the value from buf. This default implementation uses buf as it is.
        virtual int read(const std::string& buf);
        /*!
          @brief Read the value from a character buffer.

          @note The byte order is required by the interface but not used by this
                method, so just use the default.

          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer
          @param byteOrder Byte order. Not needed.

          @return 0 if successful.
         */
        virtual int read(const byte* buf,
                         long len,
                         ByteOrder byteOrder =invalidByteOrder);
        //@}

        //! @name Accessors
        //@{
        AutoPtr clone() const { return AutoPtr(clone_()); }
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
        virtual long copy(byte* buf, ByteOrder byteOrder =invalidByteOrder) const;
        virtual long count() const { return size(); }
        virtual long size() const;
        virtual long toLong(long n =0) const;
        virtual float toFloat(long n =0) const;
        virtual Rational toRational(long n =0) const;
        virtual std::ostream& write(std::ostream& os) const;
        //@}

    protected:
        //! Assignment operator.
        StringValueBase& operator=(const StringValueBase& rhs);
        //! Internal virtual copy constructor.
        virtual StringValueBase* clone_() const =0;

    public:
        // DATA
        std::string value_;                     //!< Stores the string value.

    }; // class StringValueBase

    /*!
      @brief %Value for string type.

      This can be a plain Ascii string or a multipe byte encoded string. It is
      left to caller to decode and encode the string to and from readable
      text if that is required.
    */
    class StringValue : public StringValueBase {
    public:
        //! Shortcut for a %StringValue auto pointer.
        typedef std::auto_ptr<StringValue> AutoPtr;

        //! @name Creators
        //@{
        //! Default constructor.
        StringValue()
            : StringValueBase(string) {}
        //! Constructor
        explicit StringValue(const std::string& buf)
            : StringValueBase(string, buf) {}
        //! Virtual destructor.
        virtual ~StringValue() {}
        //@}

        //! @name Accessors
        //@{
        AutoPtr clone() const { return AutoPtr(clone_()); }
        //@}

    private:
        //! Internal virtual copy constructor.
        virtual StringValue* clone_() const;

    }; // class StringValue

    /*!
      @brief %Value for an Ascii string type.

      This class is for null terminated single byte Ascii strings.
      This class also ensures that the string is null terminated.
     */
    class AsciiValue : public StringValueBase {
    public:
        //! Shortcut for a %AsciiValue auto pointer.
        typedef std::auto_ptr<AsciiValue> AutoPtr;

        //! @name Creators
        //@{
        //! Default constructor.
        AsciiValue()
            : StringValueBase(asciiString) {}
        //! Constructor
        explicit AsciiValue(const std::string& buf)
            : StringValueBase(asciiString, buf) {}
        //! Virtual destructor.
        virtual ~AsciiValue() {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Set the value to that of the string buf. Overrides base class
                 to append a terminating '\\0' character if buf doesn't end
                 with '\\0'.
         */
        virtual int read(const std::string& buf);
        //@}

        //! @name Accessors
        //@{
        AutoPtr clone() const { return AutoPtr(clone_()); }
        /*!
          @brief Write the value to an output stream. Any trailing '\\0'
                 characters of the ASCII value are stripped and not written to
                 the output stream.
        */
        virtual std::ostream& write(std::ostream& os) const;
        //@}

    private:
        //! Internal virtual copy constructor.
        virtual AsciiValue* clone_() const;

    }; // class AsciiValue

    /*!
      @brief %Value for an Exif comment.

      This can be a plain Ascii string or a multipe byte encoded string. The
      comment is expected to be encoded in the character set indicated (default
      undefined), but this is not checked. It is left to caller to decode and
      encode the string to and from readable text if that is required.
    */
    class CommentValue : public StringValueBase {
    public:
        //! Character set identifiers for the character sets defined by %Exif
        enum CharsetId { ascii, jis, unicode, undefined,
                         invalidCharsetId, lastCharsetId };
        //! Information pertaining to the defined character sets
        struct CharsetTable {
            //! Constructor
            CharsetTable(CharsetId charsetId,
                         const char* name,
                         const char* code);
            CharsetId charsetId_;                   //!< Charset id
            const char* name_;                      //!< Name of the charset
            const char* code_;                      //!< Code of the charset
        }; // struct CharsetTable
        //! Charset information lookup functions. Implemented as a static class.
        class CharsetInfo {
            //! Prevent construction: not implemented.
            CharsetInfo() {}
            //! Prevent copy-construction: not implemented.
            CharsetInfo(const CharsetInfo&);
            //! Prevent assignment: not implemented.
            CharsetInfo& operator=(const CharsetInfo&);

        public:
            //! Return the name for a charset id
            static const char* name(CharsetId charsetId);
            //! Return the code for a charset id
            static const char* code(CharsetId charsetId);
            //! Return the charset id for a name
            static CharsetId charsetIdByName(const std::string& name);
            //! Return the charset id for a code
            static CharsetId charsetIdByCode(const std::string& code);

        private:
            static const CharsetTable charsetTable_[];
        }; // class CharsetInfo

        //! Shortcut for a %CommentValue auto pointer.
        typedef std::auto_ptr<CommentValue> AutoPtr;

        //! @name Creators
        //@{
        //! Default constructor.
        CommentValue()
            : StringValueBase(Exiv2::undefined) {}
        //! Constructor, uses read(const std::string& comment)
        explicit CommentValue(const std::string& comment);
        //! Virtual destructor.
        virtual ~CommentValue() {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the value from a comment

          The format of \em comment is:
          <BR>
          <CODE>[charset=["]Ascii|Jis|Unicode|Undefined["] ]comment</CODE>
          <BR>
          The default charset is Undefined.

          @return 0 if successful<BR>
                  1 if an invalid character set is encountered
        */
        int read(const std::string& comment);
        //@}

        //! @name Accessors
        //@{
        AutoPtr clone() const { return AutoPtr(clone_()); }
        /*!
          @brief Write the comment in a format which can be read by
          read(const std::string& comment).
         */
        std::ostream& write(std::ostream& os) const;
        //! Return the comment (without a charset="..." prefix)
        std::string comment() const;
        //! Return the charset id of the comment
        CharsetId charsetId() const;
        //@}

    private:
        //! Internal virtual copy constructor.
        virtual CommentValue* clone_() const;

    }; // class CommentValue

    /*!
      @brief Base class for all Exiv2 values used to store XMP property values.
     */
    class XmpValue : public Value {
    public:
        //! Shortcut for a %XmpValue auto pointer.
        typedef std::auto_ptr<XmpValue> AutoPtr;

        //! XMP array types.
        enum XmpArrayType { xaNone, xaAlt, xaBag, xaSeq };
        //! XMP structure indicator.
        enum XmpStruct { xsNone, xsStruct };

        //! @name Creators
        //@{
        explicit XmpValue(TypeId typeId);
        //@}

        //! @name Accessors
        //@{
        //! Return XMP array type, indicates if an XMP value is an array.
        XmpArrayType xmpArrayType() const;
        //! Return XMP struct, indicates if an XMP value is a structure.
        XmpStruct xmpStruct() const;
        virtual long size() const;
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
        virtual long copy(byte* buf, ByteOrder byteOrder =invalidByteOrder) const;
        //@}

        //! @name Manipulators
        //@{
        //! Set the XMP array type to indicate that an XMP value is an array.
        void setXmpArrayType(XmpArrayType xmpArrayType);
        //! Set the XMP struct type to indicate that an XMP value is a structure.
        void setXmpStruct(XmpStruct xmpStruct =xsStruct);
        /*!
          @brief Read the value from a character buffer.

          Uses read(const std::string& buf)

          @note The byte order is required by the interface but not used by this
                method, so just use the default.

          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer
          @param byteOrder Byte order. Not needed.

          @return 0 if successful.
         */
        virtual int read(const byte* buf,
                         long len,
                         ByteOrder byteOrder =invalidByteOrder);
        virtual int read(const std::string& buf) =0;
        //@}

        /*!
          @brief Return XMP array type for an array Value TypeId, xaNone if 
                 \em typeId is not an XMP array value type.
         */
        static XmpArrayType xmpArrayType(TypeId typeId);

    protected:
        /*!
          @brief Assignment operator. Protected so that it can only be used
                 by subclasses but not directly.
         */
        XmpValue& operator=(const XmpValue& rhs);

    private:
        // DATA
        XmpArrayType xmpArrayType_;             //!< Type of XMP array
        XmpStruct    xmpStruct_;                //!< XMP structure indicator

    }; // class XmpValue

    /*!
      @brief %Value type suitable for simple XMP properties and 
             XMP nodes of complex types which are not parsed into
             specific values.

      Uses a std::string to store the value.
     */
    class XmpTextValue : public XmpValue {
    public:
        //! Shortcut for a %XmpTextValue auto pointer.
        typedef std::auto_ptr<XmpTextValue> AutoPtr;

        //! @name Creators
        //@{
        //! Constructor.
        XmpTextValue();
        //! Constructor, reads the value from a string.
        explicit XmpTextValue(const std::string& buf);
        //@}

        //! @name Manipulators
        //@{
        virtual int read(const std::string& buf);
        //@}

        //! @name Accessors
        //@{
        AutoPtr clone() const;
        long size() const;
        virtual long count() const;
        /*!
          @brief Convert the value to a long.
                 The optional parameter \em n is not used and is ignored.

          @return The converted value.
         */
        virtual long toLong(long n =0) const;
        /*!
          @brief Convert the value to a float.
                 The optional parameter \em n is not used and is ignored.

          @return The converted value.
         */
        virtual float toFloat(long n =0) const;
        /*!
          @brief Convert the value to a Rational.
                 The optional parameter \em n is not used and is ignored.

          @return The converted value.
         */
        virtual Rational toRational(long n =0) const;
        virtual std::ostream& write(std::ostream& os) const;
        //@}

    private:
        //! Internal virtual copy constructor.
        virtual XmpTextValue* clone_() const;

    public:
        // DATA
        std::string value_;                     //!< Stores the string values.

    }; // class XmpTextValue

    /*!
      @brief %Value type for simple arrays. Each item in the array is a simple
             value, without qualifiers. The array may be an ordered (\em seq), 
             unordered (\em bag) or alternative array (\em alt). The array 
             items must not contain qualifiers. For language alternatives use
             LangAltValue.

      Uses a vector of std::string to store the value(s).
     */
    class XmpArrayValue : public XmpValue {
    public:
        //! Shortcut for a %XmpArrayValue auto pointer.
        typedef std::auto_ptr<XmpArrayValue> AutoPtr;

        //! @name Creators
        //@{
        //! Constructor. \em typeId can be one of xmpBag, xmpSeq or xmpAlt.
        explicit XmpArrayValue(TypeId typeId =xmpBag);
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read a simple property value from \em buf and append it 
                 to the value.

          Appends \em buf to the value after the last existing array element.
          Subsequent calls will therefore populate multiple array elements in 
          the order they are read.

          @return 0 if successful.
         */
        virtual int read(const std::string& buf);
        //@}

        //! @name Accessors
        //@{
        AutoPtr clone() const;
        virtual long count() const;
        /*!
          @brief Return the <EM>n</EM>-th component of the value as a string.
                 The behaviour of this method may be undefined if there is no
                 <EM>n</EM>-th component.
         */
        virtual std::string toString(long n) const;
        virtual long toLong(long n =0) const;
        virtual float toFloat(long n =0) const;
        virtual Rational toRational(long n =0) const;
        /*!
          @brief Write all elements of the value to \em os, separated by commas.

          @note The output of this method cannot directly be used as the parameter
                for read().
         */
        virtual std::ostream& write(std::ostream& os) const;
        //@}

    private:
        //! Internal virtual copy constructor.
        virtual XmpArrayValue* clone_() const;

    public:
        //! Type used to store XMP array elements.
        typedef std::vector<std::string> ValueType;
        // DATA
        std::vector<std::string> value_;        //!< Stores the string values.

    }; // class XmpArrayValue

    /*!
      @brief %Value type for XMP language alternative properties. 

      A language alternative is an array consisting of simple text values,
      each of which has a language qualifier.
     */
    class LangAltValue : public XmpValue {
    public:
        //! Shortcut for a %LangAltValue auto pointer.
        typedef std::auto_ptr<LangAltValue> AutoPtr;

        //! @name Creators
        //@{
        //! Constructor.
        LangAltValue();
        //! Constructor, reads the value from a string.
        explicit LangAltValue(const std::string& buf);
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read a simple property value from \em buf and append it 
                 to the value.

          Appends \em buf to the value after the last existing array element.
          Subsequent calls will therefore populate multiple array elements in 
          the order they are read.

          The format of \em buf is:
          <BR>
          <CODE>[lang=["]language code["] ]text</CODE>
          <BR>
          The XMP default language code <CODE>x-default</CODE> is used if 
          \em buf doesn't start with the keyword <CODE>lang</CODE>.

          @return 0 if successful.
         */
        virtual int read(const std::string& buf);
        //@}

        //! @name Accessors
        //@{
        AutoPtr clone() const;
        virtual long count() const;
        /*!
          @brief Return the <EM>n</EM>-th component of the value as a string.
                 The behaviour of this method may be undefined if there is no
                 <EM>n</EM>-th component.
         */
        virtual std::string toString(long n) const;
        virtual long toLong(long n =0) const;
        virtual float toFloat(long n =0) const;
        virtual Rational toRational(long n =0) const;
        /*!
          @brief Write all elements of the value to \em os, separated by commas.

          @note The output of this method cannot directly be used as the parameter
                for read().
         */
        virtual std::ostream& write(std::ostream& os) const;
        //@}

    private:
        //! Internal virtual copy constructor.
        virtual LangAltValue* clone_() const;

    public:
        //! Type used to store language alternative arrays.
        typedef std::map<std::string, std::string> ValueType;
        // DATA
        /*!
          @brief Map to store the language alternative values. The language
                 qualifier is used as the key for the map entries.
         */
        ValueType value_; 

    }; // class LangAltValue

    /*!
      @brief %Value for simple ISO 8601 dates

      This class is limited to parsing simple date strings in the ISO 8601
      format CCYYMMDD (century, year, month, day).
     */
    class DateValue : public Value {
    public:
        //! Shortcut for a %DateValue auto pointer.
        typedef std::auto_ptr<DateValue> AutoPtr;

        //! @name Creators
        //@{
        //! Default constructor.
        DateValue();
        //! Constructor
        DateValue(int year, int month, int day);
        //! Virtual destructor.
        virtual ~DateValue() {}
        //@}

        //! Simple Date helper structure
        struct Date
        {
            Date() : year(0), month(0), day(0) {}
            int year;                           //!< Year
            int month;                          //!< Month
            int day;                            //!< Day
        };

        //! @name Manipulators
        //@{
        /*!
          @brief Read the value from a character buffer.

          @note The byte order is required by the interface but not used by this
                method, so just use the default.

          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer
          @param byteOrder Byte order. Not needed.

          @return 0 if successful<BR>
                  1 in case of an unsupported date format
         */
        virtual int read(const byte* buf,
                         long len,
                         ByteOrder byteOrder =invalidByteOrder);
        /*!
          @brief Set the value to that of the string buf.

          @param buf String containing the date

          @return 0 if successful<BR>
                  1 in case of an unsupported date format
         */
        virtual int read(const std::string& buf);
        //! Set the date
        void setDate(const Date& src);
        //@}

        //! @name Accessors
        //@{
        AutoPtr clone() const { return AutoPtr(clone_()); }
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
        virtual long copy(byte* buf, ByteOrder byteOrder =invalidByteOrder) const;
        //! Return date struct containing date information
        virtual const Date& getDate() const { return date_; }
        virtual long count() const { return size(); }
        virtual long size() const;
        virtual std::ostream& write(std::ostream& os) const;
        //! Return the value as a UNIX calender time converted to long.
        virtual long toLong(long n =0) const;
        //! Return the value as a UNIX calender time converted to float.
        virtual float toFloat(long n =0) const
            { return static_cast<float>(toLong(n)); }
        //! Return the value as a UNIX calender time  converted to Rational.
        virtual Rational toRational(long n =0) const
            { return Rational(toLong(n), 1); }
        //@}

    private:
        //! Internal virtual copy constructor.
        virtual DateValue* clone_() const;

        // DATA
        Date date_;

    }; // class DateValue

    /*!
     @brief %Value for simple ISO 8601 times.

     This class is limited to handling simple time strings in the ISO 8601
     format HHMMSS±HHMM where HHMMSS refers to local hour, minute and
     seconds and ±HHMM refers to hours and minutes ahead or behind
     Universal Coordinated Time.
     */
    class TimeValue : public Value {
    public:
        //! Shortcut for a %TimeValue auto pointer.
        typedef std::auto_ptr<TimeValue> AutoPtr;

        //! @name Creators
        //@{
        //! Default constructor.
        TimeValue();
        //! Constructor
        TimeValue(int hour, int minute, int second =0,
                  int tzHour =0, int tzMinute =0);

        //! Virtual destructor.
        virtual ~TimeValue() {}
        //@}

        //! Simple Time helper structure
        struct Time
        {
            Time() : hour(0), minute(0), second(0), tzHour(0), tzMinute(0) {}

            int hour;                           //!< Hour
            int minute;                         //!< Minute
            int second;                         //!< Second
            int tzHour;                         //!< Hours ahead or behind UTC
            int tzMinute;                       //!< Minutes ahead or behind UTC
        };

        //! @name Manipulators
        //@{
        /*!
          @brief Read the value from a character buffer.

          @note The byte order is required by the interface but not used by this
                method, so just use the default.

          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer
          @param byteOrder Byte order. Not needed.

          @return 0 if successful<BR>
                  1 in case of an unsupported time format
         */
        virtual int read(const byte* buf,
                         long len,
                         ByteOrder byteOrder =invalidByteOrder);
        /*!
          @brief Set the value to that of the string buf.

          @param buf String containing the time.

          @return 0 if successful<BR>
                  1 in case of an unsupported time format
         */
        virtual int read(const std::string& buf);
        //! Set the time
        void setTime(const Time& src);
        //@}

        //! @name Accessors
        //@{
        AutoPtr clone() const { return AutoPtr(clone_()); }
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
        virtual long copy(byte* buf, ByteOrder byteOrder =invalidByteOrder) const;
        //! Return time struct containing time information
        virtual const Time& getTime() const { return time_; }
        virtual long count() const { return size(); }
        virtual long size() const;
        virtual std::ostream& write(std::ostream& os) const;
        //! Returns number of seconds in the day in UTC.
        virtual long toLong(long n =0) const;
        //! Returns number of seconds in the day in UTC converted to float.
        virtual float toFloat(long n =0) const
            { return static_cast<float>(toLong(n)); }
        //! Returns number of seconds in the day in UTC converted to Rational.
        virtual Rational toRational(long n =0) const
            { return Rational(toLong(n), 1); }
        //@}

    private:
        //! @name Manipulators
        //@{
        /*!
          @brief Set time from \em buf if it conforms to \em format
                 (3 input items).

          This function only sets the hour, minute and second parts of time_.

          @param buf    A 0 terminated C-string containing the time to parse.
          @param format Format string for sscanf().
          @return 0 if successful, else 1.
         */
        int scanTime3(const char* buf, const char* format);
        /*!
          @brief Set time from \em buf if it conforms to \em format
                 (6 input items).

          This function sets all parts of time_.

          @param buf    A 0 terminated C-string containing the time to parse.
          @param format Format string for sscanf().
          @return 0 if successful, else 1.
         */
        int scanTime6(const char* buf, const char* format);
        //@}

        //! @name Accessors
        //@{
        //! Internal virtual copy constructor.
        virtual TimeValue* clone_() const;
        //@}

        // DATA
        Time time_;

    }; // class TimeValue

    //! Template to determine the TypeId for a type T
    template<typename T> TypeId getType();

    //! Specialization for an unsigned short
    template<> inline TypeId getType<uint16_t>() { return unsignedShort; }
    //! Specialization for an unsigned long
    template<> inline TypeId getType<uint32_t>() { return unsignedLong; }
    //! Specialization for an unsigned rational
    template<> inline TypeId getType<URational>() { return unsignedRational; }
    //! Specialization for a signed short
    template<> inline TypeId getType<int16_t>() { return signedShort; }
    //! Specialization for a signed long
    template<> inline TypeId getType<int32_t>() { return signedLong; }
    //! Specialization for a signed rational
    template<> inline TypeId getType<Rational>() { return signedRational; }

    // No default implementation: let the compiler/linker complain
    // template<typename T> inline TypeId getType() { return invalid; }

    /*!
      @brief Template for a %Value of a basic type. This is used for unsigned
             and signed short, long and rationals.
     */
    template<typename T>
    class ValueType : public Value {
    public:
        //! Shortcut for a %ValueType\<T\> auto pointer.
        typedef std::auto_ptr<ValueType<T> > AutoPtr;

        //! @name Creators
        //@{
        //! Default constructor.
        ValueType() : Value(getType<T>()), pDataArea_(0), sizeDataArea_(0) {}
        //! Constructor
        ValueType(const byte* buf, long len, ByteOrder byteOrder);
        //! Constructor
        explicit ValueType(const T& val, ByteOrder byteOrder =littleEndian);
        //! Copy constructor
        ValueType(const ValueType<T>& rhs);
        //! Virtual destructor.
        virtual ~ValueType();
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator.
        ValueType<T>& operator=(const ValueType<T>& rhs);
        virtual int read(const byte* buf, long len, ByteOrder byteOrder);
        /*!
          @brief Set the data from a string of values of type T (e.g.,
                 "0 1 2 3" or "1/2 1/3 1/4" depending on what T is).
                 Generally, the accepted input format is the same as that
                 produced by the write() method.
         */
        virtual int read(const std::string& buf);
        /*!
          @brief Set the data area. This method copies (clones) the buffer
                 pointed to by buf.
         */
        virtual int setDataArea(const byte* buf, long len);
        //@}

        //! @name Accessors
        //@{
        AutoPtr clone() const { return AutoPtr(clone_()); }
        virtual long copy(byte* buf, ByteOrder byteOrder) const;
        virtual long count() const { return static_cast<long>(value_.size()); }
        virtual long size() const;
        virtual std::ostream& write(std::ostream& os) const;
        /*!
          @brief Return the <EM>n</EM>-th component of the value as a string.
                 The behaviour of this method may be undefined if there is no
                 <EM>n</EM>-th
                 component.
         */
        virtual std::string toString(long n) const;
        virtual long toLong(long n =0) const;
        virtual float toFloat(long n =0) const;
        virtual Rational toRational(long n =0) const;
        //! Return the size of the data area.
        virtual long sizeDataArea() const { return sizeDataArea_; }
        /*!
          @brief Return a copy of the data area in a DataBuf. The caller owns
                 this copy and DataBuf ensures that it will be deleted.
         */
        virtual DataBuf dataArea() const;
        //@}

        //! Container for values
        typedef std::vector<T> ValueList;
        //! Iterator type defined for convenience.
        typedef typename std::vector<T>::iterator iterator;
        //! Const iterator type defined for convenience.
        typedef typename std::vector<T>::const_iterator const_iterator;

        // DATA
        /*!
          @brief The container for all values. In your application, if you know
                 what subclass of Value you're dealing with (and possibly the T)
                 then you can access this STL container through the usual
                 standard library functions.
         */
        ValueList value_;

    private:
        //! Internal virtual copy constructor.
        virtual ValueType<T>* clone_() const;

        // DATA
        //! Pointer to the buffer, 0 if none has been allocated
        byte* pDataArea_;
        //! The current size of the buffer
        long sizeDataArea_;
    }; // class ValueType

    //! Unsigned short value type
    typedef ValueType<uint16_t> UShortValue;
    //! Unsigned long value type
    typedef ValueType<uint32_t> ULongValue;
    //! Unsigned rational value type
    typedef ValueType<URational> URationalValue;
    //! Signed short value type
    typedef ValueType<int16_t> ShortValue;
    //! Signed long value type
    typedef ValueType<int32_t> LongValue;
    //! Signed rational value type
    typedef ValueType<Rational> RationalValue;

// *****************************************************************************
// free functions, template and inline definitions

    /*!
      @brief Read a value of type T from the data buffer.

      We need this template function for the ValueType template classes.
      There are only specializations of this function available; no default
      implementation is provided.

      @param buf Pointer to the data buffer to read from.
      @param byteOrder Applicable byte order (little or big endian).
      @return A value of type T.
     */
    template<typename T> T getValue(const byte* buf, ByteOrder byteOrder);
    // Specialization for a 2 byte unsigned short value.
    template<>
    inline uint16_t getValue(const byte* buf, ByteOrder byteOrder)
    {
        return getUShort(buf, byteOrder);
    }
    // Specialization for a 4 byte unsigned long value.
    template<>
    inline uint32_t getValue(const byte* buf, ByteOrder byteOrder)
    {
        return getULong(buf, byteOrder);
    }
    // Specialization for an 8 byte unsigned rational value.
    template<>
    inline URational getValue(const byte* buf, ByteOrder byteOrder)
    {
        return getURational(buf, byteOrder);
    }
    // Specialization for a 2 byte signed short value.
    template<>
    inline int16_t getValue(const byte* buf, ByteOrder byteOrder)
    {
        return getShort(buf, byteOrder);
    }
    // Specialization for a 4 byte signed long value.
    template<>
    inline int32_t getValue(const byte* buf, ByteOrder byteOrder)
    {
        return getLong(buf, byteOrder);
    }
    // Specialization for an 8 byte signed rational value.
    template<>
    inline Rational getValue(const byte* buf, ByteOrder byteOrder)
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
    template<typename T> long toData(byte* buf, T t, ByteOrder byteOrder);
    /*!
      @brief Specialization to write an unsigned short to the data buffer.
             Return the number of bytes written.
     */
    template<>
    inline long toData(byte* buf, uint16_t t, ByteOrder byteOrder)
    {
        return us2Data(buf, t, byteOrder);
    }
    /*!
      @brief Specialization to write an unsigned long to the data buffer.
             Return the number of bytes written.
     */
    template<>
    inline long toData(byte* buf, uint32_t t, ByteOrder byteOrder)
    {
        return ul2Data(buf, t, byteOrder);
    }
    /*!
      @brief Specialization to write an unsigned rational to the data buffer.
             Return the number of bytes written.
     */
    template<>
    inline long toData(byte* buf, URational t, ByteOrder byteOrder)
    {
        return ur2Data(buf, t, byteOrder);
    }
    /*!
      @brief Specialization to write a signed short to the data buffer.
             Return the number of bytes written.
     */
    template<>
    inline long toData(byte* buf, int16_t t, ByteOrder byteOrder)
    {
        return s2Data(buf, t, byteOrder);
    }
    /*!
      @brief Specialization to write a signed long to the data buffer.
             Return the number of bytes written.
     */
    template<>
    inline long toData(byte* buf, int32_t t, ByteOrder byteOrder)
    {
        return l2Data(buf, t, byteOrder);
    }
    /*!
      @brief Specialization to write a signed rational to the data buffer.
             Return the number of bytes written.
     */
    template<>
    inline long toData(byte* buf, Rational t, ByteOrder byteOrder)
    {
        return r2Data(buf, t, byteOrder);
    }

    template<typename T>
    ValueType<T>::ValueType(const byte* buf, long len, ByteOrder byteOrder)
        : Value(getType<T>()), pDataArea_(0), sizeDataArea_(0)
    {
        read(buf, len, byteOrder);
    }

    template<typename T>
    ValueType<T>::ValueType(const T& val, ByteOrder byteOrder)
        : Value(getType<T>()), pDataArea_(0), sizeDataArea_(0)
    {
        read(reinterpret_cast<const byte*>(&val),
             TypeInfo::typeSize(typeId()),
             byteOrder);
    }

    template<typename T>
    ValueType<T>::ValueType(const ValueType<T>& rhs)
        : Value(rhs), value_(rhs.value_), pDataArea_(0), sizeDataArea_(0)
    {
        if (rhs.sizeDataArea_ > 0) {
            pDataArea_ = new byte[rhs.sizeDataArea_];
            std::memcpy(pDataArea_, rhs.pDataArea_, rhs.sizeDataArea_);
            sizeDataArea_ = rhs.sizeDataArea_;
        }
    }

    template<typename T>
    ValueType<T>::~ValueType()
    {
        delete[] pDataArea_;
    }

    template<typename T>
    ValueType<T>& ValueType<T>::operator=(const ValueType<T>& rhs)
    {
        if (this == &rhs) return *this;
        Value::operator=(rhs);
        value_ = rhs.value_;

        byte* tmp = 0;
        if (rhs.sizeDataArea_ > 0) {
            tmp = new byte[rhs.sizeDataArea_];
            std::memcpy(tmp, rhs.pDataArea_, rhs.sizeDataArea_);
        }
        delete[] pDataArea_;
        pDataArea_ = tmp;
        sizeDataArea_ = rhs.sizeDataArea_;

        return *this;
    }

    template<typename T>
    int ValueType<T>::read(const byte* buf, long len, ByteOrder byteOrder)
    {
        value_.clear();
        for (long i = 0; i < len; i += TypeInfo::typeSize(typeId())) {
            value_.push_back(getValue<T>(buf + i, byteOrder));
        }
        return 0;
    }

    template<typename T>
    int ValueType<T>::read(const std::string& buf)
    {
        std::istringstream is(buf);
        T tmp;
        value_.clear();
        while (!(is.eof())) {
            is >> tmp;
            if (is.fail()) return 1;
            value_.push_back(tmp);
        }
        return 0;
    }

    template<typename T>
    long ValueType<T>::copy(byte* buf, ByteOrder byteOrder) const
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
        return static_cast<long>(TypeInfo::typeSize(typeId()) * value_.size());
    }

    template<typename T>
    ValueType<T>* ValueType<T>::clone_() const
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

    template<typename T>
    inline std::string ValueType<T>::toString(long n) const
    {
        ok_ = true;
        return Exiv2::toString(value_[n]);
    }

    // Default implementation
    template<typename T>
    inline long ValueType<T>::toLong(long n) const
    {
        ok_ = true;
        return value_[n];
    }
    // Specialization for rational
    template<>
    inline long ValueType<Rational>::toLong(long n) const
    {
        ok_ = (value_[n].second != 0);
        return value_[n].first / value_[n].second;
    }
    // Specialization for unsigned rational
    template<>
    inline long ValueType<URational>::toLong(long n) const
    {
        ok_ = (value_[n].second != 0);
        return value_[n].first / value_[n].second;
    }
    // Default implementation
    template<typename T>
    inline float ValueType<T>::toFloat(long n) const
    {
        ok_ = true;
        return static_cast<float>(value_[n]);
    }
    // Specialization for rational
    template<>
    inline float ValueType<Rational>::toFloat(long n) const
    {
        ok_ = (value_[n].second != 0);
        return static_cast<float>(value_[n].first) / value_[n].second;
    }
    // Specialization for unsigned rational
    template<>
    inline float ValueType<URational>::toFloat(long n) const
    {
        ok_ = (value_[n].second != 0);
        return static_cast<float>(value_[n].first) / value_[n].second;
    }
    // Default implementation
    template<typename T>
    inline Rational ValueType<T>::toRational(long n) const
    {
        ok_ = true;
        return Rational(value_[n], 1);
    }
    // Specialization for rational
    template<>
    inline Rational ValueType<Rational>::toRational(long n) const
    {
        ok_ = true;
        return Rational(value_[n].first, value_[n].second);
    }
    // Specialization for unsigned rational
    template<>
    inline Rational ValueType<URational>::toRational(long n) const
    {
        ok_ = true;
        return Rational(value_[n].first, value_[n].second);
    }

    template<typename T>
    inline DataBuf ValueType<T>::dataArea() const
    {
        return DataBuf(pDataArea_, sizeDataArea_);
    }

    template<typename T>
    inline int ValueType<T>::setDataArea(const byte* buf, long len)
    {
        byte* tmp = 0;
        if (len > 0) {
            tmp = new byte[len];
            std::memcpy(tmp, buf, len);
        }
        delete[] pDataArea_;
        pDataArea_ = tmp;
        sizeDataArea_ = len;
        return 0;
    }
}                                       // namespace Exiv2

#endif                                  // #ifndef VALUE_HPP_
