// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005 Andreas Huggel <ahuggel@gmx.net>
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
  @file    ifd.hpp
  @brief   Encoding and decoding of IFD (%Image File Directory) data
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    09-Jan-04, ahu: created<BR>
           11-Feb-04, ahu: isolated as a component
 */
#ifndef IFD_HPP_
#define IFD_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <string>
#include <vector>
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class Ifd;

// *****************************************************************************
// class definitions

    /*!
      @brief Data structure for one IFD directory entry. See the description of
             class Ifd for an explanation of the supported modes for memory
             allocation.
    */
    class Entry {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Default constructor. The entry allocates memory for its 
          data if alloc is true (the default), otherwise it remembers
          just the pointers into a read and writeable data buffer which
          it doesn't allocate or delete.
         */ 
        explicit Entry(bool alloc =true);
        //! Destructor
        ~Entry();
        //! Copy constructor
        Entry(const Entry& rhs);
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator
        Entry& operator=(const Entry& rhs);
        //! Set the tag
        void setTag(uint16_t tag) { tag_ = tag; }
        //! Set the IFD id
        void setIfdId(IfdId ifdId) { ifdId_ = ifdId; }
        //! Set the index (unique id of an entry within one IFD)
        void setIdx(int idx) { idx_ = idx; }
        //! Set the offset. The offset is relative to the start of the IFD.
        void setOffset(long offset) { offset_ = offset; }
        /*!
          @brief Set the value of the entry to a single unsigned long component,
                 i.e., set the type of the entry to unsigned long, number of
                 components to one and the value according to the data provided. 

          The size of the data buffer is set to at least four bytes, but is left
          unchanged if it can accomodate the pointer.  This method can be used
          to set the value of a tag which contains a pointer (offset) to a
          location in the Exif data (like e.g., ExifTag, 0x8769 in IFD0, which
          contains a pointer to the Exif IFD). 
          <BR>This method cannot be used to set the value of a newly created
          %Entry in non-alloc mode.

          @note This method is now deprecated, use data area related methods
                instead.
         */
        void setValue(uint32_t data, ByteOrder byteOrder);
        /*!
          @brief Set type, count, the data buffer and its size.

          Copies the provided buffer when called in memory allocation mode.
          <BR>In non-alloc mode, use this method to initialise the data of a
          newly created %Entry.  In this case, only the pointer to the buffer is
          copied, i.e., the buffer must remain valid throughout the life of the
          %Entry.  Subsequent calls in non-alloc mode will overwrite the data
          pointed to by this pointer with the data provided, i.e., the buffer
          provided in subsequent calls can be deleted after the call. 
          <BR>In either memory allocation mode, the data buffer provided must be
          large enough to hold count components of type. The size of the buffer 
          will be as indicated in the size argument. I.e., it is possible to
          allocate (set) a data buffer larger than required to hold count
          components of the given type.

          @param type The type of the data.
          @param count Number of components in the buffer.
          @param data Pointer to the data buffer.
          @param size Size of the desired data buffer in bytes.
          @throw Error if no memory allocation is allowed 
                 and the size of the data buffer is larger than the existing 
                 data buffer of the entry or if size is not large enough to hold
                 count components of the given type.
         */
        void setValue(uint16_t type, uint32_t count, const byte* data, long size);
        /*!
          @brief Set the data area. Memory management as for 
          setValue(uint16_t, uint32_t, const byte*, long)

          For certain tags the regular value of an IFD entry is an offset to a
          data area outside of the IFD. Examples are Exif tag 0x8769 in IFD0
          (Exif.Image.ExifTag) or tag 0x0201 in IFD1
          (Exif.Thumbnail.JPEGInterchangeFormat). The offset of ExifTag points
          to a data area containing the Exif IFD. That of JPEGInterchangeFormat
          contains the JPEG thumbnail image.  
          This method sets the data area of a tag in accordance with the memory
          allocation mode.

          @param buf Pointer to the data area.
          @param len Size of the data area.
          
          @throw Error in non-alloc mode, if there already is a dataarea but the 
                 size of the existing dataarea is not large enough for the 
                 new buffer.
         */
        void setDataArea(const byte* buf, long len);
        /*!
          @brief Set the offset(s) to the data area of an entry. 

          Add @em offset to each data component of the entry. This is used by
          Ifd::copy to convert the data components of an entry containing
          offsets relative to the data area to become offsets from the start of
          the TIFF header.  Usually, entries with a data area have exactly one 
          unsigned long data component, which is 0.

          @param offset Offset 
          @param byteOrder Byte order

          @throw Error if the offset is out of range for the data type of the 
                 tag or the data type is not supported.
         */
        void setDataAreaOffsets(uint32_t offset, ByteOrder byteOrder);
        /*!
          @brief Update the base pointer of the Entry from \em pOldBase 
                 to \em pNewBase.

          Allows to re-locate the underlying data buffer to a new location
          \em pNewBase. This method only has an effect in non-alloc mode.

          @param pOldBase Base pointer of the old data buffer
          @param pNewBase Base pointer of the new data buffer
         */
        void updateBase(byte* pOldBase, byte* pNewBase);
        //@}

        //! @name Accessors
        //@{
        //! Return the tag
        uint16_t tag() const { return tag_; }
        //! Return the type id.
        uint16_t type() const { return type_; }
        //! Return the name of the type
        const char* typeName() const 
            { return TypeInfo::typeName(TypeId(type_)); }
        //! Return the size in bytes of one element of this type
        long typeSize() const
            { return TypeInfo::typeSize(TypeId(type_)); }
        //! Return the IFD id
        IfdId ifdId() const { return ifdId_; }
        //! Return the index (unique id >0 of an entry within an IFD, 0 if not set)
        int idx() const { return idx_; }
        //! Return the number of components in the value
        uint32_t count() const { return count_; }
        /*!
          @brief Return the size of the data buffer in bytes.
          @note  There is no minimum size for the data buffer, except that it
                 must be large enough to hold the data.
         */
        long size() const { return size_; }
        //! Return the offset from the start of the IFD to the data of the entry
        long offset() const { return offset_; }
        /*!
          @brief Return a pointer to the data buffer. Do not attempt to write
                 to this pointer.
         */
        const byte* data() const { return pData_; }
        /*!
          @brief Return a pointer to the n-th component, 0 if there is no 
                 n-th component. Do not attempt to write to this pointer.
         */
        const byte* component(uint32_t n) const;
        //! Get the memory allocation mode
        bool alloc() const { return alloc_; }
        //! Return the size of the data area.
        long sizeDataArea() const { return sizeDataArea_; }
        /*!
          @brief Return a pointer to the data area. Do not attempt to write to
                 this pointer.

          For certain tags the regular value of an IFD entry is an offset to a
          data area outside of the IFD. Examples are Exif tag 0x8769 in IFD0
          (Exif.Image.ExifTag) or tag 0x0201 in IFD1
          (Exif.Thumbnail.JPEGInterchangeFormat). The offset of ExifTag points
          to a data area containing the Exif IFD. That of JPEGInterchangeFormat
          contains the JPEG thumbnail image.
          Use this method to access (read-only) the data area of a tag. Use 
          setDataArea() to write to the data area.

          @return Return a pointer to the data area.
         */
        const byte* dataArea() const { return pDataArea_; }
        //@}

    private:
        // DATA
        /*!
          True:  Requires memory allocation and deallocation,<BR>
          False: No memory management needed.
         */
        bool alloc_;
        //! Redundant IFD id (it is also at the IFD)
        IfdId ifdId_;
        //! Unique id of an entry within an IFD (0 if not set)
        int idx_;
        //! Tag
        uint16_t tag_;
        //! Type
        uint16_t type_;
        //! Number of components
        uint32_t count_;
        //! Offset from the start of the IFD to the data
        long offset_;
        /*!
          Size of the data buffer holding the value in bytes, there is 
          no minimum size.
         */
        long size_;
        //! Pointer to the data buffer
        byte* pData_;
        //! Size of the data area
        long sizeDataArea_;
        //! Pointer to the data area
        byte* pDataArea_;

    }; // class Entry

    //! Container type to hold all IFD directory entries
    typedef std::vector<Entry> Entries;

    //! Unary predicate that matches an Entry with a given index
    class FindEntryByIdx {
    public:
        //! Constructor, initializes the object with the index to look for
        FindEntryByIdx(int idx) : idx_(idx) {}
        /*!
          @brief Returns true if the idx of the argument entry is equal
                 to that of the object.
        */
        bool operator()(const Entry& entry) const
            { return idx_ == entry.idx(); }

    private:
        int idx_;

    }; // class FindEntryByIdx

    //! Unary predicate that matches an Entry with a given tag
    class FindEntryByTag {
    public:
        //! Constructor, initializes the object with the tag to look for
        FindEntryByTag(uint16_t tag) : tag_(tag) {}
        /*!
          @brief Returns true if the tag of the argument entry is equal
                 to that of the object.
        */
        bool operator()(const Entry& entry) const
            { return tag_ == entry.tag(); }

    private:
        uint16_t tag_;
        
    }; // class FindEntryByTag

    /*!
      @brief Models an IFD (%Image File Directory)

      This class models an IFD as described in the TIFF 6.0 specification. 

      An instance of class %Ifd can operate in two modes, one that allocates and
      deallocates the memory required to store data, and one that doesn't
      perform such memory management.
      <BR>An external data buffer (not managed by %Ifd) is needed for an instance
      of %Ifd which operates in no memory management mode. The %Ifd will
      maintain only pointers into this buffer.
      <BR> The mode without memory management is used to make "non-intrusive
      write support" possible. This allows writing to Exif data of an image
      without changing the data layout of the Exif data, to maximize chances
      that tag data, which the Exif reader may not understand (e.g., the
      Makernote) remains valid. A "non-intrusive write operation" is the
      modification of tag data without increasing the data size.
   
      @note Use the mode with memory management (the default) if you are unsure 
            or if these memory management considerations are of no concern to you.

      @note The two different modes imply completely different copy and
            assignment behaviours, with the first resulting in entirely separate
            classes and the second mode resulting in multiple classes using one
            and the same data buffer.
     */
    class Ifd {
        //! @name Not implemented
        //@{
        //! Assignment not allowed (memory management mode alloc_ is const)
        Ifd& operator=(const Ifd& rhs);
        //@}

    public:
        //! %Entries const iterator type
        typedef Entries::const_iterator const_iterator;
        //! %Entries iterator type
        typedef Entries::iterator iterator;

        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to set the IFD identifier. Memory management
                 is enabled, offset is set to 0. Serves as default constructor.
         */
        explicit Ifd(IfdId ifdId =ifdIdNotSet);
        /*!
          @brief Constructor. Allows to set the IFD identifier and the offset of
                 the IFD from the start of TIFF header. Memory management is
                 enabled.
         */
        Ifd(IfdId ifdId, long offset);
        /*!
          @brief Constructor. Allows to set the IFD identifier, offset of the
                 IFD from the start of TIFF header, choose whether or not
                 memory management is required for the Entries, and decide
                 whether this IFD has a next pointer.
         */
        Ifd(IfdId ifdId, long offset, bool alloc, bool hasNext =true);
        //! Copy constructor
        Ifd(const Ifd& rhs);
        //! Destructor
        ~Ifd();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read a complete IFD and its data from a data buffer

          @param buf Pointer to the Exif data buffer that contains the IFD to 
                     decode. Usually, the buffer will contain all Exif data 
                     starting from the TIFF header.
          @param len Number of bytes in the Exif data buffer.
          @param start IFD starts at buf + start.
          @param byteOrder Applicable byte order (little or big endian).
          @param shift IFD offsets are relative to buf + shift.

          @return 0 if successful;<BR>
                  6 if the data buffer is too small, e.g., if an offset points 
                    beyond the provided buffer. The IFD is cleared in this 
                    case.
         */
        int read(const byte* buf, 
                 long len, 
                 long start, 
                 ByteOrder byteOrder,
                 long shift =0);
        /*!
          @brief Copy the IFD to a data array, update the offsets of the IFD and
                 all its entries, return the number of bytes written.

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
          @return Returns the number of characters written.
         */
        long copy(byte* buf, ByteOrder byteOrder, long offset =0);
        /*!
          @brief Reset the IFD. Delete all IFD entries from the class and put
                 the object in a state where it can accept completely new
                 entries.
         */
        void clear();
        /*!
          @brief Set the offset of the next IFD. Byte order is needed to update
                 the underlying data buffer in non-alloc mode. This method only
                 has an effect if the IFD was instantiated with hasNext = true.
         */
        void setNext(uint32_t next, ByteOrder byteOrder);
        /*!
          @brief Add the entry to the IFD. No duplicate-check is performed,
                 i.e., it is possible to add multiple entries with the same tag.
                 The memory allocation mode of the entry to be added must match
                 that of the IFD and the IFD ids of the IFD and entry must
                 match.
         */
        void add(const Entry& entry);
        /*!
          @brief Delete the directory entry with the given tag. Return the index 
                 of the deleted entry or 0 if no entry with tag was found.
         */
        int erase(uint16_t tag);
        /*!
          @brief Delete the directory entry at iterator position pos, return the
                 position of the next entry. Note that iterators into the
                 directory, including pos, are potentially invalidated by this
                 call.
         */
        iterator erase(iterator pos);
        //! Sort the IFD entries by tag
        void sortByTag();
        //! The first entry
        iterator begin() { return entries_.begin(); }
        //! End of the entries
        iterator end() { return entries_.end(); }
        //! Find an IFD entry by idx, return an iterator into the entries list
        iterator findIdx(int idx);
        //! Find an IFD entry by tag, return an iterator into the entries list
        iterator findTag(uint16_t tag);
        /*!
          @brief Update the base pointer of the Ifd and all entries to \em pNewBase.

          Allows to re-locate the underlying data buffer to a new location
          \em pNewBase. This method only has an effect in non-alloc mode.

          @param pNewBase Pointer to the new data buffer

          @return Old base pointer or 0 if called in alloc mode
         */
        byte* updateBase(byte* pNewBase);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Read a sub-IFD from the location pointed to by the directory entry 
                 with the given tag.

          @param dest References the destination IFD.
          @param buf The data buffer to read from. The buffer must contain all Exif 
                     data starting from the TIFF header.
          @param len Number of bytes in the data buffer 
          @param byteOrder Applicable byte order (little or big endian).
          @param tag Tag to look for.

          @return 0 if successful;<BR>
                  6 if reading the sub-IFD failed (see read() above) or
                    the location pointed to by the directory entry with the 
                    given tag is outside of the data buffer.

          @note It is not considered an error if the tag cannot be found in the 
                IFD. 0 is returned and no action is taken in this case.
        */
        int readSubIfd(
            Ifd& dest, const byte* buf, long len, ByteOrder byteOrder, uint16_t tag
        ) const;
        //! Get the memory allocation mode, see the Ifd class description for details
        bool alloc() const { return alloc_; }
        //! The first entry
        const_iterator begin() const { return entries_.begin(); }
        //! End of the entries
        const_iterator end() const { return entries_.end(); }
        //! Find an IFD entry by idx, return a const iterator into the entries list
        const_iterator findIdx(int idx) const;
        //! Find an IFD entry by tag, return a const iterator into the entries list
        const_iterator findTag(uint16_t tag) const;
        //! Get the IfdId of the IFD
        IfdId ifdId() const { return ifdId_; }
        //! Get the offset of the IFD from the start of the TIFF header
        long offset() const { return offset_; }
        /*!
          @brief Get the offset of the first data entry outside of the IFD from
                 the start of the TIFF header, return 0 if there is none. The 
                 data offset is determined when the IFD is read.
         */
        long dataOffset() const { return dataOffset_; }
        //! Get the offset to the next IFD from the start of the TIFF header
        uint32_t next() const { return next_; }
        //! Get the number of directory entries in the IFD
        long count() const { return static_cast<long>(entries_.size()); }
        //! Get the size of this IFD in bytes (IFD only, without data)
        long size() const;
        /*!
          @brief Return the total size of the data of this IFD in bytes; sums
                 the size of all directory entries where size is greater than
                 four plus the size of all data areas, i.e., all data that
                 requires memory outside the IFD directory entries is counted.
         */
        long dataSize() const;
        /*!
          @brief Print the IFD in human readable format to the given stream;
                 begin each line with prefix.
         */
        void print(std::ostream& os, const std::string& prefix ="") const;
        //@}

    private:
        //! Helper structure to build IFD entries
        struct PreEntry {
            uint16_t tag_;
            uint16_t type_; 
            uint32_t count_;
            long size_;
            long offsetLoc_;
            long offset_;
        };

        //! cmpPreEntriesByOffset needs to know about PreEntry, that's all.
        friend bool cmpPreEntriesByOffset(const PreEntry&, const PreEntry&);
    
        //! Container for 'pre-entries'
        typedef std::vector<PreEntry> PreEntries;

        // DATA
        /*!
          True:  requires memory allocation and deallocation,
          False: no memory management needed.
        */
        const bool alloc_;
        //! IFD entries
        Entries entries_;
        //! IFD Id
        IfdId ifdId_;
        //! Pointer to IFD
        byte* pBase_;
        //! Offset of the IFD from the start of the TIFF header
        long offset_;
        //! Offset of the first data entry outside of the IFD directory
        long dataOffset_;
        //! Indicates whether the IFD has a next pointer
        bool hasNext_;
        //! Pointer to the offset of next IFD
        byte* pNext_;
        /*!
          The offset of the next IFD from the start of the TIFF header as data 
          value (always in sync with *pNext_)
        */
        uint32_t next_;

    }; // class Ifd

// *****************************************************************************
// free functions

    /*!
      @brief Compare two IFD entries by tag. Return true if the tag of entry
             lhs is less than that of rhs.
     */
    bool cmpEntriesByTag(const Entry& lhs, const Entry& rhs);

    /*!
      @brief Compare two 'pre-IFD entries' by offset, taking care of special
             cases where one or both of the entries don't have an offset.
             Return true if the offset of entry lhs is less than that of rhs,
             else false. By definition, entries without an offset are greater
             than those with an offset.
    */
    bool cmpPreEntriesByOffset(const Ifd::PreEntry& lhs, const Ifd::PreEntry& rhs);
   
}                                       // namespace Exiv2

#endif                                  // #ifndef IFD_HPP_
