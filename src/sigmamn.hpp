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
  @file    sigmamn.hpp
  @brief   Sigma and Foveon MakerNote implemented according to the specification
           in "SIGMA and FOVEON EXIF MakerNote Documentation" by Foveon.
           <http://www.x3f.info/technotes/FileDocs/MakerNoteDoc.html>
  @version $Name:  $ $Revision: 1.3 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    02-Apr-04, ahu: created
 */
#ifndef SIGMAMN_HPP_
#define SIGMAMN_HPP_

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
    MakerNote* createSigmaMakerNote(bool alloc =true);

// *****************************************************************************
// class definitions

    //! MakerNote for Sigma (Foveon) cameras
    class SigmaMakerNote : public IfdMakerNote {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        SigmaMakerNote(bool alloc =true);
        //! Virtual destructor
        virtual ~SigmaMakerNote() {}
        //@}

        //! @name Accessors
        //@{        
        MakerNote* clone(bool alloc =true) const;
        //! Return the name of the makernote section ("Sigma")
        std::string sectionName(uint16 tag) const { return sectionName_; }
        std::ostream& printTag(std::ostream& os,
                               uint16 tag, 
                               const Value& value) const;
        //@}

        //! @name Print functions for Sigma (Foveon) %MakerNote tags 
        //@{
        //! Strip the label from the value and print the remainder
        static std::ostream& printStripLabel(std::ostream& os, const Value& value);
        //! Print exposure mode
        static std::ostream& print0x0008(std::ostream& os, const Value& value);
        //! Print metering mode
        static std::ostream& print0x0009(std::ostream& os, const Value& value);
        //@}

    private:
        //! Structure used to auto-register the MakerNote.
        struct RegisterMakerNote {
            //! Default constructor
            RegisterMakerNote() 
            {
                MakerNoteFactory& mnf = MakerNoteFactory::instance();
                mnf.registerMakerNote("SIGMA", "*", createSigmaMakerNote); 
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

    }; // class SigmaMakerNote

}                                       // namespace Exiv2

#endif                                  // #ifndef SIGMAMN_HPP_
