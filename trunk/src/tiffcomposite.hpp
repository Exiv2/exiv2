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
        const uint16_t mn      = 256; //!< Makernote
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
      @brief This class models a TIFF header structure.
     */
    class TiffHeade2 {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffHeade2()
            : byteOrder_ (littleEndian),
              offset_    (0x00000008)
            {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the TIFF header from a data buffer. Return false if the
                 data buffer does not contain a TIFF header, else true.

          @param pData Pointer to the data buffer.
          @param size  Number of bytes in the data buffer.
         */
        bool read(const byte* pData, uint32_t size);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Write the TIFF header to the binary image \em blob.
                 This method appends to the blob.

          @param blob Binary image to add to.

          @throw Error If the header cannot be written.
         */
        void write(Blob& blob) const;
        /*!
          @brief Print debug info for the TIFF header to \em os.

          @param os Output stream to write to.
          @param prefix Prefix to be written before each line of output.
         */
        void print(std::ostream& os, const std::string& prefix ="") const;
        //! Return the byte order (little or big endian).
        ByteOrder byteOrder() const { return byteOrder_; }
        //! Return the offset to the start of the root directory
        uint32_t offset() const { return offset_; }
        //@}

    private:
        // DATA
        ByteOrder             byteOrder_; //!< Applicable byte order
        uint32_t              offset_;    //!< Offset to the start of the root dir

        static const uint16_t tag_;       //!< 42, identifies the buffer as TIFF data

    }; // class TiffHeade2

    /*!
      @brief Interface class for components of a TIFF directory hierarchy
             (Composite pattern).  Both TIFF directories as well as entries
             implement this interface.  A component can be uniquely identified
             by a tag, group tupel.  This class is implemented as a NVI
             (Non-Virtual Interface) and it has an interface for visitors (Visitor
             pattern).
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
      @brief This class models a TIFF directory (%Ifd). It is a composite
             component of the TIFF tree.
     */
    class TiffDirectory : public TiffComponent {
        friend class TiffPrinter;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffDirectory(uint16_t tag, uint16_t group)
            : TiffComponent(tag, group), pNext_(0) {}
        //! Virtual destructor
        virtual ~TiffDirectory();
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
        TiffComponent* pNext_;  //!< Pointer to the next IFD

    }; // class TiffDirectory

    /*!
      @brief This class models a TIFF sub-directory (sub-IFD). A sub-IFD
             is an entry with a value that is a pointer to an IFD
             structure and contains this IFD. The TIFF standard defines
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
            : TiffEntryBase(tag, group), ifd_(tag, newGroup) {}
        //! Virtual destructor
        virtual ~TiffSubIfd() {}
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
        TiffDirectory ifd_; //!< The subdirectory

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
             unsigned short value. Canon makernotes use such tags. The
             elements of this component are usually of type TiffArrayElement.
             If the type of the entry is not unsigned short, it degenerates 
             to a standard TIFF entry.
     */
    class TiffArrayEntry : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffArrayEntry(uint16_t tag, uint16_t group, uint16_t elGroup)
            : TiffEntryBase(tag, group), elGroup_(elGroup) {}
        //! Virtual destructor
        virtual ~TiffArrayEntry();
        //@}

        //! @name Accessors
        //@{
        //! Return the group for the array elements
        uint16_t elGroup() const { return elGroup_; }
        //@}

    private:
        //! @name Manipulators
        //@{
        virtual void doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        //@}

    private:
        // DATA
        uint16_t   elGroup_;  //!< Group for the elements
        Components elements_; //!< List of elements in this composite
    }; // class TiffArrayEntry

    /*!
      @brief Element of a TiffArrayEntry. The value is exactly one unsigned
             short component. Canon makernotes use arrays of such elements.
     */
    class TiffArrayElement : public TiffEntryBase {
    public:
        //! @name Creators
        //@{
        //! Constructor
        TiffArrayElement(uint16_t tag, uint16_t group) 
            : TiffEntryBase(tag, group) {}
        //! Virtual destructor.
        virtual ~TiffArrayElement() {}
        //@}

    private:
        //! @name Manipulators
        //@{
        virtual void doAccept(TiffVisitor& visitor);
        //@}

    }; // class TiffArrayElement

// *****************************************************************************
// template, inline and free functions

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
    TiffComponent::AutoPtr newTiffArrayEntry(uint16_t tag,
                                             const TiffStructure* ts);

    //! Function to create and initialize a new array element
    TiffComponent::AutoPtr newTiffArrayElement(uint16_t tag,
                                               const TiffStructure* ts);

}                                       // namespace Exiv2

#endif                                  // #ifndef TIFFCOMPOSITE_HPP_
