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
  @file    tiffcomposite_int.hpp
  @brief   Internal classes used in a TIFF composite structure
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    11-Apr-06, ahu: created
 */
#ifndef TIFFCOMPOSITE_INT_HPP_
#define TIFFCOMPOSITE_INT_HPP_

// *****************************************************************************
// included header files
#include "image.hpp"                            // for Blob
#include "tifffwd_int.hpp"
#include "types.hpp"

// + standard includes
#include <iosfwd>
#include <vector>
#include <string>
#include <cassert>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

// *****************************************************************************
// class definitions

    //! TIFF value type.
    typedef uint16_t TiffType;

    const TiffType ttUnsignedByte     = 1; //!< Exif BYTE type
    const TiffType ttAsciiString      = 2; //!< Exif ASCII type
    const TiffType ttUnsignedShort    = 3; //!< Exif SHORT type
    const TiffType ttUnsignedLong     = 4; //!< Exif LONG type
    const TiffType ttUnsignedRational = 5; //!< Exif RATIONAL type
    const TiffType ttSignedByte       = 6; //!< Exif SBYTE type
    const TiffType ttUndefined        = 7; //!< Exif UNDEFINED type
    const TiffType ttSignedShort      = 8; //!< Exif SSHORT type
    const TiffType ttSignedLong       = 9; //!< Exif SLONG type
    const TiffType ttSignedRational   =10; //!< Exif SRATIONAL type
    const TiffType ttTiffFloat        =11; //!< TIFF FLOAT type
    const TiffType ttTiffDouble       =12; //!< TIFF DOUBLE type
    const TiffType ttTiffIfd          =13; //!< TIFF IFD type

    //! Convert the \em tiffType of a \em tag and \em group to an Exiv2 \em typeId.
    TypeId toTypeId(TiffType tiffType, uint16_t tag, uint16_t group);
    //! Convert the %Exiv2 \em typeId to a TIFF value type.
    TiffType toTiffType(TypeId typeId);

    /*!
      Known TIFF groups

      Todo: what exactly are these and where should they go?
      Are they going to be mapped to the second part of an Exif key or are they
      the second part of the key?
    */
    namespace Group {
        const uint16_t none    =   0; //!< Dummy group
        const uint16_t ifd0    =   1; //!< Exif IFD0
        const uint16_t ifd1    =   2; //!< Thumbnail IFD
        const uint16_t ifd2    =   3; //!< IFD2
        const uint16_t exif    =   4; //!< Exif IFD
        const uint16_t gps     =   5; //!< GPS IFD
        const uint16_t iop     =   6; //!< Interoperability IFD
        const uint16_t subimg1 =   7; //!< 1st TIFF SubIFD in IFD0
        const uint16_t subimg2 =   8; //!< 2nd TIFF SubIFD in IFD0
        const uint16_t subimg3 =   9; //!< 3rd TIFF SubIFD in IFD0
        const uint16_t subimg4 =  10; //!< 4th TIFF SubIFD in IFD0
        const uint16_t mn      = 256; //!< Makernote
        const uint16_t ignr    = 511; //!< Read but do not decode
    }

    /*!
      Special TIFF tags for the use in TIFF structures only

      Todo: Same Q as above...
    */
    namespace Tag {
        const uint32_t none = 0x10000; //!< Dummy tag
        const uint32_t root = 0x20000; //!< Special tag: root IFD
        const uint32_t next = 0x30000; //!< Special tag: next IFD
        const uint32_t all  = 0x40000; //!< Special tag: all tags in a group
    }

    /*!
      @brief Interface class for components of a TIFF directory hierarchy
             (Composite pattern).  Both TIFF directories as well as entries
             implement this interface.  A component can be uniquely identified
             by a tag, group tupel.  This class is implemented as a NVI
             (Non-Virtual Interface) and it has an interface for visitors
             (Visitor pattern) to perform operations on all components.
     */
    class TiffComponent {
    public:
        //! TiffComponent auto_ptr type
        typedef std::auto_ptr<TiffComponent> AutoPtr;
        //! Container type to hold all metadata
        typedef std::vector<TiffComponent*> Components;

        //! @name Creators
        //@{
        //! Constructor
        TiffComponent(uint16_t tag, uint16_t group);
        //! Virtual destructor.
        virtual ~TiffComponent() {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Add a TIFF entry \em tag to the component. Components on
                 the path to the entry are added if they don't exist yet.

          @param tag      The tag of the new entry
          @param tiffPath A path from the TIFF root element to a TIFF entry.

          @return A pointer to the newly added TIFF entry.
         */
        TiffComponent* addPath(uint16_t tag, TiffPath& tiffPath);
        /*!
          @brief Add a child to the component. Default is to do nothing.
          @param tiffComponent Auto pointer to the component to add.
          @return Return a pointer to the newly added child element or 0.
         */
        TiffComponent* addChild(AutoPtr tiffComponent);
        /*!
            @brief Add a "next" component to the component. Default is to do
                   nothing.
            @param tiffComponent Auto pointer to the component to add.
            @return Return a pointer to the newly added "next" element or 0.
         */
        TiffComponent* addNext(AutoPtr tiffComponent);
        /*!
          @brief Interface to accept visitors (Visitor pattern). Visitors
                 can perform operations on all components of the composite.

          @param visitor The visitor.
         */
        void accept(TiffVisitor& visitor);
        /*!
          @brief Set a pointer to the start of the binary representation of the
                 component in a memory buffer. The buffer must be allocated and
                 freed outside of this class.
         */
        void setStart(const byte* pStart) { pStart_ = const_cast<byte*>(pStart); }
        //@}

        //! @name Accessors
        //@{
        //! Return the tag of this entry.
        uint16_t tag()                        const { return tag_; }
        //! Return the group id of this component
        uint16_t group()                      const { return group_; }
        //! Return a pointer to the start of the binary representation of the component
        byte* start()                         const { return pStart_; }
        //@}

        //! @name Write support (Manipulators)
        //@{
        /*!
          @brief Write a TiffComponent to a binary image.

          @param blob       Binary image to append the TiffComponent to.
          @param byteOrder  Applicable byte order (little or big endian).
          @param offset     Offset from the start of the image (TIFF header) to
                            the component.
          @param valueIdx   Index of the component to be written relative to offset.
          @param dataIdx    Index of the data area of the component relative to offset.
          @param imageIdx   Index of the image data area relative to offset.
          @return           Number of bytes written to the blob including all
                            nested components.
          @throw            Error If the component cannot be written.
         */
        uint32_t write(Blob&     blob,
                       ByteOrder byteOrder,
                       int32_t   offset,
                       uint32_t  valueIdx,
                       uint32_t  dataIdx,
                       uint32_t& imageIdx);
        //@}

        //! @name Write support (Accessors)
        //@{
        /*!
          @brief Write the IFD data of this component to a binary image.
                 Return the number of bytes written. Components derived from
                 TiffEntryBase implement this method if needed.
         */
        uint32_t writeData(Blob&     blob,
                           ByteOrder byteOrder,
                           int32_t   offset,
                           uint32_t  dataIdx,
                           uint32_t& imageIdx) const;
        /*!
          @brief Write the image data of this component to a binary image.
                 Return the number of bytes written. TIFF components implement
                 this method if needed.
         */
        uint32_t writeImage(Blob&     blob,
                            ByteOrder byteOrder) const;
        /*!
          @brief Return the size in bytes of the IFD value of this component
                 when written to a binary image.
         */
        uint32_t size() const;
        /*!
          @brief Return the number of components in this component.
         */
        uint32_t count() const;
        /*!
          @brief Return the size in bytes of the IFD data of this component when
                 written to a binary image.  This is a support function for
                 write(). Components derived from TiffEntryBase implement this
                 method corresponding to their implementation of writeData().
         */
        uint32_t sizeData() const;
        /*!
          @brief Return the size in bytes of the image data of this component
                 when written to a binary image.  This is a support function for
                 write(). TIFF components implement this method corresponding to
                 their implementation of writeImage().
         */
        uint32_t sizeImage() const;
        //@}

    protected:
        //! @name Manipulators
        //@{
        //! Implements addPath(). The default implementation does nothing.
        virtual TiffComponent* doAddPath(uint16_t  /*tag*/,
                                         TiffPath& /*tiffPath*/) { return this; }
        //! Implements addChild(). The default implementation does nothing.
        virtual TiffComponent* doAddChild(AutoPtr /*tiffComponent*/) { return 0; }
        //! Implements addNext(). The default implementation does nothing.
        virtual TiffComponent* doAddNext(AutoPtr /*tiffComponent*/) { return 0; }
        //! Implements accept().
        virtual void doAccept(TiffVisitor& visitor) =0;
        //@}

        //! @name Write support (Manipulators)
        //@{
        //! Implements write().
        virtual uint32_t doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx) =0;
        //@}

        //! @name Write support (Accessors)
        //@{
        //! Implements writeData().
        virtual uint32_t doWriteData(Blob&     blob,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const =0;
        //! Implements writeImage().
        virtual uint32_t doWriteImage(Blob&     blob,
                                      ByteOrder byteOrder) const =0;
        //! Implements size().
        virtual uint32_t doSize() const =0;
        //! Implements count().
        virtual uint32_t doCount() const =0;
        //! Implements sizeData().
        virtual uint32_t doSizeData() const =0;
        //! Implements sizeImage().
        virtual uint32_t doSizeImage() const =0;
        //@}

    private:
        // DATA
        uint16_t tag_;      //!< Tag that identifies the component
        uint16_t group_;    //!< Group id for this component
        /*!
          Pointer to the start of the binary representation of the component in
          a memory buffer. The buffer is allocated and freed outside of this class.
         */
        byte*    pStart_;

    }; // class TiffComponent

    /*!
      @brief Data structure used as a row (element) of a table (array)
             describing the TIFF structure of an image format for reading and
             writing.  Different tables can be used to support different TIFF
             based image formats.
     */
    struct TiffStructure {
        struct Key;
        //! Comparison operator to compare a TiffStructure with a TiffStructure::Key
        bool operator==(const Key& key) const;
        //! Return the tag corresponding to the extended tag
        uint16_t tag() const { return static_cast<uint16_t>(extendedTag_ & 0xffff); }

        // DATA
        uint32_t       extendedTag_;    //!< Tag (32 bit so that it can contain special tags)
        uint16_t       group_;          //!< Group that contains the tag
        uint16_t       newGroup_;       //!< Group of the newly created component
        uint32_t       parentExtTag_;   //!< Parent tag (32 bit so that it can contain special tags)
        uint16_t       parentGroup_;    //!< Parent group
        NewTiffCompFct newTiffCompFct_; //!< Function to create the correct TIFF component
    };

    //! Search key for TIFF structure.
    struct TiffStructure::Key {
        //! Constructor
        Key(uint32_t e, uint16_t g) : e_(e), g_(g) {}
        uint32_t e_;                    //!< Extended tag
        uint16_t g_;                    //!< %Group
    };

    //! TIFF mapping table for functions to decode special cases
    struct TiffMappingInfo {
        struct Key;
        /*!
          @brief Compare a TiffMappingInfo with a TiffMappingInfo::Key.
                 The two are equal if TiffMappingInfo::make_ equals a substring
                 of the key of the same size. E.g., mapping info = "OLYMPUS",
                 key = "OLYMPUS OPTICAL CO.,LTD" (found in the image) match,
                 the extendedTag is Tag::all or equal to the extended tag of the
                 key, and the group is equal to that of the key.
         */
        bool operator==(const Key& key) const;
        //! Return the tag corresponding to the extended tag
        uint16_t tag() const { return static_cast<uint16_t>(extendedTag_ & 0xffff); }

        // DATA
        const char* make_;        //!< Camera make for which these mapping functions apply
        uint32_t    extendedTag_; //!< Tag (32 bit so that it can contain special tags)
        uint16_t    group_;       //!< Group that contains the tag
        DecoderFct  decoderFct_;  //!< Decoder function for matching tags
        EncoderFct  encoderFct_;  //!< Encoder function for matching tags

    }; // struct TiffMappingInfo

    //! Search key for TIFF mapping structures.
    struct TiffMappingInfo::Key {
        //! Constructor
        Key(const std::string& m, uint32_t e, uint16_t g) : m_(m), e_(e), g_(g) {}
        std::string m_;                    //!< Camera make
        uint32_t    e_;                    //!< Extended tag
        uint16_t    g_;                    //!< %Group
    };

    /*!
      @brief This abstract base class provides the common functionality of an
             IFD directory entry and defines an extended interface for derived
             concrete entries, which allows access to the attributes of the
             entry.
     */
    class TiffEntryBase : public TiffComponent {
        friend class TiffReader;
        friend class TiffEncoder;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffEntryBase(uint16_t tag, uint16_t group, TiffType tiffType =ttUndefined);
        //! Virtual destructor.
        virtual ~TiffEntryBase();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Encode a TIFF component from the metadatum provided and
                 information from the \em encoder as needed.

          Implemented as double-dispatch calls back to one of the specific
          encoding functions at the \em encoder.
         */
        void encode(TiffEncoder& encoder, const Exifdatum* datum);
        //! Set the offset
        void setOffset(int32_t offset) { offset_ = offset; }
        //! Set pointer and size of the entry's data.
        void setData(byte* pData, int32_t size);
        /*!
          @brief Update the value. Takes ownership of the pointer passed in.

          Update binary value data and call setValue().
        */
        void updateValue(Value::AutoPtr value, ByteOrder byteOrder);
        /*!
          @brief Set tag value. Takes ownership of the pointer passed in.

          Update type, count and the pointer to the value.
        */
        void setValue(Value::AutoPtr value);
        //@}

        //! @name Accessors
        //@{
        //! Return the TIFF type
        TiffType tiffType()      const { return tiffType_; }
        /*!
          @brief Return the offset to the data area relative to the base
                 for the component (usually the start of the TIFF header)
         */
        int32_t offset()         const { return offset_; }
        /*!
          @brief Return a pointer to the binary representation of the
                 value of this component.
         */
        const byte* pData()      const { return pData_; }
        //! Return a const pointer to the converted value of this component
        const Value* pValue()    const { return pValue_; }
        //@}

    protected:
        //! @name Manipulators
        //@{
        //! Implements encode().
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum) =0;
        //! Set the number of components in this entry
        void setCount(uint32_t count) { count_ = count; }
        //@}

        //! @name Accessors
        //@{
        //! Implements count().
        virtual uint32_t doCount() const;
        //@}
        //! @name Write support (Manipulators)
        //@{
        /*!
          @brief Implements write(). Write the value of a standard TIFF entry to
                 the \em blob, return the number of bytes written. Only the \em
                 blob and \em byteOrder arguments are used.
         */
        virtual uint32_t doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}
        //! @name Write support (Accessors)
        //@{
        /*!
          @brief Implements writeData(). Standard TIFF entries have no data:
                 write nothing and return 0.
         */
        virtual uint32_t doWriteData(Blob&     blob,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const;
        /*!
          @brief Implements writeImage(). Standard TIFF entries have no image data:
                 write nothing and return 0.
         */
        virtual uint32_t doWriteImage(Blob&     blob,
                                      ByteOrder byteOrder) const;
        //! Implements size(). Return the size of a standard TIFF entry
        virtual uint32_t doSize() const;
        //! Implements sizeData(). Return 0.
        virtual uint32_t doSizeData() const;
        //! Implements sizeImage(). Return 0.
        virtual uint32_t doSizeImage() const;
        //@}

        //! Helper function to write an \em offset to a preallocated binary buffer
        static uint32_t writeOffset(byte*     buf,
                                    int32_t   offset,
                                    TiffType  tiffType,
                                    ByteOrder byteOrder);

    private:
        //! @name Manipulators
        //@{
        //! Allocate \em len bytes for the binary representation of the value.
        void allocData(uint32_t len);
        //@}

        // DATA
        TiffType tiffType_;   //!< Field TIFF type
        uint32_t count_;      //!< The number of values of the indicated type
        int32_t  offset_;     //!< Offset to the data area
        /*!
          Size of the data buffer holding the value in bytes, there is no
          minimum size.
         */
        uint32_t size_;
        byte*    pData_;      //!< Pointer to the data area
        bool     isMalloced_; //!< True if this entry owns the value data
        Value*   pValue_;     //!< Converted data value

    }; // class TiffEntryBase

    /*!
      @brief A standard TIFF IFD entry.
     */
    class TiffEntry : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffEntry(uint16_t tag, uint16_t group) : TiffEntryBase(tag, group) {}
        //! Virtual destructor.
        virtual ~TiffEntry() {}
        //@}

    protected:
        //! @name Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        //@}

    }; // class TiffEntry

    /*!
      @brief Interface for a standard TIFF IFD entry consisting of a value
             which is a set of offsets to a data area. The sizes of these "strips"
             are provided in a related TiffSizeEntry, tag and group of which are
             set in the constructor. The implementations of this interface differ
             in whether the data areas are extracted to the higher level metadata
             (TiffDataEntry) or not (TiffImageEntry).
     */
    class TiffDataEntryBase : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffDataEntryBase(uint16_t tag, uint16_t group, uint16_t szTag, uint16_t szGroup)
            : TiffEntryBase(tag, group),
              szTag_(szTag), szGroup_(szGroup) {}
        //! Virtual destructor.
        virtual ~TiffDataEntryBase() {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Set the data areas ("strips").

          @param pSize Pointer to the Value holding the sizes corresponding
                       to this data entry.
          @param pData Pointer to the data area.
          @param sizeData Size of the data area.
          @param baseOffset Base offset into the data area.
         */
        virtual void setStrips(const Value* pSize,
                               const byte*  pData,
                               uint32_t     sizeData,
                               uint32_t     baseOffset) =0;
        //@}

        //! @name Accessors
        //@{
        //! Return the group of the entry which has the size
        uint16_t szTag() const { return szTag_; }
        //! Return the group of the entry which has the size
        uint16_t szGroup() const { return szGroup_; }
        //@}

    private:
        // DATA
        const uint16_t szTag_;               //!< Tag of the entry with the size
        const uint16_t szGroup_;             //!< Group of the entry with the size

    }; // class TiffDataEntryBase

    /*!
      @brief A standard TIFF IFD entry consisting of a value which is an offset
             to a data area and the data area. The size of the data area is
             provided in a related TiffSizeEntry, tag and group of which are set
             in the constructor.

             This component extracts the data areas ("strips") and makes them
             available in the higher level metadata. It is used, e.g., for
             \em Exif.Thumbnail.JPEGInterchangeFormat for which the size
             is provided in \em Exif.Thumbnail.JPEGInterchangeFormatLength.
     */
    class TiffDataEntry : public TiffDataEntryBase {
        friend class TiffEncoder;
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffDataEntry(uint16_t tag, uint16_t group, uint16_t szTag, uint16_t szGroup)
            : TiffDataEntryBase(tag, group, szTag, szGroup),
              pDataArea_(0), sizeDataArea_(0) {}
        //! Virtual destructor.
        virtual ~TiffDataEntry() {}
        //@}

        //! @name Manipulators
        //@{
        virtual void setStrips(const Value* pSize,
                               const byte*  pData,
                               uint32_t     sizeData,
                               uint32_t     baseOffset);
        //@}

    protected:
        //! @name Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        //@}

        //! @name Write support (Manipulators)
        //@{
        /*!
          @brief Implements write(). Write pointers into the data area to the
                 \em blob, relative to the offsets in the value. Return the
                 number of bytes written. The \em valueIdx argument is not used.

          The number of components in the value determines how many offsets are
          written. Set the first value to 0, the second to the size of the first
          data area, etc. when creating a new data entry. Offsets will be adjusted
          on write. The type of the value can only be signed or unsigned short or
          long.
         */
        virtual uint32_t doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}
        //! @name Write support (Accessors)
        //@{
        /*!
          @brief Implements writeData(). Write the data area to the blob. Return
                 the number of bytes written.
         */
        virtual uint32_t doWriteData(Blob&     blob,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const;
        // Using doWriteImage from base class
        // Using doSize() from base class
        //! Implements sizeData(). Return the size of the data area.
        virtual uint32_t doSizeData() const;
        // Using doSizeImage from base class
        //@}

    private:
        // DATA
        byte*          pDataArea_;           //!< Pointer to the data area (never alloc'd)
        uint32_t       sizeDataArea_;        //!< Size of the data area

    }; // class TiffDataEntry

    /*!
      @brief A standard TIFF IFD entry consisting of a value which is an array
             of offsets to image data areas. The sizes of the image data areas are
             provided in a related TiffSizeEntry, tag and group of which are set
             in the constructor.

             The data is not extracted into the higher level metadata tags, it is
             only copied to the target image when the image is written.
             This component is used, e.g., for
             \em Exif.Image.StripOffsets for which the sizes are provided in
             \em Exif.Image.StripByteCounts.
     */
    class TiffImageEntry : public TiffDataEntryBase {
        friend class TiffEncoder;
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffImageEntry(uint16_t tag, uint16_t group, uint16_t szTag, uint16_t szGroup)
            : TiffDataEntryBase(tag, group, szTag, szGroup) {}
        //! Virtual destructor.
        virtual ~TiffImageEntry() {}
        //@}

        //! @name Manipulators
        //@{
        virtual void setStrips(const Value* pSize,
                               const byte*  pData,
                               uint32_t     sizeData,
                               uint32_t     baseOffset);
        //@}

    protected:
        //! @name Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        //@}

        //! @name Write support (Manipulators)
        //@{
        /*!
          @brief Implements write(). Write pointers into the image data area to the
                 \em blob. Return the number of bytes written. The \em valueIdx
                 and \em dataIdx  arguments are not used.
         */
        virtual uint32_t doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}
        //! @name Write support (Accessors)
        //@{
        // Using doWriteData from base class
        /*!
          @brief Implements writeImage(). Write the image data area to the blob.
                 Return the number of bytes written.
         */
        virtual uint32_t doWriteImage(Blob&     blob,
                                      ByteOrder byteOrder) const;
        //! Implements size(). Return the size of the strip pointers.
        virtual uint32_t doSize() const;
        // Using doSizeData from base class
        //! Implements sizeImage(). Return the size of the image data area.
        virtual uint32_t doSizeImage() const;
        //@}

    private:
        //! Pointers to the image data (strips) and their sizes.
        typedef std::vector<std::pair<const byte*, uint32_t> > Strips;

        // DATA
        Strips   strips_;       //!< Image strips data (never alloc'd) and sizes

    }; // class TiffImageEntry

    /*!
      @brief A TIFF IFD entry containing the size of a data area of a related
             TiffDataEntry. This component is used, e.g. for
             \em Exif.Thumbnail.JPEGInterchangeFormatLength, which contains the
             size of \em Exif.Thumbnail.JPEGInterchangeFormat.
     */
    class TiffSizeEntry : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffSizeEntry(uint16_t tag, uint16_t group, uint16_t dtTag, uint16_t dtGroup)
            : TiffEntryBase(tag, group), dtTag_(dtTag), dtGroup_(dtGroup) {}
        //! Virtual destructor.
        virtual ~TiffSizeEntry() {}
        //@}

        //! @name Accessors
        //@{
        //! Return the group of the related entry which has the data area
        uint16_t dtTag() const { return dtTag_; }
        //! Return the group of the related entry which has the data area
        uint16_t dtGroup() const { return dtGroup_; }
        //@}

    protected:
        //! @name Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        //@}

    private:
        // DATA
        const uint16_t dtTag_;        //!< Tag of the entry with the data area
        const uint16_t dtGroup_;      //!< Group of the entry with the data area

    }; // class TiffSizeEntry

    /*!
      @brief This class models a TIFF directory (%Ifd). It is a composite
             component of the TIFF tree.
     */
    class TiffDirectory : public TiffComponent {
        friend class TiffEncoder;
        friend class TiffPrinter;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffDirectory(uint16_t tag, uint16_t group, bool hasNext =true)
            : TiffComponent(tag, group), hasNext_(hasNext), pNext_(0) {}
        //! Virtual destructor
        virtual ~TiffDirectory();
        //@}

        //! @name Accessors
        //@{
        //! Return true if the directory has a next pointer
        bool hasNext() const { return hasNext_; }
        //@}

    protected:
        //! @name Manipulators
        //@{
        virtual TiffComponent* doAddPath(uint16_t tag, TiffPath& tiffPath);
        virtual TiffComponent* doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual TiffComponent* doAddNext(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        //@}

        //! @name Write support (Manipulators)
        //@{
        /*!
          @brief Implements write(). Write the TIFF directory, values and
                 additional data, including the next-IFD, if any, to the blob,
                 return the number of bytes written.
         */
        virtual uint32_t doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}
        //! @name Write support (Accessors)
        //@{
        /*!
          @brief This class does not really implement writeData(), it only has
                 write(). This method must not be called; it commits suicide.
         */
        virtual uint32_t doWriteData(Blob&     blob,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const;
        /*!
          @brief Implements writeImage(). Write the image data of the TIFF
                 directory to the blob by forwarding the call to each component
                 as well as the next-IFD, if there is any. Return the number of
                 bytes written.
         */
        virtual uint32_t doWriteImage(Blob&     blob,
                                      ByteOrder byteOrder) const;
        /*!
          @brief Implements size(). Return the size of the TIFF directory,
                 values and additional data, including the next-IFD, if any.
         */
        virtual uint32_t doSize() const;
        /*!
          @brief Implements count(). Return the number of entries in the TIFF
                 directory. Does not count entries which are marked as deleted.
         */
        virtual uint32_t doCount() const;
        /*!
          @brief This class does not really implement sizeData(), it only has
                 size(). This method must not be called; it commits suicide.
         */
        virtual uint32_t doSizeData() const;
        /*!
          @brief Implements sizeImage(). Return the sum of the image sizes of 
                 all components plus that of the next-IFD, if there is any.
         */
        virtual uint32_t doSizeImage() const;
        //@}

    private:
        //! @name Accessors
        //@{
        //! Write a binary directory entry for a TIFF component.
        uint32_t writeDirEntry(Blob&          blob,
                               ByteOrder      byteOrder,
                               int32_t        offset,
                               TiffComponent* pTiffComponent,
                               uint32_t       valueIdx,
                               uint32_t       dataIdx,
                               uint32_t&      imageIdx) const;
        //@}

    private:
        // DATA
        Components components_; //!< List of components in this directory
        const bool hasNext_;    //!< True if the directory has a next pointer
        TiffComponent* pNext_;  //!< Pointer to the next IFD

    }; // class TiffDirectory

    /*!
      @brief This class models a TIFF sub-directory (sub-IFD). A sub-IFD
             is an entry with one or more values that are pointers to IFD
             structures containing an IFD. The TIFF standard defines
             some important tags to be sub-IFDs, including the %Exif and
             GPS tags.
     */
    class TiffSubIfd : public TiffEntryBase {
        friend class TiffReader;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffSubIfd(uint16_t tag, uint16_t group, uint16_t newGroup);
        //! Virtual destructor
        virtual ~TiffSubIfd();
        //@}

    protected:
        //! @name Manipulators
        //@{
        virtual TiffComponent* doAddPath(uint16_t tag, TiffPath& tiffPath);
        virtual TiffComponent* doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        //@}

        //! @name Write support (Manipulators)
        //@{
        /*!
          @brief Implements write(). Write the sub-IFD pointers to the \em blob,
                 return the number of bytes written. The \em valueIdx and
                 \em imageIdx arguments are not used.
         */
        virtual uint32_t doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}
        //! @name Write support (Accessors)
        //@{
        /*!
          @brief Implements writeData(). Write the sub-IFDs to the blob. Return
                 the number of bytes written.
         */
        virtual uint32_t doWriteData(Blob&     blob,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const;
        /*!
          @brief Implements writeImage(). Write the image data of each sub-IFD to
                 the blob. Return the number of bytes written.
         */
        virtual uint32_t doWriteImage(Blob&     blob,
                                      ByteOrder byteOrder) const;
        //! Implements size(). Return the size of the sub-Ifd pointers.
        uint32_t doSize() const;
        //! Implements sizeData(). Return the sum of the sizes of all sub-IFDs.
        virtual uint32_t doSizeData() const;
        //! Implements sizeImage(). Return the sum of the image sizes of all sub-IFDs.
        virtual uint32_t doSizeImage() const;
        //@}

    private:
        //! A collection of TIFF directories (IFDs)
        typedef std::vector<TiffDirectory*> Ifds;

        // DATA
        uint16_t newGroup_; //!< Start of the range of group numbers for the sub-IFDs
        Ifds     ifds_;     //!< The subdirectories

    }; // class TiffSubIfd

    /*!
      @brief This class is the basis for Makernote support in TIFF. It contains
             a pointer to a concrete Makernote. The TiffReader visitor has the
             responsibility to create the correct Make/Model specific Makernote
             for a particular TIFF file. Calls to child management methods are
             forwarded to the concrete Makernote, if there is one.
     */
    class TiffMnEntry : public TiffEntryBase {
        friend class TiffReader;
        friend class TiffDecoder;
        friend class TiffEncoder;
        friend class TiffPrinter;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffMnEntry(uint16_t tag, uint16_t group, uint16_t mnGroup);
        //! Virtual destructor
        virtual ~TiffMnEntry();
        //@}

    protected:
        //! @name Manipulators
        //@{
        virtual TiffComponent* doAddPath(uint16_t tag, TiffPath& tiffPath);
        virtual TiffComponent* doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual TiffComponent* doAddNext(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        //@}

        //! @name Accessors
        //@{
        //! Implements count(). Return number of components in the entry.
        virtual uint32_t doCount() const;
        //@}

        //! @name Write support (Manipulators)
        //@{
        /*!
          @brief Implements write() by forwarding the call to the actual
                 concrete Makernote, if there is one.
         */
        virtual uint32_t doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}
        //! @name Write support (Accessors)
        //@{
        // Using doWriteData from base class
        // Using doWriteImage from base class
        /*!
          @brief Implements size() by forwarding the call to the actual
                 concrete Makernote, if there is one.
         */
        virtual uint32_t doSize() const;
        // Using doSizeData from base class
        // Using doSizeImage from base class
        //@}

    private:
        // DATA
        uint16_t       mnGroup_;             //!< New group for concrete mn
        TiffComponent* mn_;                  //!< The Makernote

    }; // class TiffMnEntry

    /*!
      @brief Composite to model an array of tags, each consisting of values
             of a given type. Canon and Minolta makernotes use such tags. The
             elements of this component are of type TiffArrayElement.
     */
    class TiffArrayEntry : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffArrayEntry(uint16_t tag,
                       uint16_t group,
                       uint16_t elGroup,
                       TiffType elTiffType,
                       bool     addSizeElement);
        //! Virtual destructor
        virtual ~TiffArrayEntry();
        //@}

        //! @name Accessors
        //@{
        //! Return the size of the array elements
        uint16_t  elSize()  const { return elSize_; }
        //! Return the group for the array elements
        uint16_t  elGroup() const { return elGroup_; }
        //@}

    protected:
        //! @name Manipulators
        //@{
        virtual TiffComponent* doAddPath(uint16_t tag, TiffPath& tiffPath);
        virtual TiffComponent* doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        //@}

        //! @name Accessors
        //@{
        //! Implements count(). Return number of components in the entry.
        virtual uint32_t doCount() const;
        //@}

        //! @name Write support (Manipulators)
        //@{
        /*!
          @brief Implements write(). Write each component, fill gaps with 0s.
                 Check for duplicate tags and throw Error(50) if any are
                 detected.
         */
        virtual uint32_t doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}
        //! @name Write support (Accessors)
        //@{
        // Using doWriteData from base class
        // Using doWriteImage from base class
        /*!
          @brief Implements size().
         */
        virtual uint32_t doSize() const;
        // Using doSizeData from base class
        // Using doSizeImage from base class
        //@}

    private:
        // DATA
        uint16_t   elSize_;         //!< Size of the array elements (in bytes)
        uint16_t   elGroup_;        //!< Group for the elements
        bool       addSizeElement_; //!< Indicates size needs to be provided in the first element
        Components elements_;       //!< List of elements in this composite
    }; // class TiffArrayEntry

    /*!
      @brief Element of a TiffArrayEntry. The value of all elements of a TiffArrayEntry
             must be of the same type. Canon and Minolta makernotes use such arrays.
     */
    class TiffArrayElement : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffArrayElement(uint16_t  tag,
                         uint16_t  group,
                         TiffType  elTiffType,
                         ByteOrder elByteOrder)
            : TiffEntryBase(tag, group),
              elTiffType_(elTiffType),
              elByteOrder_(elByteOrder) {}
        //! Virtual destructor.
        virtual ~TiffArrayElement() {}
        //@}

        //! @name Accessors
        //@{
        TiffType  elTiffType()  const { return elTiffType_; }
        ByteOrder elByteOrder() const { return elByteOrder_; }
        //@}

    protected:
        //! @name Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum);
        //@}

        //! @name Write support (Manipulators)
        //@{
        /*!
          @brief Implements write(). Write the value using the element specific
                 byte order, if any. Make sure the value is of the correct type,
                 else throw Error(51).
         */
        virtual uint32_t doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}
        // Using doWriteData from base class
        // Using doWriteImage from base class
        // Using doSize from base class
        // Using doSizeData from base class
        // Using doSizeImage from base class

    private:
        // DATA
        TiffType  elTiffType_;    //!< TIFF type of the element
        ByteOrder elByteOrder_;   //!< Byte order to read/write the element

    }; // class TiffArrayElement

// *****************************************************************************
// template, inline and free functions

    /*!
      @brief Compare two TIFF component pointers by tag. Return true if the tag
             of component lhs is less than that of rhs.
     */
    bool cmpTagLt(TiffComponent const* lhs, TiffComponent const* rhs);

    //! Return the group name for a group
    const char* tiffGroupName(uint16_t group);

    //! Return the TIFF group id for a group name
    uint16_t tiffGroupId(const std::string& groupName);

    //! Function to create and initialize a new TIFF directory
    TiffComponent::AutoPtr newTiffDirectory(uint16_t tag,
                                            const TiffStructure* ts);

    //! Function to create and initialize a new TIFF entry
    TiffComponent::AutoPtr newTiffEntry(uint16_t tag,
                                        const TiffStructure* ts);

    //! Function to create and initialize a new TIFF sub-directory
    TiffComponent::AutoPtr newTiffSubIfd(uint16_t tag,
                                         const TiffStructure* ts);

    //! Function to create and initialize a new TIFF makernote entry
    TiffComponent::AutoPtr newTiffMnEntry(uint16_t tag,
                                          const TiffStructure* ts);

    //! Function to create and initialize a new array entry
    template<TiffType tiffType, bool addSizeElement>
    TiffComponent::AutoPtr newTiffArrayEntry(uint16_t tag,
                                             const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(
            new TiffArrayEntry(tag, ts->group_, ts->newGroup_, tiffType, addSizeElement));
    }

    //! Function to create and initialize a new array element
    template<TiffType tiffType, ByteOrder byteOrder>
    TiffComponent::AutoPtr newTiffArrayElement(uint16_t tag,
                                               const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(
            new TiffArrayElement(tag, ts->group_, tiffType, byteOrder));
    }

    template<TiffType tiffType>
    TiffComponent::AutoPtr newTiffArrayElement(uint16_t tag,
                                               const TiffStructure* ts)
    {
        return newTiffArrayElement<tiffType, invalidByteOrder>(tag, ts);
    }

    //! Function to create and initialize a new TIFF entry for a thumbnail (data)
    template<uint16_t szTag, uint16_t szGroup>
    TiffComponent::AutoPtr newTiffThumbData(uint16_t tag,
                                            const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(
            new TiffDataEntry(tag, ts->group_, szTag, szGroup));
    }

    //! Function to create and initialize a new TIFF entry for a thumbnail (size)
    template<uint16_t dtTag, uint16_t dtGroup>
    TiffComponent::AutoPtr newTiffThumbSize(uint16_t tag,
                                            const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(
            new TiffSizeEntry(tag, ts->group_, dtTag, dtGroup));
    }

    //! Function to create and initialize a new TIFF entry for image data
    template<uint16_t szTag, uint16_t szGroup>
    TiffComponent::AutoPtr newTiffImageData(uint16_t tag,
                                            const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(
            new TiffImageEntry(tag, ts->group_, szTag, szGroup));
    }

    //! Function to create and initialize a new TIFF entry for image data (size)
    template<uint16_t dtTag, uint16_t dtGroup>
    TiffComponent::AutoPtr newTiffImageSize(uint16_t tag,
                                            const TiffStructure* ts)
    {
        // Todo: Same as newTiffThumbSize - consolidate (rename)?
        assert(ts);
        return TiffComponent::AutoPtr(
            new TiffSizeEntry(tag, ts->group_, dtTag, dtGroup));
    }

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef TIFFCOMPOSITE_INT_HPP_
