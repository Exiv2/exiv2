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
                 long start,
                 ByteOrder byteOrder,
                 long shift);
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
        //! Print the model ID
        static std::ostream& print0x0010(std::ostream& os, const Value& value);
        //! Print the color space
        static std::ostream& print0x00b4(std::ostream& os, const Value& value);
        //! Print the serial number format
        static std::ostream& print0x0015(std::ostream& os, const Value& value);
        //! Print the serial number of the camera
        static std::ostream& print0x000c(std::ostream& os, const Value& value);

        //! Macro mode
        static std::ostream& printCs0x0001(std::ostream& os, const Value& value);
        //! Self timer
        static std::ostream& printCs0x0002(std::ostream& os, const Value& value);
        //! Quality
        static std::ostream& printCs0x0003(std::ostream& os, const Value& value);
        //! Flash mode
        static std::ostream& printCs0x0004(std::ostream& os, const Value& value);
        //! Drive mode
        static std::ostream& printCs0x0005(std::ostream& os, const Value& value);
        //! Focus mode (G1 seems to use field 32 in preference to this)
        static std::ostream& printCs0x0007(std::ostream& os, const Value& value);
        //! Image size
        static std::ostream& printCs0x000a(std::ostream& os, const Value& value);
        //! Easy shooting
        static std::ostream& printCs0x000b(std::ostream& os, const Value& value);
        //! Digital zoom
        static std::ostream& printCs0x000c(std::ostream& os, const Value& value);
        //! ISO
        static std::ostream& printCs0x0010(std::ostream& os, const Value& value);
        //! Metering mode
        static std::ostream& printCs0x0011(std::ostream& os, const Value& value);
        //! Focus range
        static std::ostream& printCs0x0012(std::ostream& os, const Value& value);
        //! AF point selected
        static std::ostream& printCs0x0013(std::ostream& os, const Value& value);
        //! Exposure mode
        static std::ostream& printCs0x0014(std::ostream& os, const Value& value);
        //! LensType mode
        static std::ostream& printCs0x0016(std::ostream& os, const Value& value);
        //! Flash activity
        static std::ostream& printCs0x001c(std::ostream& os, const Value& value);
        //! Flash details
        static std::ostream& printCs0x001d(std::ostream& os, const Value& value);
        //! Focus mode (G1 seems to use this in preference to field 7)
        static std::ostream& printCs0x0020(std::ostream& os, const Value& value);
        //! AE Setting
        static std::ostream& printCs0x0021(std::ostream& os, const Value& value);
        //! Image Stabilization
        static std::ostream& printCs0x0022(std::ostream& os, const Value& value);
        //! Photo Effect
        static std::ostream& printCs0x0026(std::ostream& os, const Value& value);
        //! Focal Type
        static std::ostream& printFl0x0000(std::ostream& os, const Value& value);
        //! Low, normal, high print function
        static std::ostream& printCsLnh(std::ostream& os, const Value& value);
        //! Aperture
        static std::ostream& printCsAper(std::ostream& os, const Value& value);
        //! ISO speed used
        static std::ostream& printSi0x0002(std::ostream& os, const Value& value);
        //! White balance
        static std::ostream& printSi0x0007(std::ostream& os, const Value& value);
        //! Slow shutter
        static std::ostream& printSi0x0008(std::ostream& os, const Value& value);
        //! Sequence number
        static std::ostream& printSi0x0009(std::ostream& os, const Value& value);
        //! AF point used
        static std::ostream& printSi0x000e(std::ostream& os, const Value& value);
        //! Flash bias
        static std::ostream& printSi0x000f(std::ostream& os, const Value& value);
        //! Auto Exposure Bracketing
        static std::ostream& printSi0x0010(std::ostream& os, const Value& value);
        //! Subject distance
        static std::ostream& printSi0x0013(std::ostream& os, const Value& value);
        //! Auto Rorate
        static std::ostream& printSi0x001b(std::ostream& os, const Value& value);
        //! Panorama Frame2
        static std::ostream& printPa0x0001(std::ostream& os, const Value& value);
        //! Panorama Direction
        static std::ostream& printPa0x0005(std::ostream& os, const Value& value);
        //! Shutter speed
        static std::ostream& printSiExp(std::ostream& os, const Value& value);
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
        static const TagInfo tagInfoCs_[];
        static const TagInfo tagInfoFl_[];
        static const TagInfo tagInfoSi_[];
        static const TagInfo tagInfoPa_[];
        static const TagInfo tagInfoCl2_[];
        static const TagInfo tagInfoCi_[];
        static const TagInfo tagInfoCf_[];
        static const TagInfo tagInfoPi_[];
        static const TagInfo tagInfoCf2_[];
        static const TagInfo tagInfoPf_[];
        static const TagInfo tagInfoPv_[];
        static const TagInfo tagInfoFi_[];
        static const TagInfo tagInfoPc_[];
        static const TagInfo tagInfoCb_[];
        static const TagInfo tagInfoFg_[];
        static const TagInfo tagInfoMi_[];
        static const TagInfo tagInfoPr_[];
        static const TagInfo tagInfoSn_[];
        static const TagInfo tagInfoCb2_[];
        static const TagInfo tagInfoCl_[];

    }; // class CanonMakerNote

    static CanonMakerNote::RegisterMn registerCanonMakerNote;

// *****************************************************************************
// template, inline and free functions

    /*!
       @brief Convert Canon hex-based EV (modulo 0x20) to real number
              Ported from Phil Harvey's Image::ExifTool::Canon::CanonEv
              by Will Stokes

       0x00 -> 0
       0x0c -> 0.33333
       0x10 -> 0.5
       0x14 -> 0.66666
       0x20 -> 1
       ..
       160 -> 5
       128 -> 4
       143 -> 4.46875
     */
    float canonEv(long val);

}                                       // namespace Exiv2

#endif                                  // #ifndef CANONMN_HPP_
