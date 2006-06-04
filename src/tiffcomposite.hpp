// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006 Andreas Huggel <ahuggel@gmx.net>
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
  @file    tiffcomposite.hpp
  @brief
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    11-Apr-06, ahu: created
 */
#ifndef TIFFCOMPOSITE_HPP_
#define TIFFCOMPOSITE_HPP_

// *****************************************************************************
// included header files
#include "image.hpp"                            // for Blob
#include "types.hpp"

// + standard includes
#include <iosfwd>
#include <vector>
#include <string>
#include <cassert>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations

    class Value;
    class TiffVisitor;
    class TiffReader;
    class TiffMetadataDecoder;
    class TiffPrinter;
    class TiffIfdMakernote;
    struct TiffStructure;

// *****************************************************************************
// class definitions

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
        const uint16_t exif    =   3; //!< Exif IFD
        const uint16_t gps     =   4; //!< GPS IFD
        const uint16_t iop     =   5; //!< Interoperability IFD
        const uint16_t sub0_0  =   6; //!< Tiff SubIFD 0 in IFD0
        const uint16_t sub0_1  =   7; //!< Tiff SubIFD 1 in IFD0
        const uint16_t sub0_2  =   8; //!< Tiff SubIFD 2 in IFD0
        const uint16_t sub0_3  =   9; //!< Tiff SubIFD 3 in IFD0
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
             (Visitor pattern).
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
        TiffComponent(uint16_t tag, uint16_t group)
            : tag_(tag), group_(group), pData_(0) {}

        //! Virtual destructor.
        virtual ~TiffComponent() {}
        //@}

        //! @name Manipulators
        //@{
        //! Add a child to the component. Default is to do nothing.
        void addChild(AutoPtr tiffComponent);
        //! Add a "next" component to the component. Default is to do nothing.
        void addNext(AutoPtr tiffComponent);
        /*!
          @brief Interface to accept visitors (Visitor pattern).

          @param visitor The visitor.
         */
        void accept(TiffVisitor& visitor);
        /*!
          @brief Set a pointer to the start of the binary representation of the
                 component in a memory buffer. The buffer must be allocated and
                 freed outside of this class.
         */
        void setStart(const byte* pData) { pData_ = const_cast<byte*>(pData); }
        //@}

        //! @name Accessors
        //@{
        //! Return the tag of this entry.
        uint16_t tag()                        const { return tag_; }
        //! Return the group id of this component
        uint16_t group()                      const { return group_; }
        //! Return the group name of this component
        std::string groupName() const;
        //! Return a pointer to the start of the binary representation of the component
        const byte* start()                   const { return pData_; }

    protected:
        //! @name Manipulators
        //@{
        //! Implements addChild(). The default implementation does nothing.
        virtual void doAddChild(AutoPtr tiffComponent) {}

        //! Implements addNext(). The default implementation does nothing.
        virtual void doAddNext(AutoPtr tiffComponent) {}

        //! Implements accept()
        virtual void doAccept(TiffVisitor& visitor) =0;
        //@}

    private:
        // DATA
        uint16_t tag_;      //!< Tag that identifies the component
        uint16_t group_;    //!< Group id for this component
        /*!
          Pointer to the start of the binary representation of the component in
          a memory buffer. The buffer is allocated and freed outside of this class.
        */
        byte*    pData_;

    }; // class TiffComponent

    /*!
      Type for a function pointer for a function to create a TIFF component.
     */
    typedef TiffComponent::AutoPtr (*NewTiffCompFct)(uint16_t tag,
                                                     const TiffStructure* ts);

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
        NewTiffCompFct newTiffCompFct_; //!< Function to create the correct TIFF component
        uint16_t       newGroup_;       //!< Group of the newly created component
    };

    //! Search key for TIFF structure.
    struct TiffStructure::Key {
        //! Constructor
        Key(uint32_t e, uint16_t g) : e_(e), g_(g) {}
        uint32_t e_;                    //!< Extended tag
        uint16_t g_;                    //!< %Group
    };

    /*!
      Type for a factory function to create new TIFF components.
     */
    typedef TiffComponent::AutoPtr (*TiffCompFactoryFct)(uint32_t extendedTag,
                                                         uint16_t group);

    /*!
      @brief This abstract base class provides the common functionality of an
             IFD directory entry and defines an extended interface for derived
             concrete entries, which allows access to the attributes of the
             entry.
     */
    class TiffEntryBase : public TiffComponent {
        friend class TiffReader;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffEntryBase(uint16_t tag, uint16_t group)
            : TiffComponent(tag, group),
              type_(0), count_(0), offset_(0),
              size_(0), pData_(0), isAllocated_(false), pValue_(0) {}
        //! Virtual destructor.
        virtual ~TiffEntryBase();
        //@}

        //! @name Accessors
        //@{
        //! Return the Exiv2 type which corresponds to the field type
        TypeId   typeId()        const { return TypeId(type_); }
        //! Return the number of components in this entry
        uint32_t count()         const { return count_; }
        /*!
          Return the offset to the data area relative to the base for
          the component (usually the start of the TIFF header)
         */
        uint32_t offset()        const { return offset_; }
        //! Return the size of this component in bytes
        uint32_t size()          const { return size_; }
        //! Return a pointer to the data area of this component
        const byte* pData()      const { return pData_; }
        //! Return a pointer to the converted value of this component
        const Value* pValue()    const { return pValue_; }
        //@}

    private:
        // DATA
        uint16_t type_;     //!< Field Type
        uint32_t count_;    //!< The number of values of the indicated Type
        uint32_t offset_;   //!< Offset to the data area
        /*!
          Size of the data buffer holding the value in bytes, there is no
          minimum size.
         */
        uint32_t size_;
        const byte* pData_; //!< Pointer to the data area
        bool     isAllocated_; //!< True if this entry owns the value data
        Value*   pValue_;   //!< Converted data value

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

    private:
        //! @name Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        //@}

    }; // class TiffEntry

    /*!
      @brief A standard TIFF IFD entry consisting of a value which is an offset
             to a data area and the data area. The size of the data area is
             provided in a related TiffSizeEntry, tag and group of which are set
             in the constructor. This component is used, e.g., for
             \em Exif.Thumbnail.JPEGInterchangeFormat for which the size is
             provided in \em Exif.Thumbnail.JPEGInterchangeFormatLength.
     */
    class TiffDataEntry : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffDataEntry(uint16_t tag, uint16_t group, uint16_t szTag, uint16_t szGroup)
            : TiffEntryBase(tag, group), szTag_(szTag), szGroup_(szGroup) {}
        //! Virtual destructor.
        virtual ~TiffDataEntry() {}
        //@}

        //! @name Accessors
        //@{
        //! Return the group of the entry which has the size
        uint16_t szTag() const { return szTag_; }
        //! Return the group of the entry which has the size
        uint16_t szGroup() const { return szGroup_; }
        //@}

    private:
        //! @name Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        //@}

    private:
        // DATA
        const uint16_t szTag_;               //!< Tag of the entry with the size
        const uint16_t szGroup_;             //!< Group of the entry with the size

    }; // class TiffDataEntry

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

    private:
        //! @name Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
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

        //! @name Manipulators
        //@{
        //! Return true if the directory has a next pointer
        bool hasNext() const { return hasNext_; }
        //@}

    private:
        //! @name Manipulators
        //@{
        virtual void doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual void doAddNext(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        //@}

    private:
        // DATA
        Components components_; //!< List of components in this directory
        const bool hasNext_;    //!< True if the directory has a next pointer
        TiffComponent* pNext_;  //!< Pointer to the next IFD

    }; // class TiffDirectory

    //! A collection of TIFF directories (IFDs)
    typedef std::vector<TiffDirectory*> Ifds;

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
        TiffSubIfd(uint16_t tag, uint16_t group, uint16_t newGroup)
            : TiffEntryBase(tag, group), newGroup_(newGroup) {}
        //! Virtual destructor
        virtual ~TiffSubIfd();
        //@}

    private:
        //! @name Manipulators
        //@{
        virtual void doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        //@}

    private:
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
        friend class TiffMetadataDecoder;
        friend class TiffPrinter;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffMnEntry(uint16_t tag, uint16_t group, uint16_t mnGroup)
            : TiffEntryBase(tag, group), mnGroup_(mnGroup), mn_(0) {}
        //! Virtual destructor
        virtual ~TiffMnEntry();
        //@}

    private:
        //! @name Manipulators
        //@{
        virtual void doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual void doAddNext(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        //@}

    private:
        // DATA
        uint16_t       mnGroup_;             //!< New group for concrete mn
        TiffComponent* mn_;                  //!< The Makernote

    }; // class TiffMnEntry

    /*!
      @brief Composite to model an array of tags, each consisting of one
             unsigned short value. Canon and Minolta makernotes use such tags.
             The elements of this component are usually of type
             TiffArrayElement.
     */
    class TiffArrayEntry : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffArrayEntry(uint16_t tag,
                       uint16_t group,
                       uint16_t elGroup,
                       uint16_t elSize)
            : TiffEntryBase(tag, group),
              elSize_(elSize),
              elGroup_(elGroup) {}
        //! Virtual destructor
        virtual ~TiffArrayEntry();
        //@}

        //! @name Accessors
        //@{
        //! Return the type for the array elements
        uint16_t  elSize()  const { return elSize_; }
        //! Return the group for the array elements
        uint16_t  elGroup() const { return elGroup_; }
        //@}

    private:
        //! @name Manipulators
        //@{
        virtual void doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        //@}

    private:
        // DATA
        uint16_t   elSize_;      //!< Size of the array elements (in bytes)
        uint16_t   elGroup_;     //!< Group for the elements
        Components elements_;    //!< List of elements in this composite
    }; // class TiffArrayEntry

    /*!
      @brief Element of a TiffArrayEntry. The value is exactly one unsigned
             short component. Canon and Minolta makernotes use arrays of
             such elements.
     */
    class TiffArrayElement : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffArrayElement(uint16_t  tag,
                         uint16_t  group,
                         TypeId    elTypeId,
                         ByteOrder elByteOrder)
            : TiffEntryBase(tag, group),
              elTypeId_(elTypeId),
              elByteOrder_(elByteOrder) {}
        //! Virtual destructor.
        virtual ~TiffArrayElement() {}
        //@}

        //! @name Accessors
        //@{
        TypeId    elTypeId()    const { return elTypeId_; }
        ByteOrder elByteOrder() const { return elByteOrder_; }
        //@}

    private:
        //! @name Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        //@}

    private:
        // DATA
        TypeId    elTypeId_;      //!< Type of the element
        ByteOrder elByteOrder_;   //!< Byte order to read/write the element

    }; // class TiffArrayElement

// *****************************************************************************
// template, inline and free functions

    //! Return the group name for a group
    const char* tiffGroupName(uint16_t group);

    //! Function to create and initialize a new TIFF directory
    TiffComponent::AutoPtr newTiffDirectory(uint16_t tag,
                                            const TiffStructure* ts);

    //! Function to create and initialize a new TIFF sub-directory
    TiffComponent::AutoPtr newTiffSubIfd(uint16_t tag,
                                         const TiffStructure* ts);

    //! Function to create and initialize a new TIFF makernote entry
    TiffComponent::AutoPtr newTiffMnEntry(uint16_t tag,
                                          const TiffStructure* ts);

    //! Function to create and initialize a new array entry
    template<uint16_t elSize>
    TiffComponent::AutoPtr newTiffArrayEntry(uint16_t tag,
                                             const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(
            new TiffArrayEntry(tag, ts->group_, ts->newGroup_, elSize));
    }

    //! Function to create and initialize a new array element
    template<TypeId typeId, ByteOrder byteOrder>
    TiffComponent::AutoPtr newTiffArrayElement(uint16_t tag,
                                               const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(
            new TiffArrayElement(tag, ts->group_, typeId, byteOrder));
    }

    template<TypeId typeId>
    TiffComponent::AutoPtr newTiffArrayElement(uint16_t tag,
                                               const TiffStructure* ts)
    {
        return newTiffArrayElement<typeId, invalidByteOrder>(tag, ts);
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

}                                       // namespace Exiv2

#endif                                  // #ifndef TIFFCOMPOSITE_HPP_
