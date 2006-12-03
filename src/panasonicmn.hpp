// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
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
  @file    panasonicmn.hpp
  @brief   Panasonic MakerNote implemented using the following references:
           <a href="http://www.compton.nu/panasonic.html">Panasonic MakerNote Information</a> by Tom Hughes,
           Panasonic.pm of <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/">ExifTool</a> by Phil Harvey,
           <a href="http://www.ozhiker.com/electronics/pjmt/jpeg_info/panasonic_mn.html">Panasonic Makernote Format Specification</a> by Evan Hunter.
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Gilles Caulier (gc)
           <a href="mailto:caulier dot gilles at kdemail dot net">caulier dot gilles at kdemail dot net</a>
  @date    11-Jun-05, ahu: created
 */
#ifndef PANASONICMN_HPP_
#define PANASONICMN_HPP_

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
    MakerNote::AutoPtr createPanasonicMakerNote(bool alloc,
                                                const byte* buf,
                                                long len,
                                                ByteOrder byteOrder,
                                                long offset);

// *****************************************************************************
// class definitions

    //! MakerNote for Panasonic cameras
    class PanasonicMakerNote : public IfdMakerNote {
    public:
        //! Shortcut for a %PanasonicMakerNote auto pointer.
        typedef std::auto_ptr<PanasonicMakerNote> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        PanasonicMakerNote(bool alloc =true);
        //! Copy constructor
        PanasonicMakerNote(const PanasonicMakerNote& rhs);
        //! Virtual destructor
        virtual ~PanasonicMakerNote() {}
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
        //! Return read-only list of built-in Panasonic tags
        static const TagInfo* tagList();
        //@}

        //! @name Print functions for Panasonic %MakerNote tags
        //@{
        //! Print SpotMode
        static std::ostream& print0x000f(std::ostream& os, const Value& value);
        //! Print WhiteBalanceBias
        static std::ostream& print0x0023(std::ostream& os, const Value& value);
        //@}

        //! @cond IGNORE
        // Public only so that we can create a static instance
        struct RegisterMn {
            RegisterMn();
        };
        //! @endcond

    private:
        //! Internal virtual create function.
        PanasonicMakerNote* create_(bool alloc =true) const;
        //! Internal virtual copy constructor.
        PanasonicMakerNote* clone_() const;

        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class PanasonicMakerNote

    static PanasonicMakerNote::RegisterMn registerPanasonicMakerNote;
}                                       // namespace Exiv2

#endif                                  // #ifndef PANASONICMN_HPP_
