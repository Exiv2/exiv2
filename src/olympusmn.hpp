// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2005 Andreas Huggel <ahuggel@gmx.net>
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
  @file    olympusmn.hpp
  @brief   Olympus MakerNote implemented according to the specification
           <a href="http://?????">
           Olympus MakerNote Documentation</a> by ???.           
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Will Stokes (wuz)
           <a href="mailto:wstokes@gmail.com">wstokes@gmail.com</a>
  @date    10-Mar-05, wuz: created
 */
#ifndef OLYMPUSMN_HPP_
#define OLYMPUSMN_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "makernote.hpp"
#include "tags.hpp"

// + standard includes
#include <string>
#include <iosfwd>
#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class Value;

// *****************************************************************************
// free functions

    /*!
      @brief Return an auto-pointer to a newly created empty MakerNote
             initialized to operate in the memory management model indicated.
             The caller owns this copy and the auto-pointer ensures that it
             will be deleted.
      
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
      
      @return An auto-pointer to a newly created empty MakerNote. The caller
             owns this copy and the auto-pointer ensures that it will be
             deleted.
     */
    MakerNote::AutoPtr createOlympusMakerNote(bool alloc,
                                              const byte* buf, 
                                              long len, 
                                              ByteOrder byteOrder, 
                                              long offset);

// *****************************************************************************
// class definitions

    //! MakerNote for Olympus cameras
    class OlympusMakerNote : public IfdMakerNote {
    public:
        //! Shortcut for a %OlympusMakerNote auto pointer.
        typedef std::auto_ptr<OlympusMakerNote> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        OlympusMakerNote(bool alloc =true);
        //! Copy constructor
        OlympusMakerNote(const OlympusMakerNote& rhs);
        //! Virtual destructor
        virtual ~OlympusMakerNote() {}
        //@}

        //! @name Manipulators
        //@{        
        int readHeader(const byte* buf, 
                       long len,
                       ByteOrder byteOrder);
        //@}

        //! @name Accessors
        //@{
        int checkHeader() const;
        AutoPtr create(bool alloc =true) const;
        AutoPtr clone() const;
        //@}

        //! @name Print functions for Olympus %MakerNote tags 
        //@{
        //! Print various camera settings (uses print0x0f00_XXX functions)
        static std::ostream& print0x0f00(std::ostream& os, const Value& value);

        //! Function
        static std::ostream& print0x0f00_011(std::ostream& os, long l);
        //! White Balance
        static std::ostream& print0x0f00_138(std::ostream& os, long l);
        //! Sharpness
        static std::ostream& print0x0f00_150_151(std::ostream& os, long l1, long l2);
        //@}

    private:
        //! Internal virtual create function.
        OlympusMakerNote* create_(bool alloc =true) const;
        //! Internal virtual copy constructor.
        OlympusMakerNote* clone_() const;

        //! Tag information
        static const TagInfo tagInfo_[];

        //! Structure used to auto-register the MakerNote.
        struct RegisterMakerNote {
            //! Default constructor
            RegisterMakerNote() 
            {
                MakerNoteFactory& mnf = MakerNoteFactory::instance();
                mnf.registerMakerNote("OLYMPUS*", "*", createOlympusMakerNote); 
                mnf.registerMakerNote(olympusIfdId,
                                      MakerNote::AutoPtr(new OlympusMakerNote));
                ExifTags::registerMakerTagInfo(olympusIfdId, tagInfo_);
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

    }; // class OlympusMakerNote

}                                       // namespace Exiv2

#endif                                  // #ifndef OLYMPUSMN_HPP_
