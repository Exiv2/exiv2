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
  @file    nikonmn.hpp
  @brief   Nikon MakerNote formats implemented according to the following
           references:<BR>
           Format 1: <a href="http://www.tawbaware.com/990exif.htm">MakerNote 
           EXIF Tag of the Nikon 990</a> by Max Lyons.<BR>
           Format 2: "Appendix 2: Makernote of Nikon" of the document 
           <a href="http://park2.wakwak.com/%7Etsuruzoh/Computer/Digicams/exif-e.html">
           Exif file format</a> by TsuruZoh Tachibanaya.<BR>
           Format 3: "EXIFutils Field Reference Guide".
  @version $Name:  $ $Revision: 1.2 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    17-May-04, ahu: created
           25-May-04, ahu: combined all Nikon formats in one component
 */
#ifndef NIKONMN_HPP_
#define NIKONMN_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "makernote.hpp"

// + standard includes
#include <string>
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class Value;

// *****************************************************************************
// free functions

    /*!
      @brief Return a pointer to a newly created empty MakerNote initialized to
             operate in the memory management model indicated.  The caller owns
             this copy and is responsible to delete it!
      
      @param alloc Memory management model for the new MakerNote. Determines if
             memory required to store data should be allocated and deallocated
             (true) or not (false). If false, only pointers to the buffer
             provided to read() will be kept. See Ifd for more background on
             this concept.
      @param buf Pointer to the makernote character buffer (not used). 
      @param len Length of the makernote character buffer (not used). 
      @param byteOrder Byte order in which the Exif data (and possibly the 
             makernote) is encoded (not used).
      @param offset Offset from the start of the TIFF header of the makernote
             buffer (not used).
      
      @return A pointer to a newly created empty MakerNote. The caller owns
             this copy and is responsible to delete it!
     */
    MakerNote* createNikonMakerNote(bool alloc,
                                    const char* buf, 
                                    long len, 
                                    ByteOrder byteOrder, 
                                    long offset);

// *****************************************************************************
// class definitions

    //! A MakerNote format used by Nikon cameras, such as the E990 and D1.
    class Nikon1MakerNote : public IfdMakerNote {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        Nikon1MakerNote(bool alloc =true);
        //! Virtual destructor
        virtual ~Nikon1MakerNote() {}
        //@}

        //! @name Accessors
        //@{
        Nikon1MakerNote* clone(bool alloc =true) const;
        //! Return the name of the makernote section ("Nikon1")
        std::string sectionName(uint16 tag) const { return sectionName_; }
        std::ostream& printTag(std::ostream& os,
                               uint16 tag, 
                               const Value& value) const;
        //@}

        //! @name Print functions for Nikon1 %MakerNote tags 
        //@{
        //! Print ISO setting
        static std::ostream& print0x0002(std::ostream& os, const Value& value);
        //! Print autofocus mode
        static std::ostream& print0x0007(std::ostream& os, const Value& value);
        //! Print manual focus distance
        static std::ostream& print0x0085(std::ostream& os, const Value& value);
        //! Print digital zoom setting
        static std::ostream& print0x0086(std::ostream& os, const Value& value);
        //! Print AF focus position
        static std::ostream& print0x0088(std::ostream& os, const Value& value);
        //@}

    private:
        //! Structure used to auto-register the MakerNote.
        struct RegisterMakerNote {
            //! Default constructor
            RegisterMakerNote() 
            {
                MakerNoteFactory& mnf = MakerNoteFactory::instance();
                mnf.registerMakerNote("NIKON*", "*", createNikonMakerNote); 
            }
        };
        // DATA
        /*!
          The static member variable is initialized before main (see note) and
          will in the process register the MakerNote class. (Remember the
          definition of the variable in the implementation file!)

          @note The standard says that, if no function is explicitly called ever
                in a module, then that module's static data might be never
                initialized. This clause was introduced to allow dynamic link
                libraries. The idea is, with this clause the loader is not
                forced to eagerly load all modules, but load them only on
                demand.
         */
        static const RegisterMakerNote register_; 

        //! The section name (second part of the key) used for makernote tags
        std::string sectionName_;

    }; // class Nikon1MakerNote

    /*!
      @brief A second MakerNote format used by Nikon cameras, including the 
             E700, E800, E900, E900S, E910, E950
     */ 
    class Nikon2MakerNote : public IfdMakerNote {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        Nikon2MakerNote(bool alloc =true);
        //! Virtual destructor
        virtual ~Nikon2MakerNote() {}
        //@}

        //! @name Manipulators
        //@{        
        int readHeader(const char* buf, 
                       long len,
                       ByteOrder byteOrder);
        //@}

        //! @name Accessors
        //@{
        int checkHeader() const;
        Nikon2MakerNote* clone(bool alloc =true) const;
        //! Return the name of the makernote section ("Nikon2")
        std::string sectionName(uint16 tag) const { return sectionName_; }
        std::ostream& printTag(std::ostream& os,
                               uint16 tag, 
                               const Value& value) const;
        //@}

        //! @name Print functions for Nikon2 %MakerNote tags 
        //@{
        //! Print quality setting
        static std::ostream& print0x0003(std::ostream& os, const Value& value);
        //! Print color mode setting
        static std::ostream& print0x0004(std::ostream& os, const Value& value);
        //! Print image adjustment setting
        static std::ostream& print0x0005(std::ostream& os, const Value& value);
        //! Print ISO speed setting
        static std::ostream& print0x0006(std::ostream& os, const Value& value);
        //! Print white balance setting
        static std::ostream& print0x0007(std::ostream& os, const Value& value);
        //! Print digital zoom setting
        static std::ostream& print0x000a(std::ostream& os, const Value& value);
        //@}

    private:
        //! The section name (second part of the key) used for makernote tags
        std::string sectionName_;

    }; // class Nikon2MakerNote

    //! A third MakerNote format used by Nikon cameras, e.g., E5400, SQ, D2H, D70
    class Nikon3MakerNote : public IfdMakerNote {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        Nikon3MakerNote(bool alloc =true);
        //! Virtual destructor
        virtual ~Nikon3MakerNote() {}
        //@}

        //! @name Manipulators
        //@{        
        int readHeader(const char* buf, 
                       long len,
                       ByteOrder byteOrder);
        //@}

        //! @name Accessors
        //@{
        int checkHeader() const;
        Nikon3MakerNote* clone(bool alloc =true) const;
        //! Return the name of the makernote section ("Nikon3")
        std::string sectionName(uint16 tag) const { return sectionName_; }
        std::ostream& printTag(std::ostream& os,
                               uint16 tag, 
                               const Value& value) const;
        //@}

        //! @name Print functions for Nikon3 %MakerNote tags 
        //@{
        //! Print ISO setting
        static std::ostream& print0x0002(std::ostream& os, const Value& value);
        //! Print lens type
        static std::ostream& print0x0083(std::ostream& os, const Value& value);
        //! Print lens information
        static std::ostream& print0x0084(std::ostream& os, const Value& value);
        //! Print flash used information
        static std::ostream& print0x0087(std::ostream& os, const Value& value);
        //! Print bracketing information
        static std::ostream& print0x0089(std::ostream& os, const Value& value);
        //@}

    private:
        //! The section name (second part of the key) used for makernote tags
        std::string sectionName_;

    }; // class Nikon3MakerNote

}                                       // namespace Exiv2

#endif                                  // #ifndef NIKONMN_HPP_
