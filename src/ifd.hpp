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
  @file    ifd.hpp
  @brief   Encoding and decoding of IFD (Image File Directory) data
  @version $Name:  $ $Revision: 1.11 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    09-Jan-04, ahu: created
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
namespace Exif {

// *****************************************************************************
// class declarations
    class MakerNote;

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
        void setTag(uint16 tag) { tag_ = tag; }
        //! Set the IFD id
        void setIfdId(IfdId ifdId) { ifdId_ = ifdId; }
        //! Set the index (unique id of an entry within one IFD)
        void setIdx(int idx) { idx_ = idx; }
        //! Set the pointer to the MakerNote
        void setMakerNote(MakerNote* makerNote) { makerNote_ = makerNote; }
        //! Set the offset. The offset is relative to the start of the IFD.
        void setOffset(uint32 offset) { offset_ = offset; }
        /*!
          @brief Set the value of the entry to a single unsigned long component,
                 i.e., set the type of the entry to unsigned long, number of
                 components to one and the value according to the data provided. 

          The size of the data buffer is set to at least four bytes, but is left
          unchanged if it can accomodate the pointer.  This method can be used
          to set the value of a tag which contains a pointer (offset) to a
          location in the %Exif data (like e.g., ExifTag, 0x8769 in IFD0, which
          contains a pointer to the %Exif IFD). 
          <BR>This method cannot be used to set the value of a newly created
          %Entry in non-alloc mode.
        */
        void setValue(uint32 data, ByteOrder byteOrder);
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
          @throw Error ("Value too large") if no memory allocation is allowed 
                 and the size of the data buffer is larger than the existing 
                 data buffer of the entry.<BR>
          @throw Error ("Size too small") if size is not large enough to hold
                 count components of the given type.
         */
        void setValue(uint16 type, uint32 count, const char* data, long size);
        //@}

        //! @name Accessors
        //@{
        //! Return the tag
        uint16 tag() const { return tag_; }
        //! Return the type id.
        uint16 type() const { return type_; }
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
        //! Return the pointer to the associated MakerNote
        MakerNote* makerNote() const { return makerNote_; }
        //! Return the number of components in the value
        uint32 count() const { return count_; }
        /*!
          @brief Return the size of the data buffer in bytes.
          @note  There is no minimum size for the data buffer, except that it
                 must be large enough to hold the data.
         */
        long size() const { return size_; }
        //! Return the offset from the start of the IFD to the data of the entry
        uint32 offset() const { return offset_; }
        /*!
          @brief Return a pointer to the data area. Do not attempt to write
          to this pointer.
        */
        const char* data() const { return data_; }
        /*!
          @brief Return a pointer to the n-th component, 0 if there is no 
                 n-th component. Do not attempt to write to this pointer.
         */
        const char* component(uint32 n) const;
        //! Get the memory allocation mode
        bool alloc() const { return alloc_; }
        //@}

    private:
        /*!
          True:  Requires memory allocation and deallocation,<BR>
          False: No memory management needed.
        */
        bool alloc_;
        //! Redundant IFD id (it is also at the IFD)
        IfdId ifdId_;
        //! Unique id of an entry within an IFD (0 if not set)
        int idx_;
        //! Pointer to the associated MakerNote
        MakerNote* makerNote_;
        //! Tag
        uint16 tag_;
        //! Type
        uint16 type_;
        //! Number of components
        uint32 count_;
        //! Offset from the start of the IFD to the data
        uint32 offset_;
        /*!
          Size of the data buffer holding the value in bytes, there is 
          no minimum size.
         */
        long size_;
        //! Pointer to the data buffer
        char* data_;
                               
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
        FindEntryByTag(uint16 tag) : tag_(tag) {}
        /*!
          @brief Returns true if the tag of the argument entry is equal
                 to that of the object.
        */
        bool operator()(const Entry& entry) const
            { return tag_ == entry.tag(); }

    private:
        uint16 tag_;
        
    }; // class FindEntryByTag

    /*!
      @brief Models an IFD (Image File Directory)

      This class models an IFD as described in the TIFF 6.0 specification. 

      An instance of class %Ifd can operate in two modes, one that allocates and
      deallocates the memory required to store data, and one that doesn't
      perform such memory management.
      <BR>An external data buffer (not managed by %Ifd) is needed for an instance
      of %Ifd which operates in no memory management mode. The %Ifd will
      maintain only pointers into this buffer.
      <BR> The mode without memory management is used to make "non-intrusive
      write support" possible. This allows writing to %Exif data of an image
      without changing the data layout of the %Exif data, to maximize chances
      that tag data, which the %Exif reader may not understand (e.g., the
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
    public:
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
        Ifd(IfdId ifdId, uint32 offset);
        /*!
          @brief Constructor. Allows to set the IFD identifier, offset of the
                 IFD from the start of TIFF header and choose whether or not
                 memory management is required for the Entries.
         */
        Ifd(IfdId ifdId, uint32 offset, bool alloc);
        //@}

        //! Entries const iterator type
        typedef Entries::const_iterator const_iterator;
        //! Entries iterator type
        typedef Entries::iterator iterator;

        //! @name Manipulators
        //@{
        /*!
          @brief Read a complete IFD and its data from a data buffer

          @param buf Pointer to the data to decode. The buffer must start with the 
                 IFD data (unlike the readSubIfd() method).
          @param byteOrder Applicable byte order (little or big endian).
          @param offset (Optional) offset of the IFD from the start of the TIFF
                 header, if known. If not given, the offset will be guessed
                 using the assumption that the smallest offset of all IFD
                 directory entries points to a data buffer immediately follwing
                 the IFD.

          @return 0 if successful
         */
        int read(const char* buf, ByteOrder byteOrder, long offset =0);
        /*!
          @brief Read a sub-IFD from the location pointed to by the directory entry 
                 with the given tag.

          @param dest References the destination IFD.
          @param buf The data buffer to read from. The buffer must contain all Exif 
                     data starting from the TIFF header (unlike the read() method).
          @param byteOrder Applicable byte order (little or big endian).
          @param tag Tag to look for.

          @return 0 if successful
        */
        int readSubIfd(
            Ifd& dest, const char* buf, ByteOrder byteOrder, uint16 tag
        ) const;
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
        long copy(char* buf, ByteOrder byteOrder, long offset =0);
        /*!
          @brief Reset the IFD. Delete all IFD entries from the class and put
                 the object in a state where it can accept completely new
                 entries.
         */
        void clear();
        //! Set the offset of the next IFD
        void setNext(uint32 next) { next_ = next; }
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
        int erase(uint16 tag);
        //! Delete the directory entry at iterator position pos
        void erase(iterator pos);
        //! Sort the IFD entries by tag
        void sortByTag();
        //! The first entry
        iterator begin() { return entries_.begin(); }
        //! End of the entries
        iterator end() { return entries_.end(); }
        //! Find an IFD entry by idx, return an iterator into the entries list
        iterator findIdx(int idx);
        //! Find an IFD entry by tag, return an iterator into the entries list
        iterator findTag(uint16 tag);
        //@}

        //! @name Accessors
        //@{
        //! Get the memory allocation mode, see the Ifd class description for details
        bool alloc() const { return alloc_; }
        //! The first entry
        const_iterator begin() const { return entries_.begin(); }
        //! End of the entries
        const_iterator end() const { return entries_.end(); }
        //! Find an IFD entry by idx, return a const iterator into the entries list
        const_iterator findIdx(int idx) const;
        //! Find an IFD entry by tag, return a const iterator into the entries list
        const_iterator findTag(uint16 tag) const;
        //! Get the IfdId of the IFD
        IfdId ifdId() const { return ifdId_; }
        //! Get the offset of the IFD from the start of the TIFF header
        long offset() const { return offset_; }
        //! Get the offset to the next IFD from the start of the TIFF header
        long next() const { return next_; }
        //! Get the number of directory entries in the IFD
        long count() const { return entries_.size(); }
        //! Get the size of this IFD in bytes (IFD only, without data)
        long size() const;
        /*!
          @brief Return the total size of the data of this IFD in bytes,
                 sums the size of all directory entries where size is greater
                 than four (i.e., only data that requires memory outside the 
                 IFD directory entries is counted).
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
            uint16 tag_;
            uint16 type_; 
            uint32 count_;
            long size_;
            long offsetLoc_;
            uint32 offset_;
        };

        //! cmpPreEntriesByOffset needs to know about PreEntry, that's all.
        friend bool cmpPreEntriesByOffset(const PreEntry&, const PreEntry&);
    
        //! Container for 'pre-entries'
        typedef std::vector<PreEntry> PreEntries;

        const bool alloc_; // True:  requires memory allocation and deallocation,
                           // False: no memory management needed.
        Entries entries_;  // IFD entries
        IfdId ifdId_;      // IFD Id
        long offset_;      // offset of the IFD from the start of TIFF header
        long next_;        // offset of next IFD from the start of the TIFF header

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
   
}                                       // namespace Exif

#endif                                  // #ifndef IFD_HPP_
