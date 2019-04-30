// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
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
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    09-Jan-04, ahu: created
           11-Feb-04, ahu: isolated as a component
           31-Jul-04, brad: added Time, Data and String values
 */
#pragma once

// *****************************************************************************
#include "exiv2lib_export.h"

// included header files
#include "types.hpp"

// + standard includes
#include <map>
#include <iomanip>
#include <memory>
#include <cstring>
#include <climits>

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
        typedef std::unique_ptr<Value> UniquePtr;

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
        virtual int read(const byte* buf, size_t len, ByteOrder byteOrder) =0;
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
        virtual int setDataArea(const byte* buf, size_t len);
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
        UniquePtr clone() const { return UniquePtr(clone_()); }
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
        virtual size_t sizeDataArea() const;
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
          <TR><TD><B>typeId</B></TD><TD><B>%Value subclass</B></TD></TR>
          <TR><TD><CODE>invalidTypeId</CODE></TD><TD>%DataValue(invalidTypeId)</TD></TR>
          <TR><TD><CODE>unsignedByte</CODE></TD><TD>%DataValue(unsignedByte)</TD></TR>
          <TR><TD><CODE>asciiString</CODE></TD><TD>%AsciiValue</TD></TR>
          <TR><TD><CODE>string</CODE></TD><TD>%StringValue</TD></TR>
          <TR><TD><CODE>unsignedShort</CODE></TD><TD>%ValueType &lt; uint16_t &gt;</TD></TR>
          <TR><TD><CODE>unsignedLong</CODE></TD><TD>%ValueType &lt; uint32_t &gt;</TD></TR>
          <TR><TD><CODE>unsignedRational</CODE></TD><TD>%ValueType &lt; URational &gt;</TD></TR>
          <TR><TD><CODE>invalid6</CODE></TD><TD>%DataValue(invalid6)</TD></TR>
          <TR><TD><CODE>undefined</CODE></TD><TD>%DataValue</TD></TR>
          <TR><TD><CODE>signedShort</CODE></TD><TD>%ValueType &lt; int16_t &gt;</TD></TR>
          <TR><TD><CODE>signedLong</CODE></TD><TD>%ValueType &lt; int32_t &gt;</TD></TR>
          <TR><TD><CODE>signedRational</CODE></TD><TD>%ValueType &lt; Rational &gt;</TD></TR>
          <TR><TD><CODE>tiffFloat</CODE></TD><TD>%ValueType &lt; float &gt;</TD></TR>
          <TR><TD><CODE>tiffDouble</CODE></TD><TD>%ValueType &lt; double &gt;</TD></TR>
          <TR><TD><CODE>tiffIfd</CODE></TD><TD>%ValueType &lt; uint32_t &gt;</TD></TR>
          <TR><TD><CODE>date</CODE></TD><TD>%DateValue</TD></TR>
          <TR><TD><CODE>time</CODE></TD><TD>%TimeValue</TD></TR>
          <TR><TD><CODE>comment</CODE></TD><TD>%CommentValue</TD></TR>
          <TR><TD><CODE>xmpText</CODE></TD><TD>%XmpTextValue</TD></TR>
          <TR><TD><CODE>xmpBag</CODE></TD><TD>%XmpArrayValue</TD></TR>
          <TR><TD><CODE>xmpSeq</CODE></TD><TD>%XmpArrayValue</TD></TR>
          <TR><TD><CODE>xmpAlt</CODE></TD><TD>%XmpArrayValue</TD></TR>
          <TR><TD><CODE>langAlt</CODE></TD><TD>%LangAltValue</TD></TR>
          <TR><TD><EM>default</EM></TD><TD>%DataValue(typeId)</TD></TR>
          </TABLE>

          @param typeId Type of the value.
          @return Auto-pointer to the newly created Value. The caller owns this
                  copy and the auto-pointer ensures that it will be deleted.
         */
        static UniquePtr create(TypeId typeId);

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
        typedef std::unique_ptr<DataValue> UniquePtr;

        explicit DataValue(TypeId typeId =undefined);

        DataValue(const byte* buf, size_t len, ByteOrder byteOrder = invalidByteOrder, TypeId typeId = undefined);

        virtual ~DataValue();

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
        int read(const byte* buf, size_t len, ByteOrder byteOrder = invalidByteOrder) override;

        //! Set the data from a string of integer values (e.g., "0 1 2 3")
        int read(const std::string& buf) override;
        //@}

        //! @name Accessors
        //@{
        UniquePtr clone() const { return UniquePtr(clone_()); }
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
        long copy(byte* buf, ByteOrder byteOrder =invalidByteOrder) const override;
        long count() const override;
        long size() const override;
        std::ostream& write(std::ostream& os) const override;
        /*!
          @brief Return the <EM>n</EM>-th component of the value as a string.
                 The behaviour of this method may be undefined if there is no
                 <EM>n</EM>-th component.
         */
        std::string toString(long n) const override;
        long toLong(long n =0) const override;
        float toFloat(long n =0) const override;
        Rational toRational(long n =0) const override;
        //@}

    private:
        //! Internal virtual copy constructor.
        DataValue* clone_() const override;

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
        typedef std::unique_ptr<StringValueBase> UniquePtr;

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
        int read(const std::string& buf) override;

        /*!
          @brief Read the value from a character buffer.

          @note The byte order is required by the interface but not used by this
                method, so just use the default.

          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer
          @param byteOrder Byte order. Not needed.

          @return 0 if successful.
         */
        int read(const byte* buf, size_t len, ByteOrder byteOrder =invalidByteOrder) override;
        //@}

        //! @name Accessors
        //@{
        UniquePtr clone() const { return UniquePtr(clone_()); }

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
        long copy(byte* buf, ByteOrder byteOrder =invalidByteOrder) const override;
        long count() const override;
        long size() const override;
        long toLong(long n =0) const override;
        float toFloat(long n =0) const override;
        Rational toRational(long n =0) const override;
        std::ostream& write(std::ostream& os) const override;
        //@}

    protected:
        //! Assignment operator.
        StringValueBase& operator=(const StringValueBase& rhs);
        //! Internal virtual copy constructor.
        StringValueBase* clone_() const override =0;

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
        typedef std::unique_ptr<StringValue> UniquePtr;

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
        UniquePtr clone() const { return UniquePtr(clone_()); }
        //@}

    private:
        //! Internal virtual copy constructor.
        StringValue* clone_() const override;

    }; // class StringValue

    /*!
      @brief %Value for an Ascii string type.

      This class is for null terminated single byte Ascii strings.
      This class also ensures that the string is null terminated.
     */
    class EXIV2API AsciiValue : public StringValueBase {
    public:
        //! Shortcut for a %AsciiValue auto pointer.
        typedef std::unique_ptr<AsciiValue> UniquePtr;

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
        int read(const std::string& buf) override;
        //@}

        //! @name Accessors
        //@{
        UniquePtr clone() const { return UniquePtr(clone_()); }
        /*!
          @brief Write the ASCII value up to the the first '\\0' character to an
                 output stream.  Any further characters are ignored and not
                 written to the output stream.
        */
        std::ostream& write(std::ostream& os) const override;
        //@}

    private:
        //! Internal virtual copy constructor.
        AsciiValue* clone_() const override;
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
        enum CharsetId
        {
            ascii,
            jis,
            unicode,
            undefined,
            invalidCharsetId,
            lastCharsetId
        };

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
        public:
            CharsetInfo() = delete;
            CharsetInfo& operator=(const CharsetInfo& rhs) = delete;
            CharsetInfo& operator=(const CharsetInfo&& rhs) = delete;
            CharsetInfo(const CharsetInfo& rhs) = delete;
            CharsetInfo(const CharsetInfo&& rhs) = delete;
 
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
        typedef std::unique_ptr<CommentValue> UniquePtr;

        //! @name Creators
        //@{
        //! Default constructor.
        CommentValue();

        //! Constructor, uses read(const std::string& comment)
        explicit CommentValue(const std::string& comment);

        //! Virtual destructor.
        ~CommentValue() override;
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
        int read(const std::string& comment) override;
        /*!
          @brief Read the comment from a byte buffer.
         */
        int read(const byte* buf, size_t len, ByteOrder byteOrder) override;
        //@}

        //! @name Accessors
        //@{
        UniquePtr clone() const { return UniquePtr(clone_()); }
        long copy(byte* buf, ByteOrder byteOrder) const override;
        /*!
          @brief Write the comment in a format which can be read by
          read(const std::string& comment).
         */
        std::ostream& write(std::ostream& os) const override;
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
        CommentValue* clone_() const override;

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
        typedef std::unique_ptr<XmpValue> UniquePtr;

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
        long size() const override;

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
        long copy(byte* buf, ByteOrder byteOrder =invalidByteOrder) const override;
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
        int read(const byte* buf, size_t len, ByteOrder byteOrder =invalidByteOrder) override;

        int read(const std::string& buf) override =0;
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
        typedef std::unique_ptr<XmpTextValue> UniquePtr;

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
        int read(const std::string& buf) override;
        //@}

        //! @name Accessors
        //@{
        UniquePtr clone() const;
        long size() const override;
        long count() const override;

        /// @brief Convert the value to a long. The optional parameter \em n is not used and is ignored.
        /// @return The converted value.
        long toLong(long n =0) const override;

        /// @brief Convert the value to a float. The optional parameter \em n is not used and is ignored.
        /// @return The converted value.
        float toFloat(long n =0) const override;

        /// @brief Convert the value to a Rational. The optional parameter \em n is not used and is ignored.
        /// @return The converted value.
        Rational toRational(long n =0) const override;
        std::ostream& write(std::ostream& os) const override;
        //@}

    private:
        //! Internal virtual copy constructor.
        XmpTextValue* clone_() const override;

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
        typedef std::unique_ptr<XmpArrayValue> UniquePtr;

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
        int read(const std::string& buf) override;
        //@}

        //! @name Accessors
        //@{
        UniquePtr clone() const;
        long count() const override;

        /*!
          @brief Return the <EM>n</EM>-th component of the value as a string.
                 The behaviour of this method may be undefined if there is no
                 <EM>n</EM>-th component.
         */
        std::string toString(long n) const override;
        long toLong(long n =0) const override;
        float toFloat(long n =0) const override;
        Rational toRational(long n =0) const override;

        /// @brief Write all elements of the value to \em os, separated by commas.
        /// @note The output of this method cannot directly be used as the parameter for read().
        std::ostream& write(std::ostream& os) const override;
        //@}

    private:
        //! Internal virtual copy constructor.
        XmpArrayValue* clone_() const override;

        std::vector<std::string> value_;        //!< Stores the string values.

    }; // class XmpArrayValue

    /*!
      @brief %LangAltValueComparator

      #1058
      https://www.adobe.com/content/dam/Adobe/en/devnet/xmp/pdfs/XMPSpecificationPart1.pdf
      XMP spec chapter B.4 (page 42) the xml:lang qualifier is to be compared case insensitive.
      */
    struct LangAltValueComparator {
        //! LangAltValueComparator comparison case insensitive function
        bool operator() (const std::string& str1, const std::string& str2) const
        {
            int result = str1.size() < str2.size() ?  1
                       : str1.size() > str2.size() ? -1
                       : 0
                       ;
            std::string::const_iterator c1 = str1.begin();
            std::string::const_iterator c2 = str2.begin();
            if (  result==0 ) for (
                ; result==0 && c1 != str1.end()
                ; ++c1, ++c2
                ) {
                result = tolower(*c1) < tolower(*c2) ?  1
                       : tolower(*c1) > tolower(*c2) ? -1
                       : 0
                       ;
            }
            return result < 0 ;
        }
    };

    /*!
      @brief %Value type for XMP language alternative properties.

      A language alternative is an array consisting of simple text values,
      each of which has a language qualifier.
     */
    class EXIV2API LangAltValue : public XmpValue {
    public:
        //! Shortcut for a %LangAltValue auto pointer.
        typedef std::unique_ptr<LangAltValue> UniquePtr;

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
        int read(const std::string& buf) override;
        //@}

        //! @name Accessors
        //@{
        UniquePtr clone() const;
        long count() const override;

        /*!
          @brief Return the text value associated with the default language
                 qualifier \c x-default. The parameter \em n is not used, but
                 it is suggested that only 0 is passed in. Returns an empty
                 string and sets the ok-flag to \c false if there is no
                 default value.
         */
        std::string toString(long n) const override;

        /*!
          @brief Return the text value associated with the language qualifier
                 \em qualifier. Returns an empty string and sets the ok-flag
                 to \c false if there is no entry for the language qualifier.
         */
        std::string toString(const std::string& qualifier) const;
        long toLong(long n =0) const override;
        float toFloat(long n =0) const override;
        Rational toRational(long n =0) const override;

        /// @brief Write all elements of the value to \em os, separated by commas.
        /// @note The output of this method cannot directly be used as the parameter for read().
        std::ostream& write(std::ostream& os) const override;
        //@}

    private:
        //! Internal virtual copy constructor.
        LangAltValue* clone_() const override;

    public:
        //! Type used to store language alternative arrays.
        typedef std::map<std::string, std::string,LangAltValueComparator>  ValueType;
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
        typedef std::unique_ptr<DateValue> UniquePtr;

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
        int read(const byte* buf, size_t len, ByteOrder byteOrder =invalidByteOrder) override;

        /*!
          @brief Set the value to that of the string buf.

          @param buf String containing the date

          @return 0 if successful<BR>
                  1 in case of an unsupported date format
         */
        int read(const std::string& buf) override;

        //! Set the date
        void setDate(const Date& src);
        //@}

        //! @name Accessors
        //@{
        UniquePtr clone() const { return UniquePtr(clone_()); }
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
        long copy(byte* buf, ByteOrder byteOrder =invalidByteOrder) const override;

        //! Return date struct containing date information
        const Date& getDate() const;
        long count() const override;
        long size() const override;
        std::ostream& write(std::ostream& os) const override;

        //! Return the value as a UNIX calender time converted to long.
        long toLong(long n =0) const override;

        //! Return the value as a UNIX calender time converted to float.
        float toFloat(long n =0) const override;

        //! Return the value as a UNIX calender time  converted to Rational.
        Rational toRational(long n =0) const override;
        //@}

    private:
        //! Internal virtual copy constructor.
        DateValue* clone_() const override;

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
    class EXIV2API TimeValue : public Value {
    public:
        //! Shortcut for a %TimeValue auto pointer.
        typedef std::unique_ptr<TimeValue> UniquePtr;

        //! @name Creators
        //@{
        //! Default constructor.
        TimeValue();
        //! Constructor
        TimeValue(int hour, int minute, int second =0,
                  int tzHour =0, int tzMinute =0);

        //! Virtual destructor.
        virtual ~TimeValue();
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
        int read(const byte* buf, size_t len, ByteOrder byteOrder =invalidByteOrder) override;

        /*!
          @brief Set the value to that of the string buf.

          @param buf String containing the time.

          @return 0 if successful<BR>
                  1 in case of an unsupported time format
         */
        int read(const std::string& buf) override;

        //! Set the time
        void setTime(const Time& src);
        //@}

        //! @name Accessors
        //@{
        UniquePtr clone() const { return UniquePtr(clone_()); }
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
        long copy(byte* buf, ByteOrder byteOrder =invalidByteOrder) const override;
        //! Return time struct containing time information
        const Time& getTime() const;
        long count() const override;
        long size() const override;
        std::ostream& write(std::ostream& os) const override;

        //! Returns number of seconds in the day in UTC.
        long toLong(long n =0) const override;

        //! Returns number of seconds in the day in UTC converted to float.
        float toFloat(long n =0) const override;

        //! Returns number of seconds in the day in UTC converted to Rational.
        Rational toRational(long n =0) const override;
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
        TimeValue* clone_() const override;
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
    //! Specialization for a float
    template<> inline TypeId getType<float>() { return tiffFloat; }
    //! Specialization for a double
    template<> inline TypeId getType<double>() { return tiffDouble; }

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
        typedef std::unique_ptr<ValueType<T> > UniquePtr;

        //! @name Creators
        //@{
        //! Default Constructor.
        ValueType();
        //! Constructor.
        // The default c'tor and this one can be combined, but that causes MSVC 7.1 to fall on its nose
        explicit ValueType(TypeId typeId);
        //! Constructor.
        ValueType(const byte* buf, long len, ByteOrder byteOrder, TypeId typeId =getType<T>());
        //! Constructor.
        explicit ValueType(const T& val, TypeId typeId =getType<T>());
        //! Copy constructor
        ValueType(const ValueType<T>& rhs);
        //! Virtual destructor.
        virtual ~ValueType();
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator.
        ValueType<T>& operator=(const ValueType<T>& rhs);
        int read(const byte* buf, size_t len, ByteOrder byteOrder) override;
        /*!
          @brief Set the data from a string of values of type T (e.g.,
                 "0 1 2 3" or "1/2 1/3 1/4" depending on what T is).
                 Generally, the accepted input format is the same as that
                 produced by the write() method.
         */
        int read(const std::string& buf) override;
        /*!
          @brief Set the data area. This method copies (clones) the buffer
                 pointed to by buf.
         */
        int setDataArea(const byte* buf, size_t len) override;
        //@}

        //! @name Accessors
        //@{
        UniquePtr clone() const { return UniquePtr(clone_()); }
        long copy(byte* buf, ByteOrder byteOrder) const override;
        long count() const override;
        long size() const override;
        std::ostream& write(std::ostream& os) const override;
        /*!
          @brief Return the <EM>n</EM>-th component of the value as a string.
                 The behaviour of this method may be undefined if there is no
                 <EM>n</EM>-th
                 component.
         */
        std::string toString(long n) const override;
        long toLong(long n =0) const override;
        float toFloat(long n =0) const override;
        Rational toRational(long n =0) const override;
        //! Return the size of the data area.
        size_t sizeDataArea() const override;
        /*!
          @brief Return a copy of the data area in a DataBuf. The caller owns
                 this copy and DataBuf ensures that it will be deleted.
         */
        DataBuf dataArea() const override;
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
        ValueType<T>* clone_() const override;

        // DATA
        //! Pointer to the buffer, 0 if none has been allocated
        byte* pDataArea_;
        //! The current size of the buffer
        size_t sizeDataArea_;
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
    //! Float value type
    typedef ValueType<float> FloatValue;
    //! Double value type
    typedef ValueType<double> DoubleValue;

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
    // Specialization for a 4 byte float value.
    template<>
    inline float getValue(const byte* buf, ByteOrder byteOrder)
    {
        return getFloat(buf, byteOrder);
    }
    // Specialization for a 8 byte double value.
    template<>
    inline double getValue(const byte* buf, ByteOrder byteOrder)
    {
        return getDouble(buf, byteOrder);
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
    /*!
      @brief Specialization to write a float to the data buffer.
             Return the number of bytes written.
     */
    template<>
    inline long toData(byte* buf, float t, ByteOrder byteOrder)
    {
        return f2Data(buf, t, byteOrder);
    }
    /*!
      @brief Specialization to write a double to the data buffer.
             Return the number of bytes written.
     */
    template<>
    inline long toData(byte* buf, double t, ByteOrder byteOrder)
    {
        return d2Data(buf, t, byteOrder);
    }

    template<typename T>
    ValueType<T>::ValueType()
        : Value(getType<T>()), pDataArea_(0), sizeDataArea_(0)
    {
    }

    template<typename T>
    ValueType<T>::ValueType(TypeId typeId)
        : Value(typeId), pDataArea_(0), sizeDataArea_(0)
    {
    }

    template<typename T>
    ValueType<T>::ValueType(const byte* buf, long len, ByteOrder byteOrder, TypeId typeId)
        : Value(typeId), pDataArea_(0), sizeDataArea_(0)
    {
        read(buf, len, byteOrder);
    }

    template<typename T>
    ValueType<T>::ValueType(const T& val, TypeId typeId)
        : Value(typeId), pDataArea_(0), sizeDataArea_(0)
    {
        value_.push_back(val);
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
    int ValueType<T>::read(const byte* buf, size_t len, ByteOrder byteOrder)
    {
        value_.clear();
        long ts = TypeInfo::typeSize(typeId());
        if (ts != 0)
            if (len % ts != 0) len = (len / ts) * ts;
        for (size_t i = 0; i < len; i += ts) {
            value_.push_back(getValue<T>(buf + i, byteOrder));
        }
        return 0;
    }

    template<typename T>
    int ValueType<T>::read(const std::string& buf)
    {
        std::istringstream is(buf);
        T tmp;
        ValueList val;
        while (!(is.eof())) {
            is >> tmp;
            if (is.fail()) return 1;
            val.push_back(tmp);
        }
        value_.swap(val);
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
    long ValueType<T>::count() const
    {
        return static_cast<long>(value_.size());
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
            os << std::setprecision(15) << *i;
            if (++i != end) os << " ";
        }
        return os;
    }

    template<typename T>
    std::string ValueType<T>::toString(long n) const
    {
        ok_ = true;
        return Exiv2::toString<T>(value_[n]);
    }

    // Default implementation
    template<typename T>
    long ValueType<T>::toLong(long n) const
    {
        ok_ = true;
        return static_cast<long>(value_[n]);
    }
// #55 crash when value_[n].first == LONG_MIN
#define LARGE_INT 1000000
    // Specialization for rational
    template<>
    inline long ValueType<Rational>::toLong(long n) const
    {
        ok_ = (value_[n].second != 0 && INT_MIN < value_[n].first && value_[n].first < INT_MAX );
        if (!ok_) return 0;
        return value_[n].first / value_[n].second;
    }
    // Specialization for unsigned rational
    template<>
    inline long ValueType<URational>::toLong(long n) const
    {
        ok_ = (value_[n].second != 0 && value_[n].first < LARGE_INT);
        if (!ok_) return 0;
        return value_[n].first / value_[n].second;
    }
    // Default implementation
    template<typename T>
    float ValueType<T>::toFloat(long n) const
    {
        ok_ = true;
        return static_cast<float>(value_[n]);
    }
    // Specialization for rational
    template<>
    inline float ValueType<Rational>::toFloat(long n) const
    {
        ok_ = (value_[n].second != 0);
        if (!ok_) return 0.0f;
        return static_cast<float>(value_[n].first) / value_[n].second;
    }
    // Specialization for unsigned rational
    template<>
    inline float ValueType<URational>::toFloat(long n) const
    {
        ok_ = (value_[n].second != 0);
        if (!ok_) return 0.0f;
        return static_cast<float>(value_[n].first) / value_[n].second;
    }
    // Default implementation
    template<typename T>
    Rational ValueType<T>::toRational(long n) const
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
    // Specialization for float.
    template<>
    inline Rational ValueType<float>::toRational(long n) const
    {
        ok_ = true;
        // Warning: This is a very simple conversion, see floatToRationalCast()
        return floatToRationalCast(value_[n]);
    }
    // Specialization for double.
    template<>
    inline Rational ValueType<double>::toRational(long n) const
    {
        ok_ = true;
        // Warning: This is a very simple conversion, see floatToRationalCast()
        return floatToRationalCast(static_cast<float>(value_[n]));
    }

    template<typename T>
    size_t ValueType<T>::sizeDataArea() const
    {
        return sizeDataArea_;
    }

    template<typename T>
    DataBuf ValueType<T>::dataArea() const
    {
        return DataBuf(pDataArea_, sizeDataArea_);
    }

    template<typename T>
    int ValueType<T>::setDataArea(const byte* buf, size_t len)
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
