// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2007 Michal Cihar <michal@cihar.com>
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
  @file    pentaxmn.hpp
  @brief   Pentax MakerNote implemented according to the specification
  		   http://www.gvsoft.homedns.org/exif/makernote-pentax-type3.html and 
		   based on ExifTool implementation and 
           <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/Pentax.html">Pentax Makernote list</a> by Phil Harvey<br>
  @version $Rev$
  @author  Michal Cihar
           <a href="mailto:michal@cihar.com">michal@cihar.com</a>
  @date    27-Sep-07
 */
#ifndef PENTAXMN_HPP_
#define PENTAXMN_HPP_

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
    MakerNote::AutoPtr createPentaxMakerNote(bool alloc,
                                           const byte* buf,
                                           long len,
                                           ByteOrder byteOrder,
                                           long offset);

// *****************************************************************************
// class definitions

    //! MakerNote for Pentaxfilm cameras
    class PentaxMakerNote : public IfdMakerNote {
    public:
        //! Shortcut for a %PentaxMakerNote auto pointer.
        typedef std::auto_ptr<PentaxMakerNote> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        PentaxMakerNote(bool alloc =true);
        //! Copy constructor
        PentaxMakerNote(const PentaxMakerNote& rhs);
        //! Virtual destructor
        virtual ~PentaxMakerNote() {}
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
        //! Return read-only list of built-in Pentaxfilm tags
        static const TagInfo* tagList();
        //@}

        //! @cond IGNORE
        // Public only so that we can create a static instance
        struct RegisterMn {
            RegisterMn();
        };
        //! @endcond

		static std::ostream& printPentaxVersion(std::ostream& os, const Value& value);
		static std::ostream& printPentaxResolution(std::ostream& os, const Value& value);
    	static std::ostream& printPentaxDate(std::ostream& os, const Value& value);
    	static std::ostream& printPentaxTime(std::ostream& os, const Value& value);
    	static std::ostream& printPentaxExposure(std::ostream& os, const Value& value);
    	static std::ostream& printPentaxFValue(std::ostream& os, const Value& value);
    	static std::ostream& printPentaxFocalLength(std::ostream& os, const Value& value);
    	static std::ostream& printPentaxCompensation(std::ostream& os, const Value& value);
    	static std::ostream& printPentaxTemperature(std::ostream& os, const Value& value);
    	static std::ostream& printPentaxFlashCompensation(std::ostream& os, const Value& value);
    	static std::ostream& printPentaxBracketing(std::ostream& os, const Value& value);
    	static std::ostream& printPentaxImageProcessing(std::ostream& os, const Value& value);
    	static std::ostream& printPentaxPictureMode(std::ostream& os, const Value& value);
    	static std::ostream& printPentaxDriveMode(std::ostream& os, const Value& value);
    	static std::ostream& printPentaxLensType(std::ostream& os, const Value& value);

    private:
        //! Internal virtual create function.
        PentaxMakerNote* create_(bool alloc =true) const;
        //! Internal virtual copy constructor.
        PentaxMakerNote* clone_() const;

        //! Tag information
        static const TagInfo tagInfo_[];
    }; // class PentaxMakerNote

    static PentaxMakerNote::RegisterMn registerPentaxMakerNote;
}                                       // namespace Exiv2

#endif                                  // #ifndef PENTAXMN_HPP_
