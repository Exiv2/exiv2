// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
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
  Version:   $Name:  $ $Revision: 1.14 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Feb-04, ahu: created
             07-Mar-04, ahu: isolated as a separate component
  Credits:   Canon MakerNote implemented according to the specification
             "EXIF MakerNote of Canon" <http://www.burren.cx/david/canon.html>
             by David Burren
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.14 $ $RCSfile: canonmn.cpp,v $");

// *****************************************************************************
// included header files
#include "types.hpp"
#include "canonmn.hpp"
#include "makernote.hpp"
#include "value.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>

// Define DEBUG_MAKERNOTE to output debug information to std::cerr
#undef DEBUG_MAKERNOTE

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const CanonMakerNote::RegisterMakerNote CanonMakerNote::register_;

    // Canon MakerNote Tag Info
    static const MakerNote::MnTagInfo canonMnTagInfo[] = {
        MakerNote::MnTagInfo(0x0001, "CameraSettings1", "Various camera settings (1)"),
        MakerNote::MnTagInfo(0x0004, "CameraSettings2", "Various camera settings (2)"),
        MakerNote::MnTagInfo(0x0006, "ImageType", "Image type"),
        MakerNote::MnTagInfo(0x0007, "FirmwareVersion", "Firmware version"),
        MakerNote::MnTagInfo(0x0008, "ImageNumber", "Image number"),
        MakerNote::MnTagInfo(0x0009, "OwnerName", "Owner Name"),
        MakerNote::MnTagInfo(0x000c, "SerialNumber", "Camera serial number"),
        MakerNote::MnTagInfo(0x000f, "EosD30Functions", "EOS D30 Custom Functions"),
        // End of list marker
        MakerNote::MnTagInfo(0xffff, "(UnknownCanonMakerNoteTag)", "Unknown CanonMakerNote tag")
    };

    CanonMakerNote::CanonMakerNote(bool alloc)
        : IfdMakerNote(canonMnTagInfo, alloc), ifdItem_("Canon")
    {
    }

    CanonMakerNote* CanonMakerNote::clone(bool alloc) const 
    {
        return new CanonMakerNote(alloc); 
    }

    std::ostream& CanonMakerNote::printTag(std::ostream& os, 
                                           uint16_t tag, 
                                           const Value& value) const
    {
        switch (tag) {
        case 0x0001: print0x0001(os, value); break;
        case 0x0004: print0x0004(os, value); break;
        case 0x0008: print0x0008(os, value); break;
        case 0x000c: print0x000c(os, value); break;
        case 0x000f: print0x000f(os, value); break;
        default:
            // All other tags (known or unknown) go here
            os << value;
            break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x0001(std::ostream& os, 
                                              const Value& value)
    {
        if (value.typeId() != unsignedShort) {
            return os << value;
        }
        long count = value.count();

        if (count < 2) return os;
        long l = value.toLong(1);
        os << std::setw(30) << "\n   Macro mode ";
        print0x0001_01(os, l);

        if (count < 3) return os;
        l = value.toLong(2);
        os << std::setw(30) << "\n   Self timer ";
        print0x0001_02(os, l);

        if (count < 4) return os;
        l = value.toLong(3);
        os << std::setw(30) << "\n   Quality ";
        print0x0001_03(os, l);

        if (count < 5) return os;
        l = value.toLong(4);
        os << std::setw(30) << "\n   Flash mode ";
        print0x0001_04(os, l);

        if (count < 6) return os;
        l = value.toLong(5);
        os << std::setw(30) << "\n   Drive mode ";
        print0x0001_05(os, l);

        // Meaning of the 6th ushort is unknown - ignore it

        if (count < 8) return os;
        l = value.toLong(7);
        os << std::setw(30) << "\n   Focus mode ";
        print0x0001_07(os, l);
    
        // Meaning of the 8th ushort is unknown - ignore it
        // Meaning of the 9th ushort is unknown - ignore it

        if (count < 11) return os;
        l = value.toLong(10);
        os << std::setw(30) << "\n   Image size ";
        print0x0001_10(os, l);

        if (count < 12) return os;
        l = value.toLong(11);
        os << std::setw(30) << "\n   Easy shooting mode ";
        print0x0001_11(os, l);
 
        if (count < 13) return os;
        l = value.toLong(12);
        os << std::setw(30) << "\n   Digital zoom ";
        print0x0001_12(os, l);

        if (count < 14) return os;
        l = value.toLong(13);
        os << std::setw(30) << "\n   Contrast ";
        print0x0001_lnh(os, l);

        if (count < 15) return os;
        l = value.toLong(14);
        os << std::setw(30) << "\n   Saturation ";
        print0x0001_lnh(os, l);

        if (count < 16) return os;
        l = value.toLong(15);
        os << std::setw(30) << "\n   Sharpness ";
        print0x0001_lnh(os, l);

        if (count < 17) return os;
        l = value.toLong(16);
        if (l != 0) {
            os << std::setw(30) << "\n   ISO ";
            print0x0001_16(os, l);
        }

        if (count < 18) return os;
        l = value.toLong(17);
        os << std::setw(30) << "\n   Metering mode ";
        print0x0001_17(os, l);

        if (count < 19) return os;
        l = value.toLong(18);
        os << std::setw(30) << "\n   Focus type ";
        print0x0001_18(os, l);

        if (count < 20) return os;
        l = value.toLong(19);
        os << std::setw(30) << "\n   AF point selected ";
        print0x0001_19(os, l);

        if (count < 21) return os;
        l = value.toLong(20);
        os << std::setw(30) << "\n   Exposure mode ";
        print0x0001_20(os, l);

        // Meaning of the 21st ushort is unknown - ignore it
        // Meaning of the 22nd ushort is unknown - ignore it

        if (count < 26) return os;
        os << std::setw(30) << "\n   Lens ";
        print0x0001_Lens(os, value);

        // Meaning of the 26th ushort is unknown - ignore it
        // Meaning of the 27th ushort is unknown - ignore it

        if (count < 29) return os;
        l = value.toLong(28);
        os << std::setw(30) << "\n   Flash activity ";
        print0x0001_28(os, l);

        if (count < 30) return os;
        l = value.toLong(29);
        if (l > 0) {
            os << std::setw(30) << "\n   Flash details ";
            print0x0001_29(os, l);
        }

        // Meaning of the 30th ushort is unknown - ignore it
        // Meaning of the 31st ushort is unknown - ignore it

        if (count < 33) return os;
        l = value.toLong(32);
        os << std::setw(30) << "\n   Focus mode ";
        print0x0001_32(os, l);

        // Meaning of any further ushorts is unknown - ignore them
        
        return os;

    } // CanonMakerNote::print0x0001

    std::ostream& CanonMakerNote::print0x0004(std::ostream& os, 
                                              const Value& value)
    {
        if (value.typeId() != unsignedShort) {
            return os << value;
        }
        long count = value.count();

        // Meaning of ushorts 1-6 is unknown - ignore them

        if (count < 8) return os;
        long l = value.toLong(7);
        os << std::setw(30) << "\n   White balance ";
        print0x0004_07(os, l);

        // Meaning of ushort 8 is unknown - ignore it

        if (count < 10) return os;
        l = value.toLong(9);
        os << std::setw(30) << "\n   Sequence number ";
        print0x0004_09(os, l);

        // Meaning of ushorts 10-13 is unknown - ignore them

        if (count < 15) return os;
        l = value.toLong(14);
        os << std::setw(30) << "\n   AF point used ";
        print0x0004_14(os, l);

        if (count < 16) return os;
        l = value.toLong(15);
        os << std::setw(30) << "\n   Flash bias ";
        print0x0004_15(os, l);

        // Meaning of ushorts 16-18 is unknown - ignore them

        if (count < 20) return os;
        l = value.toLong(19);
        os << std::setw(30) << "\n   Subject distance (0.01m or 0.001m) ";
        print0x0004_19(os, l);

        return os;

    } // CanonMakerNote::print0x0004

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

    std::ostream& CanonMakerNote::print0x000f(std::ostream& os, 
                                              const Value& value)
    {
        // Todo: Decode EOS D30 Custom Functions
        return os << "EOS D30 Custom Functions "
                  << value << " (Todo: decode this field)";
    }

    std::ostream& CanonMakerNote::print0x0001_01(std::ostream& os, long l)
    {
        switch (l) {
        case 1: os << "On"; break;
        case 2: os << "Off"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x0001_02(std::ostream& os, long l)
    {
        if (l == 0) {
            os << "Off";
        }
        else { 
            os << l / 10.0 << " s";
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x0001_03(std::ostream& os, long l)
    {
        switch (l) {
        case 2: os << "Normal"; break;
        case 3: os << "Fine"; break;
        case 5: os << "Superfine"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x0001_04(std::ostream& os, long l)
    {
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

    std::ostream& CanonMakerNote::print0x0001_05(std::ostream& os, long l)
    {
        switch (l) {
        case 0: os << "Single / timer"; break;
        case 1: os << "Continuous"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x0001_07(std::ostream& os, long l)
    {
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

    std::ostream& CanonMakerNote::print0x0001_10(std::ostream& os, long l)
    {
        switch (l) {
        case 0: os << "Large"; break;
        case 1: os << "Medium"; break;
        case 2: os << "Small"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x0001_11(std::ostream& os, long l)
    {
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

    std::ostream& CanonMakerNote::print0x0001_12(std::ostream& os, long l)
    {
        switch (l) {
        case 0: os << "None"; break;
        case 1: os << "2x"; break;
        case 2: os << "4x"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x0001_lnh(std::ostream& os, long l)
    {
        switch (l) {
        case 0xffff: os << "Low"; break;
        case 0x0000: os << "Normal"; break;
        case 0x0001: os << "High"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x0001_16(std::ostream& os, long l)
    {
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

    std::ostream& CanonMakerNote::print0x0001_17(std::ostream& os, long l)
    {
        switch (l) {
        case 3: os << "Evaluative"; break;
        case 4: os << "Partial"; break;
        case 5: os << "Center weighted"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x0001_18(std::ostream& os, long l)
    {
        switch (l) {
        case 0: os << "Manual"; break;
        case 1: os << "Auto"; break;
        case 3: os << "Close-up (macro)"; break;
        case 8: os << "Locked (pan mode)"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x0001_19(std::ostream& os, long l)
    {
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

    std::ostream& CanonMakerNote::print0x0001_20(std::ostream& os, long l)
    {
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

    std::ostream& CanonMakerNote::print0x0001_28(std::ostream& os, long l)
    {
        switch (l) {
        case 0: os << "Did not fire"; break;
        case 1: os << "Fired"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x0001_29(std::ostream& os, long l)
    {
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

    std::ostream& CanonMakerNote::print0x0001_32(std::ostream& os, long l)
    {
        switch (l) {
        case 0: os << "Single"; break;
        case 1: os << "Continuous"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x0001_Lens(std::ostream& os, 
                                                   const Value& value)
    {
        if (value.typeId() != unsignedShort) {
            return os;
        }
        if (value.count() < 26) return os;

        // Todo: why not use toFloat()?
        float fu = static_cast<float>(value.toLong(25));
        float len1 = value.toLong(23) / fu;
        float len2 = value.toLong(24) / fu;
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1)
           << len2 << " - " << len1 << " mm";
        os.copyfmt(oss);
        return os;
    }

    std::ostream& CanonMakerNote::print0x0004_07(std::ostream& os, long l)
    {
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

    std::ostream& CanonMakerNote::print0x0004_09(std::ostream& os, long l)
    {
        os << l << "";
        // Todo: determine unit
        return os;
    }

    std::ostream& CanonMakerNote::print0x0004_14(std::ostream& os, long l)
    {
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

    std::ostream& CanonMakerNote::print0x0004_15(std::ostream& os, long l)
    {
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

    std::ostream& CanonMakerNote::print0x0004_19(std::ostream& os, long l)
    {
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

    MakerNote* createCanonMakerNote(bool alloc,
                                    const byte* buf, 
                                    long len, 
                                    ByteOrder byteOrder, 
                                    long offset)
    {
        return new CanonMakerNote(alloc);         
    }

}                                       // namespace Exiv2
