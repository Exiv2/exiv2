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
  @file    nikon2mn.hpp
  @brief   A second Nikon MakerNote format, implemented according to the 
           specification in Appendix 2: Makernote of Nikon of the document 
           <a href="http://park2.wakwak.com/%7Etsuruzoh/Computer/Digicams/exif-e.html">
           Exif file format</a> by TsuruZoh Tachibanaya
  @version $Name:  $ $Revision: 1.1 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    17-May-04, ahu: created
 */
#ifndef NIKON2MN_HPP_
#define NIKON2MN_HPP_

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
     */
    MakerNote* createNikon2MakerNote(bool alloc =true);

// *****************************************************************************
// class definitions

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

        //! @name Print functions for Nikon1 %MakerNote tags 
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
        //! Structure used to auto-register the MakerNote.
        struct RegisterMakerNote {
            //! Default constructor
            RegisterMakerNote() 
            {
                MakerNoteFactory& mnf = MakerNoteFactory::instance();
                mnf.registerMakerNote("NIKON", "E950", createNikon2MakerNote);
                mnf.registerMakerNote("NIKON", "*", createNikon2MakerNote);
            }
        };
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

    }; // class Nikon2MakerNote

}                                       // namespace Exiv2

#endif                                  // #ifndef NIKON2MN_HPP_
