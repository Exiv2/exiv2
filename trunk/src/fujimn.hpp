// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005 Andreas Huggel <ahuggel@gmx.net>
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
  @version $Rev$
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
    MakerNote::AutoPtr createFujiMakerNote(bool alloc,
                                           const byte* buf,
                                           long len,
                                           ByteOrder byteOrder,
                                           long offset);

// *****************************************************************************
// class definitions

    //! MakerNote for Fujifilm cameras
    class FujiMakerNote : public IfdMakerNote {
    public:
        //! Shortcut for a %FujiMakerNote auto pointer.
        typedef std::auto_ptr<FujiMakerNote> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        FujiMakerNote(bool alloc =true);
        //! Copy constructor
        FujiMakerNote(const FujiMakerNote& rhs);
        //! Virtual destructor
        virtual ~FujiMakerNote() {}
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

        //! @cond IGNORE
        // Public only so that we can create a static instance
        struct RegisterMn {
            RegisterMn();
        };
        //! @endcond

    private:
        //! Internal virtual create function.
        FujiMakerNote* create_(bool alloc =true) const;
        //! Internal virtual copy constructor.
        FujiMakerNote* clone_() const;

        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class FujiMakerNote

    static FujiMakerNote::RegisterMn registerFujiMakerNote;
}                                       // namespace Exiv2

#endif                                  // #ifndef FUJIMN_HPP_
