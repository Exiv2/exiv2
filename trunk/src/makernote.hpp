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
  @file    makernote.hpp
  @brief   Defines the %Exif %MakerNote interface and an IFD MakerNote
  @version $Name:  $ $Revision: 1.5 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    18-Feb-04, ahu: created
 */
#ifndef MAKERNOTE_HPP_
#define MAKERNOTE_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "ifd.hpp"

// + standard includes
#include <string>
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exif {

    class Value;

// *****************************************************************************
// class definitions

    /*!
      @brief %Exif makernote interface

      Defines methods to
      - read the makernote from a character buffer
      - copy the makernote to a character buffer
      - add the makernote tags to the %Exif metadata
      - access Makernote specific tag descriptions and print functions

      Todo: Synchronization with ExifData:
            Add a MakerNote pointer to ExifData (owner) and Metadata
            Instead of ExifData as the owner, there should be a reference counter
            Does Entry need a MakerNote poiner too? (not nice cos of the circ deps)
     */
    class MakerNote {
    public:

        //! MakerNote Tag information
        struct MnTagInfo {
            //! Constructor
            MnTagInfo(uint16 tag, const char* name, const char* desc) 
                : tag_(tag), name_(name), desc_(desc) {}

            uint16 tag_;                //!< Tag
            const char* name_;          //!< One word tag label
            const char* desc_;          //!< Short tag description
        }; // struct MnTagInfo

        //! @name Creators
        //@{
        //! Constructor. Takes an optional MakerNote info tag array.
        MakerNote(const MnTagInfo* mnTagInfo =0) : mnTagInfo_(mnTagInfo) {}
        //! Virtual destructor.
        virtual ~MakerNote() {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the MakerNote from character buffer buf of length len at
                 position offset (from the start of the TIFF header) and encoded
                 in byte order byteOrder.
         */
        virtual int read(const char* buf, 
                         long len, 
                         ByteOrder byteOrder,
                         long offset) =0;
        /*!
          @brief Copy (write) the makerNote to the character buffer buf at 
                 position offset (from the start of the TIFF header), encoded
                 in byte order byteOrder. Update internal offsets if necessary.
                 Return the number of bytes written.
         */
        virtual long copy(char* buf, ByteOrder byteOrder, long offset) =0;
        //! The first %MakerNote entry
        virtual Entries::iterator begin() =0;
        //! End of the %MakerNote entries
        virtual Entries::iterator end() =0;
        //@}

        //! @name Accessors
        //@{
        //! Return the key for the tag.
        std::string makeKey(uint16 tag) const;
        //! Return the associated tag for a makernote key.
        uint16 decomposeKey(const std::string& key) const;
        /*!
          @brief Return the name of a makernote tag. The default implementation
                 looks up the %MakerNote info tag array if one is set, else
                 it translates the tag to a string with the hexadecimal value of
                 the tag.
         */
        virtual std::string tagName(uint16 tag) const;
        /*!
          @brief Return the tag associated with a makernote tag name. The
                 default implementation looks up the %MakerNote info tag array
                 if one is set, else it expects tag names in the form \"0x01ff\"
                 and converts them to unsigned integer.
         */
        virtual uint16 tag(const std::string& tagName) const;
        /*!
          @brief Return a pointer to a copy of itself (deep copy).
                 The caller owns this copy and is responsible to delete it!
         */
        virtual MakerNote* clone() const =0;
        //! The first %MakerNote entry
        virtual Entries::const_iterator begin() const =0;
        //! End of the %MakerNote entries
        virtual Entries::const_iterator end() const =0;
        //! Find an entry by idx, return a const iterator to the record
        virtual Entries::const_iterator findIdx(int idx) const =0;
        //! Return the size of the makernote in bytes.
        virtual long size() const =0;
        //! Return the name of the makernote section
        virtual std::string sectionName(uint16 tag) const =0; 
        //! Interpret and print the value of a makernote tag
        virtual std::ostream& printTag(std::ostream& os,
                                       uint16 tag, 
                                       const Value& value) const =0;
        //@}

    protected:
        //! Pointer to an array of MakerNote tag infos
        const MnTagInfo* mnTagInfo_;   

    }; // class MakerNote

    /*!
      @brief Interface for MakerNotes in IFD format

      Todo: Allow for a 'prefix' before the IFD (OLYMP, etc)
            Cater for offsets from start of TIFF header as well as relative to Mn
     */
    class IfdMakerNote : public MakerNote {
    public:
        //! @name Creators
        //@{        
        //! Constructor. Takes an optional MakerNote info tag array.
        IfdMakerNote(const MakerNote::MnTagInfo* mnTagInfo =0)
            : MakerNote(mnTagInfo), ifd_(makerIfd, 0, false) {}
        //! Virtual destructor
        virtual ~IfdMakerNote() {}
        //@}

        //! @name Manipulators
        //@{
        int read(const char* buf, long len, ByteOrder byteOrder, long offset);
        long copy(char* buf, ByteOrder byteOrder, long offset);
        Entries::iterator begin() { return ifd_.begin(); }
        Entries::iterator end() { return ifd_.end(); }
        //@}

        //! @name Accessors
        //@{
        Entries::const_iterator begin() const { return ifd_.begin(); }
        Entries::const_iterator end() const { return ifd_.end(); }
        Entries::const_iterator findIdx(int idx) const 
            { return ifd_.findIdx(idx); }
        long size() const;
        virtual MakerNote* clone() const =0;
        virtual std::string sectionName(uint16 tag) const =0; 
        virtual std::ostream& printTag(std::ostream& os,
                                       uint16 tag, 
                                       const Value& value) const =0;
        //@}

    protected:
        Ifd ifd_;                               //!< MakerNote IFD

    }; // class IfdMakerNote
   
}                                       // namespace Exif

#endif                                  // #ifndef MAKERNOTE_HPP_
