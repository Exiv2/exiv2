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
  @brief   
  @version $Name:  $ $Revision: 1.2 $
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
#include <utility>
#include <vector>

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

        //! Constructor. Takes an optional MakerNote info tag array.
        MakerNote(const MnTagInfo* mnTagInfo =0) : mnTagInfo_(mnTagInfo) {}
        //! Virtual destructor.
        virtual ~MakerNote() {}
        /*!
          @brief Return a pointer to a copy of itself (deep copy).
                 The caller owns this copy and is responsible to delete it!
         */
        virtual MakerNote* clone() const =0;
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
                 in byte order byteOrder. Return the number of bytes written.
         */
        virtual long copy(char* buf, ByteOrder byteOrder, long offset) const =0;

        //! @name Accessors
        //@{
        //! Return the size of the makernote in bytes.
        virtual long size() const =0;
        //! The first %MakerNote entry
        virtual Entries::const_iterator begin() const =0;
        //! End of the %MakerNote entries
        virtual Entries::const_iterator end() const =0;
        //! The first %MakerNote entry
        virtual Entries::iterator begin() =0;
        //! End of the %MakerNote entries
        virtual Entries::iterator end() =0;
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
      @brief %MakerNote factory.

      Creates an instance of the %MakerNote for one camera model. The factory is
      implemented as a singleton, which can be accessed only through the static
      member function instance().
    */
    class MakerNoteFactory {
    public:
        /*!
          @brief Access the makerNote factory. Clients access the task factory
                 exclusively through this method.
        */
        static MakerNoteFactory& instance();

        /*!
          @brief Create the appropriate %MakerNote based on camera make and
                 model, return a pointer to the newly created MakerNote
                 instance. Return 0 if no %MakerNote is defined for the camera
                 model.

          The method searches the make-model tree for a make and model
          combination in the registry that matches the search key. The search is
          case insensitive (Todo: implement case-insensitive comparisons) and
          wildcards in the registry entries are supported. First the best
          matching make is searched, then the best matching model for this make
          is searched. If there is no matching make or no matching model within
          the models registered for the best matching make, then no maker note
          is created and the function returns 0. If a match is found, the
          function returns a pointer to a clone of the registered prototype. The
          maker note pointed to is owned by the caller of the function, i.e.,
          the caller is responsible to delete the returned make note when it is
          no longer needed.
          The best match is the match with the most matching characters.

          @param make Camera manufacturer. (Typically the string from the %Exif
                 make tag.)
          @param model Camera model. (Typically the string from the %Exif
                 model tag.)
          @return A pointer that owns a %MakerNote for the camera model.  If
                 the camera is not supported, the pointer is 0.
         */
        MakerNote* create(const std::string& make, 
                          const std::string& model) const;

        /*!
          @brief Register a %MakerNote prototype for a camera make and model.

          Registers a %MakerNote for a given make and model combination with the
          factory. Both the make and model strings may contain wildcards ('*',
          e.g., "Canon*").  The method adds a new makerNote pointer to the
          registry with the make and model strings provided. It takes ownership
          of the object pointed to by the maker note pointer provided. If the
          make already exists, then a new branch for the model is added to the
          registry. If the model also already exists, then the new makerNote
          pointer replaces the old one and the maker note pointed to by the old
          pointer is deleted.

          @param make Camera manufacturer. (Typically the string from the %Exif
                 make tag.)
          @param model Camera model. (Typically the string from the %Exif
                 model tag.)
          @param makerNote Pointer to the prototype. Ownership is transfered to the
                 %MakerNote factory.
        */
        void registerMakerNote(const std::string& make, 
                               const std::string& model, 
                               MakerNote* makerNote);
        /*!
          @brief Match a registry entry with a key (used for make and model).

          The matching algorithm is case insensitive and wildcards ('*') in the
          registry entry are supported. The best match is the match with the
          most matching characters.

          @return A pair of which the first component indicates whether or not
                  the key matches and the second component contains the number
                  of matching characters.
         */
        static std::pair<bool, int> match(const std::string& regEntry, 
                                          const std::string& key);

    private:
        //! Prevent construction other than through instance().
        MakerNoteFactory();
        //! Prevent copy construction: not implemented.
        MakerNoteFactory(const MakerNoteFactory& rhs);

        //! Pointer to the one and only instance of this class.
        static MakerNoteFactory* instance_;
        //! Type used to store model labels and %MakerNote prototype classes
        typedef std::vector<std::pair<std::string, MakerNote*> > ModelRegistry;
        //! Type used to store a list of make labels and model registries
        typedef std::vector<std::pair<std::string, ModelRegistry*> > Registry;
        //! List of makernote types and corresponding prototypes.
        Registry registry_;

    }; // class MakerNoteFactory

    /*!
      @brief Interface for MakerNotes in IFD format

      Todo: Allow for a 'prefix' before the IFD (OLYMP, etc)
            Cater for offsets from start of TIFF header as well as relative to Mn
     */
    class IfdMakerNote : public MakerNote {
    public:
        //! Constructor. Takes an optional MakerNote info tag array.
        IfdMakerNote(const MakerNote::MnTagInfo* mnTagInfo =0)
            : MakerNote(mnTagInfo), ifd_(makerIfd, 0, false) {}
        virtual ~IfdMakerNote() {}
        virtual MakerNote* clone() const =0;

        int read(const char* buf, long len, ByteOrder byteOrder, long offset);
        long copy(char* buf, ByteOrder byteOrder, long offset) const;
        long size() const;
        Entries::const_iterator begin() const { return ifd_.begin(); }
        Entries::const_iterator end() const { return ifd_.end(); }
        Entries::iterator begin() { return ifd_.begin(); }
        Entries::iterator end() { return ifd_.end(); }
        virtual std::string sectionName(uint16 tag) const =0; 
        virtual std::ostream& printTag(std::ostream& os,
                                       uint16 tag, 
                                       const Value& value) const =0;

    protected:
        Ifd ifd_;                               //!< MakerNote IFD

    }; // class IfdMakerNote

    //! MakerNote for Canon cameras
    class CanonMakerNote : public IfdMakerNote {
    public:
        //! Default constructor
        CanonMakerNote();
        virtual ~CanonMakerNote() {}
        MakerNote* clone() const;
        //! Return the name of the makernote section ("Canon")
        std::string sectionName(uint16 tag) const { return sectionName_; }
        std::ostream& printTag(std::ostream& os,
                               uint16 tag, 
                               const Value& value) const;

        //! @name Print functions for Canon %MakerNote tags 
        //@{
        //! Print various camera settings, part 1
        std::ostream& print0x0001(std::ostream& os, const Value& value) const;
        //! Print various camera settings, part 2
        std::ostream& print0x0004(std::ostream& os, const Value& value) const;
        //! Print the image number
        std::ostream& print0x0008(std::ostream& os, const Value& value) const;
        //! Print the serial number of the camera
        std::ostream& print0x000c(std::ostream& os, const Value& value) const;
        //! Print EOS D30 custom functions
        std::ostream& print0x000f(std::ostream& os, const Value& value) const;
        //@}

    private:
        std::string sectionName_;

    }; // class CanonMakerNote
   
}                                       // namespace Exif

#endif                                  // #ifndef MAKERNOTE_HPP_
