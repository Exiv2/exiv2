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
  @file    nikon3mn.hpp
  @brief   Nikon MakerNote, only format implemented, no specification is 
           available.
  @version $Name:  $ $Revision: 1.1 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    17-May-04, ahu: created
 */
#ifndef NIKON3MN_HPP_
#define NIKON3MN_HPP_

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
    MakerNote* createNikon3MakerNote(bool alloc =true);

// *****************************************************************************
// class definitions

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

        //! @name Print functions for Nikon1 %MakerNote tags 
        //@{
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
        //! Structure used to auto-register the MakerNote.
        struct RegisterMakerNote {
            //! Default constructor
            RegisterMakerNote() 
            {
                MakerNoteFactory& mnf = MakerNoteFactory::instance();
                mnf.registerMakerNote("NIKON", "E5400", createNikon3MakerNote);
                mnf.registerMakerNote("NIKON", "SQ  ", createNikon3MakerNote);
                mnf.registerMakerNote("NIKON CORPORATION", "NIKON D2H", createNikon3MakerNote);
                mnf.registerMakerNote("NIKON CORPORATION", "NIKON D70", createNikon3MakerNote);
                mnf.registerMakerNote("NIKON CORPORATION", "*", createNikon3MakerNote);
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

    }; // class Nikon3MakerNote

}                                       // namespace Exiv2

#endif                                  // #ifndef NIKON3MN_HPP_
