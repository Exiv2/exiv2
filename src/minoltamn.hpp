// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006 Andreas Huggel <ahuggel@gmx.net>
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
  @file    minoltamn.hpp
  @brief   Minolta MakerNote implemented using the following references:
           <a href="http://www.dalibor.cz/minolta/makernote.htm">Minolta Makernote Format Specification</a> by Dalibor Jelinek, 
           <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/Minolta.html">Minolta Makernote list by Phil Harvey</a>
           email communication with <a href="mailto:caulier dot gilles at kdemail dot net">caulier dot gilles at kdemail dot net</a>
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Gilles Caulier (gc)
           <a href="mailto:caulier dot gilles at kdemail dot net">caulier dot gilles at kdemail dot net</a>
  @date    06-May-06, gc: submitted
 */
#ifndef MINOLTAMN_HPP_
#define MINOLTAMN_HPP_

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
    MakerNote::AutoPtr createMinoltaMakerNote(bool alloc,
                                              const byte* buf,
                                              long len,
                                              ByteOrder byteOrder,
                                              long offset);

// *****************************************************************************
// class definitions

    //! MakerNote for Minolta cameras
    class MinoltaMakerNote : public IfdMakerNote {
    public:
        //! Shortcut for a %MinoltaMakerNote auto pointer.
        typedef std::auto_ptr<MinoltaMakerNote> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        MinoltaMakerNote(bool alloc =true);
        //! Copy constructor
        MinoltaMakerNote(const MinoltaMakerNote& rhs);
        //! Virtual destructor
        virtual ~MinoltaMakerNote() {}
        //@}

        //! @name Accessors
        //@{
        AutoPtr create(bool alloc =true) const;
        AutoPtr clone() const;
        //@}

        //! @cond IGNORE
        // Public only so that we can create a static instance
        struct RegisterMn {
            RegisterMn();
        };
        //! @endcond

    private:
        //! Internal virtual create function.
        MinoltaMakerNote* create_(bool alloc =true) const;
        //! Internal virtual copy constructor.
        MinoltaMakerNote* clone_() const;

        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class MinoltaMakerNote

    static MinoltaMakerNote::RegisterMn registerMinoltaMakerNote;
}                                       // namespace Exiv2

#endif                                  // #ifndef MINOLTAMN_HPP_
