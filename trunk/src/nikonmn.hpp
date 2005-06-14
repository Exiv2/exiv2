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
  @file    nikonmn.hpp
  @brief   Nikon MakerNote formats.

  The Nikon MakerNote formats are implemented according to the following references<BR>
  Format 1: 
  <ul>
  <li><a href="http://www.tawbaware.com/990exif.htm">MakerNote EXIF Tag of the Nikon 990</a> by Max Lyons</li></ul>
  Format 2: 
  <ul><li>"Appendix 2: Makernote of Nikon" of the document 
  <a href="http://park2.wakwak.com/%7Etsuruzoh/Computer/Digicams/exif-e.html">
  Exif file format</a> by TsuruZoh Tachibanaya</li></ul>
  Format 3: 
  <ul><li>"EXIFutils Field Reference Guide"</li>
  <li><a href="http://www.ozhiker.com/electronics/pjmt/jpeg_info/nikon_mn.html#Nikon_Type_3_Tags">Nikon Type 3 Makernote Tags Definition</a> 
  of the PHP JPEG Metadata Toolkit by Evan Hunter</li>
  <li>Nikon tag information from <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/">ExifTool</a> by Phil Harvey</li>
  <li>Email communication with Robert Rottmerhusen</li>
  </ul>

  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    17-May-04, ahu: created<BR>
           25-May-04, ahu: combined all Nikon formats in one component
 */
#ifndef NIKONMN_HPP_
#define NIKONMN_HPP_

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
    MakerNote::AutoPtr createNikonMakerNote(bool alloc,
                                            const byte* buf, 
                                            long len, 
                                            ByteOrder byteOrder, 
                                            long offset);

// *****************************************************************************
// class definitions

    //! A MakerNote format used by Nikon cameras, such as the E990 and D1.
    class Nikon1MakerNote : public IfdMakerNote {
    public:
        //! Shortcut for a %Nikon1MakerNote auto pointer.
        typedef std::auto_ptr<Nikon1MakerNote> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        Nikon1MakerNote(bool alloc =true);
        //! Copy constructor
        Nikon1MakerNote(const Nikon1MakerNote& rhs);
        //! Virtual destructor
        virtual ~Nikon1MakerNote() {}
        //@}

        //! @name Accessors
        //@{
        AutoPtr create(bool alloc =true) const;
        AutoPtr clone() const;
        //@}

        //! @name Print functions for Nikon1 %MakerNote tags 
        //@{
        //! Print ISO setting
        static std::ostream& print0x0002(std::ostream& os, const Value& value);
        //! Print autofocus mode
        static std::ostream& print0x0007(std::ostream& os, const Value& value);
        //! Print manual focus distance
        static std::ostream& print0x0085(std::ostream& os, const Value& value);
        //! Print digital zoom setting
        static std::ostream& print0x0086(std::ostream& os, const Value& value);
        //! Print AF focus position
        static std::ostream& print0x0088(std::ostream& os, const Value& value);
        //@}

        //! @cond IGNORE
        // Public only so that we can create a static instance
        struct RegisterMn {
            RegisterMn();
        };
        //! @endcond

    private:
        //! Internal virtual create function.
        Nikon1MakerNote* create_(bool alloc =true) const;
        //! Internal virtual copy constructor.
        Nikon1MakerNote* clone_() const;

        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class Nikon1MakerNote

    static Nikon1MakerNote::RegisterMn registerNikon1MakerNote;

    /*!
      @brief A second MakerNote format used by Nikon cameras, including the 
             E700, E800, E900, E900S, E910, E950
     */ 
    class Nikon2MakerNote : public IfdMakerNote {
    public:
        //! Shortcut for a %Nikon2MakerNote auto pointer.
        typedef std::auto_ptr<Nikon2MakerNote> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        Nikon2MakerNote(bool alloc =true);
        //! Copy constructor
        Nikon2MakerNote(const Nikon2MakerNote& rhs);
        //! Virtual destructor
        virtual ~Nikon2MakerNote() {}
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

        //! @name Print functions for Nikon2 %MakerNote tags 
        //@{
        //! Print quality setting
        static std::ostream& print0x0003(std::ostream& os, const Value& value);
        //! Print color mode setting
        static std::ostream& print0x0004(std::ostream& os, const Value& value);
        //! Print image adjustment setting
        static std::ostream& print0x0005(std::ostream& os, const Value& value);
        //! Print ISO speed setting
        static std::ostream& print0x0006(std::ostream& os, const Value& value);
        //! Print white balance setting
        static std::ostream& print0x0007(std::ostream& os, const Value& value);
        //! Print digital zoom setting
        static std::ostream& print0x000a(std::ostream& os, const Value& value);
        //@}

        //! @cond IGNORE
        // Public only so that we can create a static instance
        struct RegisterMn {
            RegisterMn();
        };
        //! @endcond

    private:
        //! Internal virtual create function.
        Nikon2MakerNote* create_(bool alloc =true) const;
        //! Internal virtual copy constructor.
        Nikon2MakerNote* clone_() const;

        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class Nikon2MakerNote

    static Nikon2MakerNote::RegisterMn registerNikon2MakerNote;

    //! A third MakerNote format used by Nikon cameras, e.g., E5400, SQ, D2H, D70
    class Nikon3MakerNote : public IfdMakerNote {
    public:
        //! Shortcut for a %Nikon3MakerNote auto pointer.
        typedef std::auto_ptr<Nikon3MakerNote> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        Nikon3MakerNote(bool alloc =true);
        //! Copy constructor
        Nikon3MakerNote(const Nikon3MakerNote& rhs);
        //! Virtual destructor
        virtual ~Nikon3MakerNote() {}
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

        //! @name Print functions for Nikon3 %MakerNote tags 
        //@{
        //! Print ISO setting
        static std::ostream& print0x0002(std::ostream& os, const Value& value);
        //! Print flash compensation
        static std::ostream& print0x0012(std::ostream& os, const Value& value);
        //! Print lens information
        static std::ostream& print0x0084(std::ostream& os, const Value& value);
        //! Print flash used information
        static std::ostream& print0x0087(std::ostream& os, const Value& value);
        //! Print AF point
        static std::ostream& print0x0088(std::ostream& os, const Value& value);
        //! Print bracketing information
        static std::ostream& print0x0089(std::ostream& os, const Value& value);
        //! Print number of lens stops
        static std::ostream& print0x008b(std::ostream& os, const Value& value);
        //! Print number of lens data
        static std::ostream& print0x0098(std::ostream& os, const Value& value);
        //@}

        //! @cond IGNORE
        // Public only so that we can create a static instance
        struct RegisterMn {
            RegisterMn();
        };
        //! @endcond

    private:
        //! Internal virtual create function.
        Nikon3MakerNote* create_(bool alloc =true) const;
        //! Internal virtual copy constructor.
        Nikon3MakerNote* clone_() const;

        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class Nikon3MakerNote

    static Nikon3MakerNote::RegisterMn registerNikon3MakerNote;

}                                       // namespace Exiv2

#endif                                  // #ifndef NIKONMN_HPP_
