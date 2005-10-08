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
  @file    sigmamn.hpp
  @brief   Sigma and Foveon MakerNote implemented according to the specification
           <a href="http://www.x3f.info/technotes/FileDocs/MakerNoteDoc.html">
           SIGMA and FOVEON EXIF MakerNote Documentation</a> by Foveon.
  @version $Rev$
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
    MakerNote::AutoPtr createSigmaMakerNote(bool alloc,
                                            const byte* buf,
                                            long len,
                                            ByteOrder byteOrder,
                                            long offset);

// *****************************************************************************
// class definitions

    //! MakerNote for Sigma (Foveon) cameras
    class SigmaMakerNote : public IfdMakerNote {
    public:
        //! Shortcut for a %SigmaMakerNote auto pointer.
        typedef std::auto_ptr<SigmaMakerNote> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        SigmaMakerNote(bool alloc =true);
        //! Copy constructor
        SigmaMakerNote(const SigmaMakerNote& rhs);
        //! Virtual destructor
        virtual ~SigmaMakerNote() {}
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

        //! @name Print functions for Sigma (Foveon) %MakerNote tags
        //@{
        //! Strip the label from the value and print the remainder
        static std::ostream& printStripLabel(std::ostream& os, const Value& value);
        //! Print exposure mode
        static std::ostream& print0x0008(std::ostream& os, const Value& value);
        //! Print metering mode
        static std::ostream& print0x0009(std::ostream& os, const Value& value);
        //@}

        //! @cond IGNORE
        // Public only so that we can create a static instance
        struct RegisterMn {
            RegisterMn();
        };
        //! @endcond

    private:
        //! Internal virtual create function.
        SigmaMakerNote* create_(bool alloc =true) const;
        //! Internal virtual copy constructor.
        SigmaMakerNote* clone_() const;

        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class SigmaMakerNote

    static SigmaMakerNote::RegisterMn registerSigmaMakerNote;
}                                       // namespace Exiv2

#endif                                  // #ifndef SIGMAMN_HPP_
