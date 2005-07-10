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
/*
  File:      canonmn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Feb-04, ahu: created
             07-Mar-04, ahu: isolated as a separate component
  Credits:   Canon MakerNote implemented according to the specification
             "EXIF MakerNote of Canon" <http://www.burren.cx/david/canon.html>
             by David Burren
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#include "types.hpp"
#include "canonmn.hpp"
#include "makernote.hpp"
#include "value.hpp"
#include "ifd.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cmath>

// *****************************************************************************
// local declarations
namespace {
    /* 
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
}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! @cond IGNORE
    CanonMakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("Canon", "*", createCanonMakerNote); 
        
        MakerNoteFactory::registerMakerNote(
            canonIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(
            canonCs1IfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(
            canonCs2IfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(
            canonCfIfdId, MakerNote::AutoPtr(new CanonMakerNote));

        ExifTags::registerMakerTagInfo(canonIfdId, tagInfo_);
        ExifTags::registerMakerTagInfo(canonCs1IfdId, tagInfoCs1_);
        ExifTags::registerMakerTagInfo(canonCs2IfdId, tagInfoCs2_);
        ExifTags::registerMakerTagInfo(canonCfIfdId, tagInfoCf_);
    }
    //! @endcond

    // Canon MakerNote Tag Info
    const TagInfo CanonMakerNote::tagInfo_[] = {
        TagInfo(0x0000, "0x0000", "0x0000", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0001, "CameraSettings1", "CameraSettings1", "Various camera settings (1)", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0002, "0x0002", "0x0002", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0003, "0x0003", "0x0003", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0004, "CameraSettings2", "CameraSettings2", "Various camera settings (2)", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0006, "ImageType", "ImageType", "Image type", canonIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "FirmwareVersion", "FirmwareVersion", "Firmware version", canonIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0008, "ImageNumber", "ImageNumber", "Image number", canonIfdId, makerTags, unsignedLong, print0x0008),
        TagInfo(0x0009, "OwnerName", "OwnerName", "Owner Name", canonIfdId, makerTags, asciiString, printValue),
        TagInfo(0x000c, "SerialNumber", "SerialNumber", "Camera serial number", canonIfdId, makerTags, unsignedLong, print0x000c),
        TagInfo(0x000d, "0x000d", "0x000d", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000f, "CustomFunctions", "CustomFunctions", "Custom Functions", canonIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonMakerNoteTag)", "(UnknownCanonMakerNoteTag)", "Unknown CanonMakerNote tag", canonIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Camera Settings 1 Tag Info
    const TagInfo CanonMakerNote::tagInfoCs1_[] = {
        TagInfo(0x0001, "Macro", "Macro", "Macro mode", canonCs1IfdId, makerTags, unsignedShort, printCs10x0001),
        TagInfo(0x0002, "Selftimer", "Selftimer", "Self timer", canonCs1IfdId, makerTags, unsignedShort, printCs10x0002),
        TagInfo(0x0003, "Quality", "Quality", "Quality", canonCs1IfdId, makerTags, unsignedShort, printCs10x0003),
        TagInfo(0x0004, "FlashMode", "FlashMode", "Flash mode setting", canonCs1IfdId, makerTags, unsignedShort, printCs10x0004),
        TagInfo(0x0005, "DriveMode", "DriveMode", "Drive mode setting", canonCs1IfdId, makerTags, unsignedShort, printCs10x0005),
        TagInfo(0x0006, "0x0006", "0x0006", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "FocusMode", "FocusMode", "Focus mode setting", canonCs1IfdId, makerTags, unsignedShort, printCs10x0007),
        TagInfo(0x0008, "0x0008", "0x0008", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0009, "0x0009", "0x0009", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000a, "ImageSize", "ImageSize", "Image size", canonCs1IfdId, makerTags, unsignedShort, printCs10x000a),
        TagInfo(0x000b, "EasyMode", "EasyMode", "Easy shooting mode", canonCs1IfdId, makerTags, unsignedShort, printCs10x000b),
        TagInfo(0x000c, "DigitalZoom", "DigitalZoom", "Digital zoom", canonCs1IfdId, makerTags, unsignedShort, printCs10x000c),
        TagInfo(0x000d, "Contrast", "Contrast", "Contrast setting", canonCs1IfdId, makerTags, unsignedShort, printCs1Lnh),
        TagInfo(0x000e, "Saturation", "Saturation", "Saturation setting", canonCs1IfdId, makerTags, unsignedShort, printCs1Lnh),
        TagInfo(0x000f, "Sharpness", "Sharpness", "Sharpness setting", canonCs1IfdId, makerTags, unsignedShort, printCs1Lnh),
        TagInfo(0x0010, "ISOSpeed", "ISOSpeed", "ISO speed setting", canonCs1IfdId, makerTags, unsignedShort, printCs10x0010),
        TagInfo(0x0011, "MeteringMode", "MeteringMode", "Metering mode setting", canonCs1IfdId, makerTags, unsignedShort, printCs10x0011),
        TagInfo(0x0012, "FocusType", "FocusType", "Focus type setting", canonCs1IfdId, makerTags, unsignedShort, printCs10x0012),
        TagInfo(0x0013, "AFPoint", "AFPoint", "AF point selected", canonCs1IfdId, makerTags, unsignedShort, printCs10x0013),
        TagInfo(0x0014, "ExposureProgram", "ExposureProgram", "Exposure mode setting", canonCs1IfdId, makerTags, unsignedShort, printCs10x0014),
        TagInfo(0x0015, "0x0015", "0x0015", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0016, "0x0016", "0x0016", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0017, "Lens", "Lens", "'long' and 'short' focal length of lens (in 'focal units') and 'focal units' per mm", canonCs1IfdId, makerTags, unsignedShort, printCs1Lens),
        TagInfo(0x0018, "0x0018", "0x0018", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0019, "0x0019", "0x0019", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001a, "0x001a", "0x001a", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001b, "0x001b", "0x001b", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001c, "FlashActivity", "FlashActivity", "Flash activity", canonCs1IfdId, makerTags, unsignedShort, printCs10x001c),
        TagInfo(0x001d, "FlashDetails", "FlashDetails", "Flash details", canonCs1IfdId, makerTags, unsignedShort, printCs10x001d),
        TagInfo(0x001e, "0x001e", "0x001e", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001f, "0x001f", "0x001f", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0020, "FocusMode", "FocusMode", "Focus mode setting", canonCs1IfdId, makerTags, unsignedShort, printCs10x0020),
        TagInfo(0x0021, "0x0021", "0x0021", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0022, "0x0022", "0x0022", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0023, "0x0023", "0x0023", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0024, "0x0024", "0x0024", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0025, "0x0025", "0x0025", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0026, "0x0026", "0x0026", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0027, "0x0027", "0x0027", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCs1Tag)", "(UnknownCanonCs1Tag)", "Unknown Canon Camera Settings 1 tag", canonCs1IfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Camera Settings 2 Tag Info
    const TagInfo CanonMakerNote::tagInfoCs2_[] = {
        TagInfo(0x0001, "0x0001", "0x0001", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0002, "ISOSpeed", "ISOSpeed", "ISO speed used", canonCs2IfdId, makerTags, unsignedShort, printCs20x0002),
        TagInfo(0x0003, "0x0003", "0x0003", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0004, "0x0004", "0x0004", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0005, "0x0005", "0x0005", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0006, "0x0006", "0x0006", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "WhiteBalance", "WhiteBalance", "White balance setting", canonCs2IfdId, makerTags, unsignedShort, printCs20x0007),
        TagInfo(0x0008, "0x0008", "0x0008", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0009, "Sequence", "Sequence", "Sequence number (if in a continuous burst)", canonCs2IfdId, makerTags, unsignedShort, printCs20x0009),
        TagInfo(0x000a, "0x000a", "0x000a", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000b, "0x000b", "0x000b", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000c, "0x000c", "0x000c", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000d, "0x000d", "0x000d", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000e, "AFPointUsed", "AFPointUsed", "AF point used", canonCs2IfdId, makerTags, unsignedShort, printCs20x000e),
        TagInfo(0x000f, "FlashBias", "FlashBias", "Flash bias", canonCs2IfdId, makerTags, unsignedShort, printCs20x000f),
        TagInfo(0x0010, "0x0010", "0x0010", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0011, "0x0011", "0x0011", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0012, "0x0012", "0x0012", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0013, "SubjectDistance", "SubjectDistance", "Subject distance (units are not clear)", canonCs2IfdId, makerTags, unsignedShort, printCs20x0013),
        TagInfo(0x0014, "0x0014", "0x0014", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0015, "0x0015", "0x0015", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0016, "0x0016", "0x0016", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0017, "0x0017", "0x0017", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0018, "0x0018", "0x0018", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0019, "0x0019", "0x0019", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001a, "0x001a", "0x001a", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCs2Tag)", "(UnknownCanonCs2Tag)", "Unknown Canon Camera Settings 2 tag", canonCs2IfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Custom Function Tag Info
    const TagInfo CanonMakerNote::tagInfoCf_[] = {
        TagInfo(0x0001, "NoiseReduction", "NoiseReduction", "Long exposure noise reduction", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0002, "ShutterAeLock", "ShutterAeLock", "Shutter/AE lock buttons", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0003, "MirrorLockup", "MirrorLockup", "Mirror lockup", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0004, "ExposureLevelIncrements", "ExposureLevelIncrements", "Tv/Av and exposure level", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0005, "AFAssist", "AFAssist", "AF assist light", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0006, "FlashSyncSpeedAv", "FlashSyncSpeedAv", "Shutter speed in Av mode", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "AEBSequence", "AEBSequence", "AEB sequence/auto cancellation", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0008, "ShutterCurtainSync", "ShutterCurtainSync", "Shutter curtain sync", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0009, "LensAFStopButton", "LensAFStopButton", "Lens AF stop button Fn. Switch", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000a, "FillFlashAutoReduction", "FillFlashAutoReduction", "Auto reduction of fill flash", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000b, "MenuButtonReturn", "MenuButtonReturn", "Menu button return position", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000c, "SetButtonFunction", "SetButtonFunction", "SET button func. when shooting", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000d, "SensorCleaning", "SensorCleaning", "Sensor cleaning", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000e, "SuperimposedDisplay", "SuperimposedDisplay", "Superimposed display", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000f, "ShutterReleaseNoCFCard", "ShutterReleaseNoCFCard", "Shutter Release W/O CF Card", canonCfIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCfTag)", "(UnknownCanonCfTag)", "Unknown Canon Custom Function tag", canonCfIfdId, makerTags, invalidTypeId, printValue)
    };

    int CanonMakerNote::read(const byte* buf,
                             long len, 
                             ByteOrder byteOrder, 
                             long offset)
    {
        int rc = IfdMakerNote::read(buf, len, byteOrder, offset);
        if (rc) return rc;

        // Decode camera settings 1 and add settings as additional entries
        Entries::iterator cs = ifd_.findTag(0x0001);
        if (cs != ifd_.end() && cs->type() == unsignedShort) {
            for (uint16_t c = 1; cs->count() > c; ++c) {
                if (c == 23 && cs->count() > 25) {
                    // Pack related lens info into one tag
                    addCsEntry(canonCs1IfdId, c, cs->offset() + c*2, 
                               cs->data() + c*2, 3);
                    c += 2;
                }
                else {
                    addCsEntry(canonCs1IfdId, c, cs->offset() + c*2,
                               cs->data() + c*2, 1);
                }
            }
            // Discard the original entry
            ifd_.erase(cs);
        }

        // Decode camera settings 2 and add settings as additional entries
        cs = ifd_.findTag(0x0004);
        if (cs != ifd_.end() && cs->type() == unsignedShort) {
            for (uint16_t c = 1; cs->count() > c; ++c) {
                addCsEntry(canonCs2IfdId, c, cs->offset() + c*2,
                           cs->data() + c*2, 1);
            }
            // Discard the original entry
            ifd_.erase(cs);
        }

        // Decode custom functions and add each as an additional entry
        cs = ifd_.findTag(0x000f);
        if (cs != ifd_.end() && cs->type() == unsignedShort) {
            for (uint16_t c = 1; cs->count() > c; ++c) {
                addCsEntry(canonCfIfdId, c, cs->offset() + c*2,
                           cs->data() + c*2, 1);
            }
            // Discard the original entry
            ifd_.erase(cs);
        }

        // Copy remaining ifd entries
        entries_.insert(entries_.begin(), ifd_.begin(), ifd_.end());

        // Set idx
        int idx = 0;
        Entries::iterator e = entries_.end();
        for (Entries::iterator i = entries_.begin(); i != e; ++i) {
            i->setIdx(++idx);
        }

        return 0;
    }

    void CanonMakerNote::addCsEntry(IfdId ifdId, 
                                    uint16_t tag, 
                                    long offset, 
                                    const byte* data,
                                    int count)
    {
        Entry e(false);
        e.setIfdId(ifdId);
        e.setTag(tag);
        e.setOffset(offset);
        e.setValue(unsignedShort, count, data, 2*count);
        add(e);
    }

    void CanonMakerNote::add(const Entry& entry)
    {
        assert(alloc_ == entry.alloc());
        assert(   entry.ifdId() == canonIfdId 
               || entry.ifdId() == canonCs1IfdId
               || entry.ifdId() == canonCs2IfdId
               || entry.ifdId() == canonCfIfdId);
        // allow duplicates
        entries_.push_back(entry);
    }

    long CanonMakerNote::copy(byte* buf, ByteOrder byteOrder, long offset)
    {
        if (byteOrder_ == invalidByteOrder) byteOrder_ = byteOrder;

        assert(ifd_.alloc()); 
        ifd_.clear();

        // Add all standard Canon entries to the IFD
        Entries::const_iterator end = entries_.end();
        for (Entries::const_iterator i = entries_.begin(); i != end; ++i) {
            if (i->ifdId() == canonIfdId) {
                ifd_.add(*i);
            }
        }
        // Collect camera settings 1 entries and add the original Canon tag
        Entry cs1;
        if (assemble(cs1, canonCs1IfdId, 0x0001, byteOrder_)) {
            ifd_.erase(0x0001);
            ifd_.add(cs1);
        }
        // Collect camera settings 2 entries and add the original Canon tag
        Entry cs2;
        if (assemble(cs2, canonCs2IfdId, 0x0004, byteOrder_)) {
            ifd_.erase(0x0004);
            ifd_.add(cs2);
        }
        // Collect custom function entries and add the original Canon tag
        Entry cf;
        if (assemble(cf, canonCfIfdId, 0x000f, byteOrder_)) {
            ifd_.erase(0x000f);
            ifd_.add(cf);
        }

        return IfdMakerNote::copy(buf, byteOrder_, offset);
    } // CanonMakerNote::copy

    void CanonMakerNote::updateBase(byte* pNewBase)
    {
        byte* pBase = ifd_.updateBase(pNewBase);
        if (absOffset_ && !alloc_) {
            Entries::iterator end = entries_.end();
            for (Entries::iterator pos = entries_.begin(); pos != end; ++pos) {
                pos->updateBase(pBase, pNewBase);
            }
        }
    } // CanonMakerNote::updateBase

    long CanonMakerNote::size() const
    {
        Ifd ifd(canonIfdId, 0, alloc_); // offset doesn't matter

        // Add all standard Canon entries to the IFD
        Entries::const_iterator end = entries_.end();
        for (Entries::const_iterator i = entries_.begin(); i != end; ++i) {
            if (i->ifdId() == canonIfdId) {
                ifd.add(*i);
            }
        }
        // Collect camera settings 1 entries and add the original Canon tag
        Entry cs1(alloc_);
        if (assemble(cs1, canonCs1IfdId, 0x0001, littleEndian)) {
            ifd.erase(0x0001);
            ifd.add(cs1);
        }
        // Collect camera settings 2 entries and add the original Canon tag
        Entry cs2(alloc_);
        if (assemble(cs2, canonCs2IfdId, 0x0004, littleEndian)) {
            ifd.erase(0x0004);
            ifd.add(cs2);
        }
        // Collect custom function entries and add the original Canon tag
        Entry cf(alloc_);
        if (assemble(cf, canonCfIfdId, 0x000f, littleEndian)) {
            ifd.erase(0x000f);
            ifd.add(cf);
        }

        return headerSize() + ifd.size() + ifd.dataSize();
    } // CanonMakerNote::size

    long CanonMakerNote::assemble(Entry& e, 
                                  IfdId ifdId,
                                  uint16_t tag,
                                  ByteOrder byteOrder) const
    {
        DataBuf buf(1024);
        memset(buf.pData_, 0x0, 1024);
        uint16_t len = 0;
        Entries::const_iterator end = entries_.end();
        for (Entries::const_iterator i = entries_.begin(); i != end; ++i) {
            if (i->ifdId() == ifdId) {
                uint16_t pos = i->tag() * 2;
                uint16_t size = pos + static_cast<uint16_t>(i->size());
                assert(size <= 1024);
                memcpy(buf.pData_ + pos, i->data(), i->size());
                if (len < size) len = size;
            }
        }
        if (len > 0) {
            // Number of shorts in the buffer (rounded up)
            uint16_t s = (len+1) / 2;
            us2Data(buf.pData_, s*2, byteOrder);

            e.setIfdId(canonIfdId);
            e.setIdx(0); // don't care
            e.setTag(tag);
            e.setOffset(0); // will be calculated when the IFD is written
            e.setValue(unsignedShort, s, buf.pData_, s*2);
        }
        return len;
    } // CanonMakerNote::assemble

    Entries::const_iterator CanonMakerNote::findIdx(int idx) const 
    {
        return std::find_if(entries_.begin(), entries_.end(),
                            FindEntryByIdx(idx));
    }

    CanonMakerNote::CanonMakerNote(bool alloc)
        : IfdMakerNote(canonIfdId, alloc)
    {
    }

    CanonMakerNote::CanonMakerNote(const CanonMakerNote& rhs)
        : IfdMakerNote(rhs)
    {
        entries_ = rhs.entries_;
    }

    CanonMakerNote::AutoPtr CanonMakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    CanonMakerNote* CanonMakerNote::create_(bool alloc) const 
    {
        return new CanonMakerNote(alloc); 
    }

    CanonMakerNote::AutoPtr CanonMakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    CanonMakerNote* CanonMakerNote::clone_() const 
    {
        return new CanonMakerNote(*this); 
    }

    std::ostream& CanonMakerNote::print0x0008(std::ostream& os,
                                              const Value& value)
    {
        std::string n = value.toString();
        return os << n.substr(0, n.length() - 4) << "-" 
                  << n.substr(n.length() - 4);
    }

    std::ostream& CanonMakerNote::print0x000c(std::ostream& os,
                                              const Value& value)
    {
        std::istringstream is(value.toString());
        uint32_t l;
        is >> l;
        return os << std::setw(4) << std::setfill('0') << std::hex 
                  << ((l & 0xffff0000) >> 16)
                  << std::setw(5) << std::setfill('0') << std::dec
                  << (l & 0x0000ffff);
    }

    std::ostream& CanonMakerNote::printCs10x0001(std::ostream& os, 
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 1: os << "On"; break;
        case 2: os << "Off"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x0002(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        if (l == 0) {
            os << "Off";
        }
        else { 
            os << l / 10.0 << " s";
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x0003(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 2: os << "Normal"; break;
        case 3: os << "Fine"; break;
        case 5: os << "Superfine"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x0004(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Off"; break;
        case 1: os << "Auto"; break;
        case 2: os << "On"; break;
        case 3: os << "Red-eye"; break;
        case 4: os << "Slow sync"; break;
        case 5: os << "Auto + red-eye"; break;
        case 6: os << "On + red-eye"; break;
        case 16: os << "External"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x0005(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value; 
        long l = value.toLong();
        switch (l) {
        case 0: os << "Single / timer"; break;
        case 1: os << "Continuous"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x0007(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "One shot"; break;
        case 1: os << "AI servo"; break;
        case 2: os << "AI Focus"; break;
        case 3: os << "MF"; break;
        case 4: os << "Single"; break;
        case 5: os << "Continuous"; break;
        case 6: os << "MF"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x000a(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Large"; break;
        case 1: os << "Medium"; break;
        case 2: os << "Small"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x000b(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case  0: os << "Full auto"; break;
        case  1: os << "Manual"; break;
        case  2: os << "Landscape"; break;
        case  3: os << "Fast shutter"; break;
        case  4: os << "Slow shutter"; break;
        case  5: os << "Night"; break;
        case  6: os << "B&W"; break;
        case  7: os << "Sepia"; break;
        case  8: os << "Portrait"; break;
        case  9: os << "Sports"; break;
        case 10: os << "Macro / close-up"; break;
        case 11: os << "Pan focus"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x000c(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "None"; break;
        case 1: os << "2x"; break;
        case 2: os << "4x"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs1Lnh(std::ostream& os,
                                              const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0xffff: os << "Low"; break;
        case 0x0000: os << "Normal"; break;
        case 0x0001: os << "High"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x0010(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case  0: os << "n/a"; break;
        case 15: os << "Auto"; break;
        case 16: os << "50"; break;
        case 17: os << "100"; break;
        case 18: os << "200"; break;
        case 19: os << "400"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x0011(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 3: os << "Evaluative"; break;
        case 4: os << "Partial"; break;
        case 5: os << "Center weighted"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x0012(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Manual"; break;
        case 1: os << "Auto"; break;
        case 3: os << "Close-up (macro)"; break;
        case 8: os << "Locked (pan mode)"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x0013(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0x3000: os << "None (MF)"; break;
        case 0x3001: os << "Auto-selected"; break;
        case 0x3002: os << "Right"; break;
        case 0x3003: os << "Center"; break;
        case 0x3004: os << "Left"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x0014(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Easy shooting"; break;
        case 1: os << "Program"; break;
        case 2: os << "Shutter priority"; break;
        case 3: os << "Aperture priority"; break;
        case 4: os << "Manual"; break;
        case 5: os << "A-DEP"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x001c(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Did not fire"; break;
        case 1: os << "Fired"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x001d(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        bool coma = false;
        if (l & 0x4000) {
            if (coma) os << ", ";
            os << "External TTL";
            coma = true;
        }
        if (l & 0x2000) {
            if (coma) os << ", ";
            os << "Internal flash";
            coma = true;
        }
        if (l & 0x0800) {
            if (coma) os << ", ";
            os << "FP sync used";
            coma = true;
        }
        if (l & 0x0080) {
            if (coma) os << ", ";
            os << "Rear curtain sync used";
            coma = true;
        }
        if (l & 0x0010) {
            if (coma) os << ", ";
            os << "FP sync enabled";
            coma = true;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs10x0020(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Single"; break;
        case 1: os << "Continuous"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs1Lens(std::ostream& os,
                                                const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        if (value.count() < 3) return os << value;

        float fu = value.toFloat(2);
        float len1 = value.toLong(0) / fu;
        float len2 = value.toLong(1) / fu;
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1)
           << len2 << " - " << len1 << " mm";
        os.copyfmt(oss);
        return os;
    }

    std::ostream& CanonMakerNote::printCs20x0002(std::ostream& os,
                                                 const Value& value)
    {
        // Ported from Exiftool by Will Stokes
        return os << exp(canonEv(value.toLong()) * log(2.0)) * 100.0 / 32.0;
    }

    std::ostream& CanonMakerNote::printCs20x0007(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Auto"; break;
        case 1: os << "Sunny"; break;
        case 2: os << "Cloudy"; break;
        case 3: os << "Tungsten"; break;
        case 4: os << "Fluorescent"; break;
        case 5: os << "Flash"; break;
        case 6: os << "Custom"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs20x0009(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        os << l << "";
        // Todo: determine unit
        return os;
    }

    std::ostream& CanonMakerNote::printCs20x000e(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        long num = (l & 0xf000) >> 12;
        os << num << " focus points; ";
        long used = l & 0x0fff;
        if (used == 0) {
            os << "none";
        }
        else {
            bool coma = false;
            if (l & 0x0004) {
                if (coma) os << ", ";
                os << "left";
                coma = true;
            }
            if (l & 0x0002) {
                if (coma) os << ", ";
                os << "center";
                coma = true;
            }
            if (l & 0x0001) {
                if (coma) os << ", ";
                os << "right";
                coma = true;
            }
        }
        os << " used";
        return os;
    }

    std::ostream& CanonMakerNote::printCs20x000f(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0xffc0: os << "-2 EV"; break;
        case 0xffcc: os << "-1.67 EV"; break;
        case 0xffd0: os << "-1.50 EV"; break;
        case 0xffd4: os << "-1.33 EV"; break;
        case 0xffe0: os << "-1 EV"; break;
        case 0xffec: os << "-0.67 EV"; break;
        case 0xfff0: os << "-0.50 EV"; break;
        case 0xfff4: os << "-0.33 EV"; break;
        case 0x0000: os << "0 EV"; break;
        case 0x000c: os << "0.33 EV"; break;
        case 0x0010: os << "0.50 EV"; break;
        case 0x0014: os << "0.67 EV"; break;
        case 0x0020: os << "1 EV"; break;
        case 0x002c: os << "1.33 EV"; break;
        case 0x0030: os << "1.50 EV"; break;
        case 0x0034: os << "1.67 EV"; break;
        case 0x0040: os << "2 EV"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs20x0013(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        if (l == 0xffff) {
            os << "Infinite";
        }
        else {
            os << l << "";
        }
        return os;
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createCanonMakerNote(bool alloc,
                                            const byte* buf, 
                                            long len, 
                                            ByteOrder byteOrder, 
                                            long offset)
    {
        return MakerNote::AutoPtr(new CanonMakerNote(alloc));
    }

}                                       // namespace Exiv2

// *****************************************************************************
// local definitions
namespace {

    float canonEv(long val)
    {
        // temporarily remove sign
        int sign = 1;
        if (val < 0) {
            sign = -1;
            val = -val;
        }
        // remove fraction
        float frac = static_cast<float>(val & 0x1f);
        val -= long(frac);
        // convert 1/3 (0x0c) and 2/3 (0x14) codes
        if (frac == 0x0c) {
            frac = 32.0f / 3;
        }
        else if (frac == 0x14) {
            frac = 64.0f / 3;
        }
        return sign * (val + frac) / 32.0f;
    }

}
