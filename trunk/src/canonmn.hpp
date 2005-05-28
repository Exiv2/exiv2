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
  @file    canonmn.hpp
  @brief   Canon MakerNote implemented according to the specification
           <a href="http://www.burren.cx/david/canon.html">
           EXIF MakerNote of Canon</a> by David Burren<br>
           and with reference to tag information from
           <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/">
           ExifTool</a> by Phil Harvey
  @version $Rev$
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
    MakerNote::AutoPtr createCanonMakerNote(bool alloc,
                                            const byte* buf, 
                                            long len, 
                                            ByteOrder byteOrder, 
                                            long offset);

// *****************************************************************************
// class definitions

    //! MakerNote for Canon cameras
    class CanonMakerNote : public IfdMakerNote {
    public:
        //! Shortcut for a %CanonMakerNote auto pointer.
        typedef std::auto_ptr<CanonMakerNote> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management
                 is required for the makernote entries.
         */
        CanonMakerNote(bool alloc =true);
        //! Copy constructor
        CanonMakerNote(const CanonMakerNote& rhs);
        //! Virtual destructor
        virtual ~CanonMakerNote() {}
        //@}

        //! @name Manipulators
        //@{
        int read(const byte* buf,
                 long len, 
                 ByteOrder byteOrder, 
                 long offset);
        long copy(byte* buf, ByteOrder byteOrder, long offset);
        void add(const Entry& entry);
        Entries::iterator begin() { return entries_.begin(); }
        Entries::iterator end() { return entries_.end(); }
        void updateBase(byte* pNewBase);
        //@}

        //! @name Accessors
        //@{
        Entries::const_iterator begin() const { return entries_.begin(); }
        Entries::const_iterator end() const { return entries_.end(); }
        Entries::const_iterator findIdx(int idx) const;
        long size() const;
        AutoPtr create(bool alloc =true) const;
        AutoPtr clone() const;
        //@}

        //! @name Print functions for Canon %MakerNote tags 
        //@{
        //! Print the image number
        static std::ostream& print0x0008(std::ostream& os, const Value& value);
        //! Print the serial number of the camera
        static std::ostream& print0x000c(std::ostream& os, const Value& value);

        //! Macro mode
        static std::ostream& printCs10x0001(std::ostream& os, const Value& value);
        //! Self timer
        static std::ostream& printCs10x0002(std::ostream& os, const Value& value);
        //! Quality
        static std::ostream& printCs10x0003(std::ostream& os, const Value& value);
        //! Flash mode
        static std::ostream& printCs10x0004(std::ostream& os, const Value& value);
        //! Drive mode
        static std::ostream& printCs10x0005(std::ostream& os, const Value& value);
        //! Focus mode (G1 seems to use field 32 in preference to this)
        static std::ostream& printCs10x0007(std::ostream& os, const Value& value);
        //! Image size
        static std::ostream& printCs10x000a(std::ostream& os, const Value& value);
        //! Easy shooting
        static std::ostream& printCs10x000b(std::ostream& os, const Value& value);
        //! Digital zoom
        static std::ostream& printCs10x000c(std::ostream& os, const Value& value);
        //! ISO
        static std::ostream& printCs10x0010(std::ostream& os, const Value& value);
        //! Metering mode
        static std::ostream& printCs10x0011(std::ostream& os, const Value& value);
        //! Focus type
        static std::ostream& printCs10x0012(std::ostream& os, const Value& value);
        //! AF point selected
        static std::ostream& printCs10x0013(std::ostream& os, const Value& value);
        //! Exposure mode
        static std::ostream& printCs10x0014(std::ostream& os, const Value& value);
        //! Flash activity
        static std::ostream& printCs10x001c(std::ostream& os, const Value& value);
        //! Flash details 
        static std::ostream& printCs10x001d(std::ostream& os, const Value& value);
        //! Focus mode (G1 seems to use this in preference to field 7)
        static std::ostream& printCs10x0020(std::ostream& os, const Value& value);
        //! Low, normal, high print function
        static std::ostream& printCs1Lnh(std::ostream& os, const Value& value);
        //! Camera lens information
        static std::ostream& printCs1Lens(std::ostream& os, const Value& value);
        //! ISO speed used
        static std::ostream& printCs20x0002(std::ostream& os, const Value& value);
        //! White balance
        static std::ostream& printCs20x0007(std::ostream& os, const Value& value);
        //! Sequence number
        static std::ostream& printCs20x0009(std::ostream& os, const Value& value);
        //! AF point used
        static std::ostream& printCs20x000e(std::ostream& os, const Value& value);
        //! Flash bias
        static std::ostream& printCs20x000f(std::ostream& os, const Value& value);
        //! Subject distance
        static std::ostream& printCs20x0013(std::ostream& os, const Value& value);
        //@}

        //! @cond IGNORE
        // Public only so that we can create a static instance
        struct RegisterMn {
            RegisterMn();
        };
        //! @endcond

    private:
        //! @name Manipulators
        //@{
        //! Add a camera settings entry to the makernote entries
        void addCsEntry(IfdId ifdId, 
                        uint16_t tag, 
                        long offset,
                        const byte* data,
                        int count);
        //@}

        //! @name Accessors
        //@{
        //! Assemble special Canon entries into an entry with the original tag
        long assemble(Entry& e, 
                      IfdId ifdId, 
                      uint16_t tag,
                      ByteOrder byteOrder) const;
        //! Internal virtual create function.
        CanonMakerNote* create_(bool alloc =true) const;
        //! Internal virtual copy constructor.
        CanonMakerNote* clone_() const;
        //@}

        // DATA
        //! Container to store Makernote entries (instead of Ifd)
        Entries entries_;

        //! Tag information
        static const TagInfo tagInfo_[];
        static const TagInfo tagInfoCs1_[];
        static const TagInfo tagInfoCs2_[];
        static const TagInfo tagInfoCf_[];

    }; // class CanonMakerNote

    static CanonMakerNote::RegisterMn registerCanonMakerNote;
}                                       // namespace Exiv2

#endif                                  // #ifndef CANONMN_HPP_
