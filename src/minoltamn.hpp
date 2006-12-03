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
  @brief   Minolta MakerNote implemented using the following references:<br>
           <a href="http://www.dalibor.cz/minolta/makernote.htm">Minolta Makernote Format Specification</a> by Dalibor Jelinek,<br>
           <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/Minolta.html">Minolta Makernote list</a> by Phil Harvey<br>
           <a href="http://www.ozhiker.com/electronics/pjmt/jpeg_info/minolta_mn.html">Minolta Makernote list from PHP JPEG Metadata Toolkit</a><br>
           Email communication with <a href="mailto:caulier dot gilles at kdemail dot net">caulier dot gilles at kdemail dot net</a><br>
           Some Minolta camera settings have been decoded by <a href="mailto:xraynaud@gmail.com">Xavier Raynaud</a> from digiKam project and added by Gilles Caulier.
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

        //! @name Manipulators
        //@{
        int read(const byte* buf, long len, long start, ByteOrder byteOrder, long shift);
        long copy(byte* buf, ByteOrder byteOrder, long offset);
        void add(const Entry& entry);
        Entries::iterator begin() { return entries_.begin(); }
        Entries::iterator end()   { return entries_.end();   }
        void updateBase(byte* pNewBase);
        //@}

        //! @name Accessors
        //@{
        Entries::const_iterator begin() const { return entries_.begin(); }
        Entries::const_iterator end() const   { return entries_.end();   }
        Entries::const_iterator findIdx(int idx) const;
        long size() const;
        AutoPtr create(bool alloc =true) const;
        AutoPtr clone() const;
        //! Return read-only list of built-in Minolta tags
        static const TagInfo* tagList();
        //! Return read-only list of built-in Minolta Standard Camera Settings tags
        static const TagInfo* tagListCsStd();
        //! Return read-only list of built-in Minolta 7D Camera Settings tags
        static const TagInfo* tagListCs7D();
        //! Return read-only list of built-in Minolta 5D Camera Settings tags
        static const TagInfo* tagListCs5D();
        //@}

        //! @name Print functions for Minolta %MakerNote tags
        //@{
        //! Print Exposure Speed setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaExposureSpeedStd(std::ostream& os, const Value& value);
        //! Print Exposure Time setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaExposureTimeStd(std::ostream& os, const Value& value);
        //! Print F Number setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaFNumberStd(std::ostream& os, const Value& value);
        //! Print Exposure Compensation setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaExposureCompensationStd(std::ostream& os, const Value& value);
        //! Print Focal Length setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaFocalLengthStd(std::ostream& os, const Value& value);
        //! Print Minolta Date from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaDateStd(std::ostream& os, const Value& value);
        //! Print Minolta Time from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaTimeStd(std::ostream& os, const Value& value);
        //! Print Flash Exposure Compensation setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaFlashExposureCompStd(std::ostream& os, const Value& value);
        //! Print White Balance setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaWhiteBalanceStd(std::ostream& os, const Value& value);
        //! Print Brightness setting from standard Minolta Camera Settings makernote
        static std::ostream& printMinoltaBrightnessStd(std::ostream& os, const Value& value);

        //! Print Exposure Manual Bias setting from 5D Minolta Camera Settings makernote
        static std::ostream& printMinoltaExposureManualBias5D(std::ostream& os, const Value& value);
        //! Print Exposure Compensation setting from 5D Minolta Camera Settings makernote
        static std::ostream& printMinoltaExposureCompensation5D(std::ostream& os, const Value& value);
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
        //! Add a Dynax 5D and 7D camera settings entry to the makernote entries
        void addCsEntry(IfdId ifdId, uint16_t tag, long offset, const byte* data, int count);
        //! Add a standard camera settings entry to the makernote entries
        void addCsStdEntry(IfdId ifdId, uint32_t tag, long offset, const byte* data, int count);
        //@}

        //! @name Accessors
        //@{
        //! Assemble special Dynax 5D or 7D Minolta entries into an entry with the original tag
        long assemble(Entry& e, IfdId ifdId, uint16_t tag, ByteOrder byteOrder) const;
        //! Assemble special standard Minolta entries into an entry with the original tag
        long assembleStd(Entry& e, IfdId ifdId, uint32_t tag, ByteOrder byteOrder) const;
        //! Internal virtual create function.
        MinoltaMakerNote* create_(bool alloc =true) const;
        //! Internal virtual copy constructor.
        MinoltaMakerNote* clone_() const;
        //@}

        // DATA
        //! Container to store Makernote entries (instead of Ifd)
        Entries entries_;

        //! Tag information
        static const TagInfo tagInfo_[];
        static const TagInfo tagInfoCs5D_[];
        static const TagInfo tagInfoCs7D_[];
        static const TagInfo tagInfoCsStd_[];

    }; // class MinoltaMakerNote

    static MinoltaMakerNote::RegisterMn registerMinoltaMakerNote;
}                                       // namespace Exiv2

#endif                                  // #ifndef MINOLTAMN_HPP_
