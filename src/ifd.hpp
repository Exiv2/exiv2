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
  @version $Name:  $ $Revision: 1.2 $
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
// class definitions

    /*!
      @brief Simple structure for 'raw' IFD directory entries (without any data
             greater than four bytes)
     */
    struct RawEntry {
        //! Default constructor
        RawEntry();
        //! The IFD id 
        IfdId ifdId_;
        //! Position in the IFD
        int ifdIdx_;
        //! Tag
        uint16 tag_;
        //! Type
        uint16 type_;
        //! Number of components
        uint32 count_;
        //! Offset, unprocessed
        uint32 offset_;
        //! Data from the IFD offset field if size is less or equal to four
        char offsetData_[4];
        //! Size of the data in bytes
        long size_;
    }; // struct RawEntry

    //! Container type to hold 'raw' IFD directory entries
    typedef std::vector<RawEntry> RawEntries;

    /*!
      @brief Data structure for one IFD directory entry. See the description of
             class Ifd for an explanation of the supported modes for memory
             allocation.
    */
    class Entry {
    public:
        /*!
          @brief Default constructor. The entry allocates memory for its 
          data if alloc is true (the default), otherwise it remembers
          just the pointers into a read and writeable data buffer which
          it doesn't allocate or delete.
        */ 
        explicit Entry(bool alloc =true);
        /*!
          @brief Constructor to create an %Entry from a raw entry structure
          and a data buffer. 

          @param e 'Raw' entry structure filled with the relevant data. The 
          offset_ field will only be used if size_ is greater than four.
          @param buf Character buffer with the data of the tag. If size_ is 
          less or equal four, the data from the original IFD offset 
          field must be available in the field offsetData_. The buf is
          not needed in this case and can be 0.
          @param alloc Determines if memory management is required. If alloc
          is true, a data buffer will be allocated to store data
          of more than four bytes, else only the pointer will be 
          stored. Data less or equal than four bytes is stored
          locally in the %Entry.
        */
        Entry(const RawEntry& e, const char* buf, bool alloc =true);
        //! Destructor
        ~Entry();
        //! Copy constructor
        Entry(const Entry& rhs);
        //! Assignment operator
        Entry& operator=(const Entry& rhs);
        //! @name Accessors
        //@{
        //! Return the IFD id
        IfdId ifdId() const { return ifdId_; }
        //! Return the index in the IFD
        int ifdIdx() const { return ifdIdx_; }
        //! Return the tag
        uint16 tag() const { return tag_; }
        //! Return the type id.
        uint16 type() const { return type_; }
        //! Return the number of components in the value
        uint32 count() const { return count_; }
        //! Return the offset from the start of the IFD
        uint32 offset() const { return offset_; }
        //! Return the size of the value in bytes
        long size() const { return size_; }
        /*!
          @brief Return a pointer to the data area. Do not attempt to write
          to this pointer.
        */
        const char* data() const;
        //! Get the memory allocation mode
        bool alloc() const { return alloc_; }
        //@}

        //! Return the size in bytes of one element of this type
        long typeSize() const
            { return TypeInfo::typeSize(TypeId(type_)); }
        //! Return the name of the type
        const char* typeName() const 
            { return TypeInfo::typeName(TypeId(type_)); }

        /*!
          @brief Set the offset. If the size of the data is not greater than
          four, the offset is written into the offset field as an
          unsigned long using the byte order given to encode it.
        */
        void setOffset(uint32 offset, ByteOrder byteOrder);
        /*!
          @brief Set type, count and the data of the entry. 
          @throw Error ("Size too large") if no memory allocation is allowed and
                 the size of the data in buf is greater than the existing size
                 of the data of the entry.
         */
        void setValue(uint16 type, const char* buf, long size);

    private:
        /*!
          @brief True: Requires memory allocation and deallocation,<BR>
                 False: No memory management needed.
        */
        bool alloc_;
        IfdId ifdId_;        // Redundant IFD id (it is also at the IFD)
        int ifdIdx_;         // Position in the IFD
        uint16 tag_;         // Tag
        uint16 type_;        // Type
        uint32 count_;       // Number of components
        uint32 offset_;      // Offset from the start of the IFD,
        // 0 if size <=4, i.e., if there is no offset
        char offsetData_[4]; // Data from the offset field if size <= 4
        long size_;          // Size of the data in bytes
        char* data_;         // Pointer to the data buffer
    }; // class Entry

    //! Container type to hold all IFD directory entries
    typedef std::vector<Entry> Entries;

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

      This class operates in two modes, one that allocates and deallocates the
      memory required to store the data, and one that doesn't perform such
      memory management and which is suitable in the case where a global data
      buffer is available and only pointers into this buffer need to be
      remembered.  Note that the different modes imply completely different copy
      and assignment behaviours, with the first resulting in entirely separate
      classes and the second mode resulting in multiple classes using one and
      the same data buffer. Use the default mode (with memory management) if
      possible. <BR>
      The mode without memory management is used to make "non-intrusive write
      support" possible. This allows writing to %Exif data of an image without
      changing the data layout of the %Exif data, to maximize chances that tag
      data, which the %Exif reader may not understand (e.g., the Makernote)
      remains valid. A "non-intrusive write operation" is the modification of 
      tag data without increasing the data size. 
    */
    class Ifd {
    public:
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

        //! Entries const iterator type
        typedef Entries::const_iterator const_iterator;
        //! Entries iterator type
        typedef Entries::iterator iterator;
        //! The first entry
        const_iterator begin() const { return entries_.begin(); }
        //! End of the entries
        const_iterator end() const { return entries_.end(); }
        //! The first entry
        iterator begin() { return entries_.begin(); }
        //! End of the entries
        iterator end() { return entries_.end(); }
        //! Find an IFD entry by tag, return a const iterator into the entries list
        const_iterator findTag(uint16 tag) const;
        //! Find an IFD entry by tag, return an iterator into the entries list
        iterator findTag(uint16 tag);
        //! Sort the IFD entries by tag
        void sortByTag();
        //! Delete the directory entry with the given tag
        void erase(uint16 tag);
        //! Delete the directory entry at iterator position pos
        void erase(iterator pos);
        /*!
          @brief Set the offset of the entry identified by tag. If no entry with
                 this tag exists, an entry of type unsigned long with one
                 component is created. If the size of the data is greater than
                 four, the offset of the entry is set to the value provided in
                 offset, else it is written to the offset field of the entry as
                 an unsigned long, encoded according to the byte order.
         */
        void setOffset(uint16 tag, uint32 offset, ByteOrder byteOrder);
        //! Set the offset of the next IFD
        void setNext(uint32 next) { next_ = next; }
        /*!
          @brief Add the Entry to the IFD. Checks for duplicates: if an entry
                 with the same tag already exists, the entry is overwritten. The
                 memory allocation mode of the entry to be added must match that
                 of the IFD and the IFD ids of the IFD and Entry must match.
         */
        void add(const Entry& entry);
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
          @brief Copy the IFD to a data array, return the number of bytes
                 written. 

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
          @return       Returns the number of characters written.
         */
        long copy(char* buf, ByteOrder byteOrder, long offset =0) const;
        //! @name Accessors
        //@{
        //! Ifd id of the IFD
        IfdId ifdId() const { return ifdId_; }
        //! Offset of the IFD from SOI
        long offset() const { return offset_; }
        //! Get the offset to the next IFD from the start of the TIFF header
        long next() const { return next_; }
        //! Get the memory allocation mode
        bool alloc() const { return alloc_; }
        //@}
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

    private:
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
      @brief Compare two 'raw' IFD entries by offset, taking care of special
             cases where one or both of the entries don't have an offset.
             Return true if the offset of entry lhs is less than that of rhs,
             else false. By definition, entries without an offset are greater
             than those with an offset.
     */
    bool cmpRawEntriesByOffset(const RawEntry& lhs, const RawEntry& rhs);        
    /*!
      @brief Compare two IFD entries by tag. Return true if the tag of entry
             lhs is less than that of rhs.
     */
    bool cmpEntriesByTag(const Entry& lhs, const Entry& rhs);
   
}                                       // namespace Exif

#endif                                  // #ifndef IFD_HPP_
