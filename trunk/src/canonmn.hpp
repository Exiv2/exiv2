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
  @file    canonmn.hpp
  @brief   Canon MakerNote implemented according to the specification
           "EXIF MakerNote of Canon" <http://www.burren.cx/david/canon.html>
           by David Burren
  @version $Name:  $ $Revision: 1.3 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    18-Feb-04, ahu: created
           07-Mar-04, ahu: isolated as a separate component
 */
#ifndef CANONMN_HPP_
#define CANONMN_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "makernote.hpp"

// + standard includes
#include <string>
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exif {

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
    MakerNote* createCanonMakerNote(bool alloc =true);

// *****************************************************************************
// class definitions

    //! MakerNote for Canon cameras
    class CanonMakerNote : public IfdMakerNote {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        CanonMakerNote(bool alloc =true);
        //! Virtual destructor
        virtual ~CanonMakerNote() {}
        //@}

        //! @name Accessors
        //@{        
        MakerNote* clone(bool alloc =true) const;
        //! Return the name of the makernote section ("Canon")
        std::string sectionName(uint16 tag) const { return sectionName_; }
        std::ostream& printTag(std::ostream& os,
                               uint16 tag, 
                               const Value& value) const;
        //@}

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
        //! Structure used to auto-register the MakerNote.
        struct RegisterMakerNote {
            //! Default constructor
            RegisterMakerNote() 
            {
                MakerNoteFactory& mnf = MakerNoteFactory::instance();
                mnf.registerMakerNote("Canon", "*", createCanonMakerNote); 
            }
        };
        /*!
          The static member variable is (see note) initialized before main and
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

    }; // class CanonMakerNote

}                                       // namespace Exif

#endif                                  // #ifndef CANONMN_HPP_
