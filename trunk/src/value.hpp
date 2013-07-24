// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2013 Andreas Huggel <ahuggel@gmx.net>
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
#include <iomanip>
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
    class EXIV2API Value {
    public:
        //! Shortcut for a %Value auto pointer.
        typedef std::auto_ptr<Value> AutoPtr;

        //! @name Creators
        //@{
        //! Constructor, taking a type id to initialize the base class with
        explicit Value(TypeId typeId);
        //! Virtual destructor.
        virtual ~Value();
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
        virtual long sizeDataArea() const;
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
        virtual DataBuf dataArea() const;
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
          <TR><TD class="indexkey">tiffFloat</TD><TD class="indexvalue">%ValueType &lt; float &gt;</TD></TR>
          <TR><TD class="indexkey">tiffDouble</TD><TD class="indexvalue">%ValueType &lt; double &gt;</TD></TR>
          <TR><TD class="indexkey">tiffIfd</TD><TD class="indexvalue">%ValueType &lt; uint32_t &gt;</TD></TR>
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
    class EXIV2API DataValue : public Value {
    public:
        //! Shortcut for a %DataValue auto pointer.
        typedef std::auto_ptr<DataValue> AutoPtr;

        //! @name Creators
        //@{
        //! Default constructor.
        explicit DataValue(TypeId typeId =undefined);
        //! Constructor
        DataValue(const byte* buf,
                  long len, ByteOrder byteOrder =invalidByteOrder,
                  TypeId typeId =undefined);
        //! Virtual destructor.
        virtual ~DataValue();
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
        virtual long count() const;
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
        EXV_DLLLOCAL virtual DataValue* clone_() const;

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
    class EXIV2API StringValueBase : public Value {
    public:
        //! Shortcut for a %StringValueBase auto pointer.
        typedef std::auto_ptr<StringValueBase> AutoPtr;

        //! @name Creators
        //@{
        //! Constructor for subclasses
        explicit StringValueBase(TypeId typeId);
        //! Constructor for subclasses
        StringValueBase(TypeId typeId, const std::string& buf);
        //! Copy constructor
        StringValueBase(const StringValueBase& rhs);
        //! Virtual destructor.
        virtual ~StringValueBase();
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
        virtual long count() const;
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
    class EXIV2API StringValue : public StringValueBase {
    public:
        //! Shortcut for a %StringValue auto pointer.
        typedef std::auto_ptr<StringValue> AutoPtr;

        //! @name Creators
        //@{
        //! Default constructor.
        StringValue();
        //! Constructor
        explicit StringValue(const std::string& buf);
        //! Virtual destructor.
        virtual ~StringValue();
        //@}

        //! @name Accessors
        //@{
        AutoPtr clone() const { return AutoPtr(clone_()); }
        //@}

    private:
        //! Internal virtual copy constructor.
        EXV_DLLLOCAL virtual StringValue* clone_() const;

    }; // class StringValue

    /*!
      @brief %Value for an Ascii string type.

      This class is for null terminated single byte Ascii strings.
      This class also ensures that the string is null terminated.
     */
    class EXIV2API AsciiValue : public StringValueBase {
    public:
        //! Shortcut for a %AsciiValue auto pointer.
        typedef std::auto_ptr<AsciiValue> AutoPtr;

        //! @name Creators
        //@{
        //! Default constructor.
        AsciiValue();
        //! Constructor
        explicit AsciiValue(const std::string& buf);
        //! Virtual destructor.
        virtual ~AsciiValue();
        //@}

        //! @name Manipulators
        //@{
        using StringValueBase::read;
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
        EXV_DLLLOCAL virtual AsciiValue* clone_() const;

    }; // class AsciiValue

    /*!
      @brief %Value for an Exif comment.

      This can be a plain Ascii string or a multipe byte encoded string. The
      comment is expected to be encoded in the character set indicated (default
      undefined), but this is not checked. It is left to caller to decode and
      encode the string to and from readable text if that is required.
    */
    class EXIV2API CommentValue : public StringValueBase {
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
        class EXIV2API CharsetInfo {
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
        CommentValue();
        //! Constructor, uses read(const std::string& comment)
        explicit CommentValue(const std::string& comment);
        //! Virtual destructor.
        virtual ~CommentValue();
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
        /*!
          @brief Read the comment from a byte buffer.
         */
        int read(const byte* buf, long len, ByteOrder byteOrder);
        //@}

        //! @name Accessors
        //@{
        AutoPtr clone() const { return AutoPtr(clone_()); }
        long copy(byte* buf, ByteOrder byteOrder) const;
        /*!
          @brief Write the comment in a format which can be read by
          read(const std::string& comment).
         */
        std::ostream& write(std::ostream& os) const;
        /*!
          @brief Return the comment (without a charset="..." prefix)

          The comment is decoded to UTF-8. For Exif UNICODE comments, the
          function makes an attempt to correctly determine the character
          encoding of the value. Alternatively, the optional \em encoding
          parameter can be used to specify it.

          @param encoding Optional argument to specify the character encoding
              that the comment is encoded in, as an iconv(3) name. Only used
              for Exif UNICODE comments.

          @return A string containing the comment converted to UTF-8.
         */
        std::string comment(const char* encoding =0) const;
        /*!
          @brief Determine the character encoding that was used to encode the
              UNICODE comment value as an iconv(3) name.

          If the comment \em c starts with a BOM, the BOM is interpreted and
          removed from the string.

          Todo: Implement rules to guess if the comment is UTF-8 encoded.
         */
        const char* detectCharset(std::string& c) const;
        //! Return the Exif charset id of the comment
        CharsetId charsetId() const;
        //@}

    private:
        //! Internal virtual copy constructor.
        EXV_DLLLOCAL virtual CommentValue* clone_() const;

    public:
        // DATA
        ByteOrder byteOrder_;      //!< Byte order of the comment string that was read

    }; // class CommentValue

    /*!
      @brief Base class for all Exiv2 values used to store XMP property values.
     */
    class EXIV2API XmpValue : public Value {
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
    class EXIV2API XmpTextValue : public XmpValue {
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
        using XmpValue::read;
        /*!
          @brief Read a simple property value from \em buf to set the value.

          Sets the value to the contents of \em buf. A optional keyword,
          \em type is supported to set the XMP value type. This is useful for
          complex value types for which Exiv2 does not have direct support.

          The format of \em buf is:
          <BR>
          <CODE>[type=["]Alt|Bag|Seq|Struct["] ]text</CODE>
          <BR>

          @return 0 if successful.
         */

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
        EXV_DLLLOCAL virtual XmpTextValue* clone_() const;

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
    class EXIV2API XmpArrayValue : public XmpValue {
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
        using XmpValue::read;
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
        EXV_DLLLOCAL virtual XmpArrayValue* clone_() const;

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
    class EXIV2API LangAltValue : public XmpValue {
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
        using XmpValue::read;
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
          @brief Return the text value associated with the default language
                 qualifier \c x-default. The parameter \em n is not used, but
                 it is suggested that only 0 is passed in. Returns an empty
                 string and sets the ok-flag to \c false if there is no
                 default value.
         */
        virtual std::string toString(long n) const;
        /*!
          @brief Return the text value associated with the language qualifier
                 \em qualifier. Returns an empty string and sets the ok-flag
                 to \c false if there is no entry for the language qualifier.
         */
        std::string toString(const std::string& qualifier) const;
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
        EXV_DLLLOCAL virtual LangAltValue* clone_() const;

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
    class EXIV2API DateValue : public Value {
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
        virtual ~DateValue();
        //@}

        //! Simple Date helper structure
        struct EXIV2API Date {
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
        virtual const Date& getDate() const;
        virtual long count() const;
        virtual long size() const;
        virtual std::ostream& write(std::ostream& os) const;
        //! Return the value as a UNIX calender time converted to long.
        virtual long toLong(long n =0) const;
        //! Return the value as a UNIX calender time converted to float.
        virtual float toFloat(long n =0) const;
        //! Return the value as a UNIX calender time  converted to Rational.
        virtual Rational toRational(long n =0) const;
        //@}

    private:
        //! Internal virtual copy constructor.
        EXV_DLLLOCAL virtual DateValue* clone_() const;

        // DATA
        Date date_;

    }; // class DateValue

    /*!
     @brief %Value for simple ISO 8601 times.

     This class is limited to handling simple time strings in the ISO 8601
