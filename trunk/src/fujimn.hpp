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
  @file    fujimn.hpp
  @brief   Fujifilm MakerNote implemented according to the specification
           in Appendix 4: Makernote of Fujifilm of the document 
           <a href="http://park2.wakwak.com/%7Etsuruzoh/Computer/Digicams/exif-e.html">
           Exif file format</a> by TsuruZoh Tachibanaya
  @version $Name:  $ $Revision: 1.4 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    11-Feb-04, ahu: created
 */
#ifndef FUJIMN_HPP_
#define FUJIMN_HPP_

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
    MakerNote* createFujiMakerNote(bool alloc =true);

// *****************************************************************************
// class definitions

    //! MakerNote for Fujifilm cameras
    class FujiMakerNote : public IfdMakerNote {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        FujiMakerNote(bool alloc =true);
        //! Virtual destructor
        virtual ~FujiMakerNote() {}
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
        FujiMakerNote* clone(bool alloc =true) const;
        //! Return the name of the makernote section ("Fujifilm")
        std::string sectionName(uint16 tag) const { return sectionName_; }
        std::ostream& printTag(std::ostream& os,
                               uint16 tag, 
                               const Value& value) const;
        //@}

        //! @name Print functions for Fujifilm %MakerNote tags 
        //@{
        //! Print Off or On status
        static std::ostream& printOffOn(std::ostream& os, const Value& value);
        //! Print sharpness
        static std::ostream& print0x1001(std::ostream& os, const Value& value);
        //! Print white balance
        static std::ostream& print0x1002(std::ostream& os, const Value& value);
        //! Print color
        static std::ostream& print0x1003(std::ostream& os, const Value& value);
        //! Print tone
        static std::ostream& print0x1004(std::ostream& os, const Value& value);
        //! Print flash mode
        static std::ostream& print0x1010(std::ostream& os, const Value& value);
        //! Print focus mode
        static std::ostream& print0x1021(std::ostream& os, const Value& value);
        //! Print picture mode
        static std::ostream& print0x1031(std::ostream& os, const Value& value);
        //@}

    private:
        //! Structure used to auto-register the MakerNote.
        struct RegisterMakerNote {
            //! Default constructor
            RegisterMakerNote() 
            {
                MakerNoteFactory& mnf = MakerNoteFactory::instance();
                mnf.registerMakerNote("FUJIFILM", "*", createFujiMakerNote); 
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

    }; // class FujiMakerNote

}                                       // namespace Exiv2

#endif                                  // #ifndef FUJIMN_HPP_
