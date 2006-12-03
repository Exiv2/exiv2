// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*!
  @file    olympusmn.hpp
  @brief   Olympus MakerNote implemented using the following references:
           <a href="http://park2.wakwak.com/%7Etsuruzoh/Computer/Digicams/exif-e.html#APP1">Exif file format, Appendix 1: MakerNote of Olympus Digicams</a> by TsuruZoh Tachibanaya,<br>
           Olympus.pm of <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/">ExifTool</a> by Phil Harvey,<br>
           <a href="http://www.ozhiker.com/electronics/pjmt/jpeg_info/olympus_mn.html">Olympus Makernote Format Specification</a> by Evan Hunter,<br>
           email communication with <a href="mailto:wstokes@gmail.com">Will Stokes</a>
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Will Stokes (wuz)
           <a href="mailto:wstokes@gmail.com">wstokes@gmail.com</a>
  @author  Gilles Caulier (gc)
           <a href="mailto:caulier dot gilles at kdemail dot net">caulier dot gilles at kdemail dot net</a>
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
        //! Return read-only list of built-in Olympus tags
        static const TagInfo* tagList();
        //@}

        //! @name Print functions for Olympus %MakerNote tags
        //@{
        //! Print 'Special Mode'
        static std::ostream& print0x0200(std::ostream& os, const Value& value);
        //! Print Digital Zoom Factor
        static std::ostream& print0x0204(std::ostream& os, const Value& value);
        //! Print White Balance Mode
        static std::ostream& print0x1015(std::ostream& os, const Value& value);
        //@}

        //! @cond IGNORE
        // Public only so that we can create a static instance
        struct RegisterMn {
            RegisterMn();
        };
        //! @endcond

    private:
        //! Internal virtual create function.
        OlympusMakerNote* create_(bool alloc =true) const;
        //! Internal virtual copy constructor.
        OlympusMakerNote* clone_() const;

        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class OlympusMakerNote

    static OlympusMakerNote::RegisterMn registerOlympusMakerNote;
}                                       // namespace Exiv2

#endif                                  // #ifndef OLYMPUSMN_HPP_
