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
           <a href="http://www.burren.cx/david/canon.html">
           EXIF MakerNote of Canon</a> by David Burren
  @version $Name:  $ $Revision: 1.12 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    18-Feb-04, ahu: created<BR>
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
    MakerNote* createCanonMakerNote(bool alloc,
                                    const byte* buf, 
                                    long len, 
                                    ByteOrder byteOrder, 
                                    long offset);

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
        CanonMakerNote* clone(bool alloc =true) const;
        //! Return the name of the makernote item ("Canon")
        std::string ifdItem() const { return ifdItem_; }
        std::ostream& printTag(std::ostream& os,
                               uint16_t tag, 
                               const Value& value) const;
        //@}

        //! @name Print functions for Canon %MakerNote tags 
        //@{
        //! Print various camera settings, part 1 (uses print0x0001_XX functions)
        static std::ostream& print0x0001(std::ostream& os, const Value& value);
        //! Print various camera settings, part 2 (uses print0x0004_XX functions)
        static std::ostream& print0x0004(std::ostream& os, const Value& value);
        //! Print the image number
        static std::ostream& print0x0008(std::ostream& os, const Value& value);
        //! Print the serial number of the camera
        static std::ostream& print0x000c(std::ostream& os, const Value& value);
        //! Print EOS D30 custom functions
        static std::ostream& print0x000f(std::ostream& os, const Value& value);

        //! Macro mode
        static std::ostream& print0x0001_01(std::ostream& os, long l);
        //! Self timer
        static std::ostream& print0x0001_02(std::ostream& os, long l);
        //! Quality
        static std::ostream& print0x0001_03(std::ostream& os, long l);
        //! Flash mode
        static std::ostream& print0x0001_04(std::ostream& os, long l);
        //! Drive mode
        static std::ostream& print0x0001_05(std::ostream& os, long l);
        //! Focus mode (G1 seems to use field 32 in preference to this)
        static std::ostream& print0x0001_07(std::ostream& os, long l);
        //! Image size
        static std::ostream& print0x0001_10(std::ostream& os, long l);
        //! Easy shooting
        static std::ostream& print0x0001_11(std::ostream& os, long l);
        //! Digital zoom
        static std::ostream& print0x0001_12(std::ostream& os, long l);
        //! ISO
        static std::ostream& print0x0001_16(std::ostream& os, long l);
        //! Metering mode
        static std::ostream& print0x0001_17(std::ostream& os, long l);
        //! Focus type
        static std::ostream& print0x0001_18(std::ostream& os, long l);
        //! AF point selected
        static std::ostream& print0x0001_19(std::ostream& os, long l);
        //! Exposure mode
        static std::ostream& print0x0001_20(std::ostream& os, long l);
        //! Flash activity
        static std::ostream& print0x0001_28(std::ostream& os, long l);
        //! Flash details 
        static std::ostream& print0x0001_29(std::ostream& os, long l);
        //! Focus mode (G1 seems to use this in preference to field 7)
        static std::ostream& print0x0001_32(std::ostream& os, long l);
        //! Low, normal, high print function
        static std::ostream& print0x0001_lnh(std::ostream& os, long l);
        //! Camera lens information
        static std::ostream& print0x0001_Lens(std::ostream& os, 
                                              const Value& value);
        //! White balance
        static std::ostream& print0x0004_07(std::ostream& os, long l);
        //! Sequence number
        static std::ostream& print0x0004_09(std::ostream& os, long l);
        //! AF point used
        static std::ostream& print0x0004_14(std::ostream& os, long l);
        //! Flash bias
        static std::ostream& print0x0004_15(std::ostream& os, long l);
        //! Subject distance
        static std::ostream& print0x0004_19(std::ostream& os, long l);
        //@}

    private:
        //! Structure used to auto-register the MakerNote.
        struct RegisterMakerNote {
            //! Default constructor
            RegisterMakerNote() 
            {
                MakerNoteFactory& mnf = MakerNoteFactory::instance();
                mnf.registerMakerNote("Canon", "*", createCanonMakerNote); 
                mnf.registerMakerNote(new CanonMakerNote);
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

        //! The item name (second part of the key) used for makernote tags
        std::string ifdItem_;

    }; // class CanonMakerNote

}                                       // namespace Exiv2

#endif                                  // #ifndef CANONMN_HPP_
