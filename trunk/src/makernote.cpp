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
  File:      makernote.cpp
  Version:   $Name:  $ $Revision: 1.2 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Feb-04, ahu: created
  Credits:   Canon MakerNote implemented according to the specification
             "EXIF MakerNote of Canon" <http://www.burren.cx/david/canon.html>
             by David Burren
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.2 $ $RCSfile: makernote.cpp,v $")

// *****************************************************************************
// included header files
#include "makernote.hpp"
#include "value.hpp"
#include "tags.hpp"                         // for ExifTags::ifdItem, printValue
#include "error.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>

// Define DEBUG_MAKERNOTE to output debug information to std::cerr
#define DEBUG_MAKERNOTE

// *****************************************************************************
// class member definitions
namespace Exif {

    std::string MakerNote::makeKey(uint16 tag) const
    {
        return std::string(ExifTags::ifdItem(makerIfd))
            + "." + sectionName(tag) + "." + tagName(tag);
    } // MakerNote::makeKey

    uint16 MakerNote::decomposeKey(const std::string& key) const
    {
        // Get the IFD, section name and tag name parts of the key
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string ifdItem = key.substr(0, pos1);
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string sectionName = key.substr(pos0, pos1 - pos0);
        pos0 = pos1 + 1;
        std::string tagName = key.substr(pos0);
        if (tagName == "") throw Error("Invalid key");

        if (ifdItem != ExifTags::ifdItem(makerIfd)) return 0xffff;
        uint16 tag = this->tag(tagName);
        if (tag == 0xffff) return tag;
        if (sectionName != this->sectionName(tag)) return 0xffff;

        return tag;
    } // MakerNote::decomposeKey

    std::string MakerNote::tagName(uint16 tag) const
    {
        std::string tagName;
        if (mnTagInfo_) {
            for (int i = 0; mnTagInfo_[i].tag_ != 0xffff; ++i) {
                if (mnTagInfo_[i].tag_ == tag) {
                    tagName = mnTagInfo_[i].name_;
                    break;
                }
            }
        }
        if (tagName.empty()) {
            std::ostringstream os;
            os << "0x" << std::setw(4) << std::setfill('0') << std::right
               << std::hex << tag;
            tagName = os.str();
        }
        return tagName;
    } // MakerNote::tagName

    uint16 MakerNote::tag(const std::string& tagName) const
    {
        uint16 tag = 0xffff;
        if (mnTagInfo_) {
            for (int i = 0; mnTagInfo_[i].tag_ != 0xffff; ++i) {
                if (mnTagInfo_[i].name_ == tagName) {
                    tag = mnTagInfo_[i].tag_;
                    break;
                }
            }
        }
        if (tag == 0xffff) {
            std::istringstream is(tagName);
            is >> std::hex >> tag;
        }
        return tag;
    } // MakerNote::tag

    MakerNoteFactory* MakerNoteFactory::instance_ = 0;

    MakerNoteFactory& MakerNoteFactory::instance()
    {
        if (0 == instance_) {
            instance_ = new MakerNoteFactory;
        }
        return *instance_;
    } // MakerNoteFactory::instance

    void MakerNoteFactory::registerMakerNote(const std::string& make, 
                                             const std::string& model, 
                                             MakerNote* makerNote)
    {
        // Todo: use case insensitive make and model comparisons

        // find or create a registry entry for make
        ModelRegistry* modelRegistry = 0;
        Registry::const_iterator end1 = registry_.end();
        Registry::const_iterator pos1;
        for (pos1 = registry_.begin(); pos1 != end1; ++pos1) {
            if (pos1->first == make) break;
        }
        if (pos1 != end1) {
            modelRegistry = pos1->second;
        }
        else {
            modelRegistry = new ModelRegistry;
            registry_.push_back(std::make_pair(make, modelRegistry));
        }
        // find or create a registry entry for model
        ModelRegistry::iterator end2 = modelRegistry->end();
        ModelRegistry::iterator pos2;
        for (pos2 = modelRegistry->begin(); pos2 != end2; ++pos2) {
            if (pos2->first == model) break;
        }
        if (pos2 != end2) {
            delete pos2->second;
            pos2->second = makerNote;
        }
        else {
            modelRegistry->push_back(std::make_pair(model, makerNote));
        }
    } // MakerNoteFactory::registerMakerNote

    MakerNoteFactory::MakerNoteFactory()
    {
        // Register a prototype of each known MakerNote
        registerMakerNote("Canon", "*", new CanonMakerNote);
    } // MakerNoteFactory c'tor

    MakerNote* MakerNoteFactory::create(const std::string& make, 
                                        const std::string& model) const
    {
        // loop through each make of the registry to find the best matching make
        int matchCount = -1;
        ModelRegistry* modelRegistry = 0;
        Registry::const_iterator end1 = registry_.end();
        Registry::const_iterator pos1;
        for (pos1 = registry_.begin(); pos1 != end1; ++pos1) {
            std::pair<bool, int> rc = match(pos1->first, make);
            if (rc.first && rc.second > matchCount) {
                matchCount = rc.second;
                modelRegistry = pos1->second;
            }
        }
        if (modelRegistry == 0) return 0;

        // loop through each model of the model registry to find the best match
        matchCount = -1;
        MakerNote* makerNote = 0;
        ModelRegistry::const_iterator end2 = modelRegistry->end();
        ModelRegistry::const_iterator pos2;
        for (pos2 = modelRegistry->begin(); pos2 != end2; ++pos2) {
            std::pair<bool, int> rc = match(pos2->first, model);
            if (rc.first && rc.second > matchCount) {
                matchCount = rc.second;
                makerNote = pos2->second;
            }
        }
        if (makerNote == 0) return 0;

        return makerNote->clone();
    } // MakerNoteFactory::create

    std::pair<bool, int> MakerNoteFactory::match(const std::string& regEntry,
                                                 const std::string& key)
    {
        // Todo: make the comparisons case insensitive

        std::string uKey = key;
        std::string uReg = regEntry;

        int count = 0;                          // number of matching characters
        std::string::size_type ei = 0;          // index in the registry entry
        std::string::size_type ki = 0;          // index in the key

        while (ei != std::string::npos) {

            std::string::size_type pos = uReg.find('*', ei);
            if (pos != ei) {
                std::string ss = pos == std::string::npos ?
                    uReg.substr(ei) : uReg.substr(ei, pos - ei);

                if (ki == std::string::npos) {
                    return std::make_pair(false, 0);
                }

                bool found = false;
                // Find the substr ss in the key starting from index ki. 
                // Take care of the special cases
                //   + where the substr must match the key from beg to end,
                //   + from beg,
                //   + to end 
                //   + and where it can be anywhere in the key.
                // If found, ki is adjusted to the position in the key after ss.
                if (ei == 0 && pos == std::string::npos) { // ei == 0 => ki == 0
                    if (0 == uKey.compare(ss)) {
                        found = true;
                        ki = std::string::npos;
                    }
                }
                else if (ei == 0) { // ei == 0 => ki == 0
                    if (0 == uKey.compare(0, ss.size(), ss)) {
                        found = true;
                        ki = ss.size();
                    }
                }
                else if (pos == std::string::npos) {
                    if (   ss.size() <= uKey.size() 
                        && ki <= uKey.size() - ss.size()) {
                        if (0 == uKey.compare(
                                uKey.size() - ss.size(), ss.size(), ss)) {
                            found = true;
                            ki = std::string::npos;
                        }
                    }
                }
                else {
                    std::string::size_type idx = uKey.find(ss, ki); 
                    if (idx != std::string::npos) {
                        found = true;
                        ki = idx + ss.size();
                    }
                }

                if (found) {
                    count += ss.size();
                }
                else {
                    return std::make_pair(false, 0);
                }
            } // if the substr is not empty

            ei = pos == std::string::npos ? std::string::npos : pos + 1;

        } // while ei doesn't point to the end of the registry entry
                                     
        return std::make_pair(true, count);
        
    } // MakerNoteFactory::match

    int IfdMakerNote::read(const char* buf,
                           long len, 
                           ByteOrder byteOrder, 
                           long offset)
    {
        int rc = ifd_.read(buf, byteOrder, offset);
        if (rc == 0) {
            // Todo: Make sure the Next field is 0.
            Entries::iterator end = ifd_.end();
            for (Entries::iterator i = ifd_.begin(); i != end; ++i) {
                i->setMakerNote(this);
            }
        }
#ifdef DEBUG_MAKERNOTE
        hexdump(std::cerr, buf, len);
        if (rc == 0) ifd_.print(std::cerr);
#endif
        return rc;
    }

    long IfdMakerNote::copy(char* buf, ByteOrder byteOrder, long offset) const
    {
        return ifd_.copy(buf, byteOrder, offset);
    }

    long IfdMakerNote::size() const
    {
         return ifd_.size() + ifd_.dataSize();
    }

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

    CanonMakerNote::CanonMakerNote()
        : IfdMakerNote(canonMnTagInfo), sectionName_("Canon")
    {
    }

    MakerNote* CanonMakerNote::clone() const 
    {
        return new CanonMakerNote(*this); 
    }

    std::ostream& CanonMakerNote::printTag(std::ostream& os, 
                                           uint16 tag, 
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

    std::ostream& CanonMakerNote::print0x0001(
        std::ostream& os, const Value& value) const
    {
        if (0 == dynamic_cast<const UShortValue*>(&value)) {
            return os << value;
        }
        const UShortValue& val = dynamic_cast<const UShortValue&>(value);
        uint32 count = val.count();

        if (count < 2) return os;
        uint16 s = val.value_[1];
        os << std::setw(30) << "\n   Macro mode ";
        switch (s) {
        case 1: os << "On"; break;
        case 2: os << "Off"; break;
        default: os << "(" << s << ")"; break;
        }

        if (count < 3) return os;
        s = val.value_[2];
        os << std::setw(30) << "\n   Self timer ";
        if (s == 0) {
            os << "Off";
        }
        else { 
            os << s / 10.0 << " s";
        }
        if (count < 4) return os;
        s = val.value_[3];
        os << std::setw(30) << "\n   Quality ";
        switch (s) {
        case 2: os << "Normal"; break;
        case 3: os << "Fine"; break;
        case 5: os << "Superfine"; break;
        default: os << "(" << s << ")"; break;
        }
        
        if (count < 5) return os;
        s = val.value_[4];
        os << std::setw(30) << "\n   Flash mode ";
        switch (s) {
        case 0: os << "Off"; break;
        case 1: os << "Auto"; break;
        case 2: os << "On"; break;
        case 3: os << "Red-eye"; break;
        case 4: os << "Slow sync"; break;
        case 5: os << "Auto + red-eye"; break;
        case 6: os << "On + red-eye"; break;
        case 16: os << "External"; break;
        default: os << "(" << s << ")"; break;
        }

        if (count < 6) return os;
        s = val.value_[5];
        os << std::setw(30) << "\n   Drive mode ";
        switch (s) {
        case 0: os << "Single / timer"; break;
        case 1: os << "Continuous"; break;
        default: os << "(" << s << ")"; break;
        }
    
        // Meaning of the 6th ushort is unknown - ignore it

        if (count < 8) return os;
        s = val.value_[7];
        os << std::setw(30) << "\n   Focus mode ";
        switch (s) {
        case 0: os << "One shot"; break;
        case 1: os << "AI servo"; break;
        case 2: os << "AI Focus"; break;
        case 3: os << "MF"; break;
        case 4: os << "Single"; break;
        case 5: os << "Continuous"; break;
        case 6: os << "MF"; break;
        default: os << "(" << s << ")"; break;
        }
    
        // Meaning of the 8th ushort is unknown - ignore it
        // Meaning of the 9th ushort is unknown - ignore it

        if (count < 11) return os;
        s = val.value_[10];
        os << std::setw(30) << "\n   Image size ";
        switch (s) {
        case 0: os << "Large"; break;
        case 1: os << "Medium"; break;
        case 2: os << "Small"; break;
        default: os << "(" << s << ")"; break;
        }

        if (count < 12) return os;
        s = val.value_[11];
        os << std::setw(30) << "\n   Easy shooting mode ";
        switch (s) {
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
        default: os << "(" << s << ")"; break;
        }
 
        if (count < 13) return os;
        s = val.value_[12];
        os << std::setw(30) << "\n   Digital zoom ";
        switch (s) {
        case 0: os << "None"; break;
        case 1: os << "2x"; break;
        case 2: os << "4x"; break;
        default: os << "(" << s << ")"; break;
        }

        if (count < 14) return os;
        s = val.value_[13];
        os << std::setw(30) << "\n   Contrast ";
        switch (s) {
        case 0xffff: os << "Low"; break;
        case 0x0000: os << "Normal"; break;
        case 0x0001: os << "High"; break;
        default: os << "(" << s << ")"; break;
        }

        if (count < 15) return os;
        s = val.value_[14];
        os << std::setw(30) << "\n   Saturation ";
        switch (s) {
        case 0xffff: os << "Low"; break;
        case 0x0000: os << "Normal"; break;
        case 0x0001: os << "High"; break;
        default: os << "(" << s << ")"; break;
        }

        if (count < 16) return os;
        s = val.value_[15];
        os << std::setw(30) << "\n   Sharpness ";
        switch (s) {
        case 0xffff: os << "Low"; break;
        case 0x0000: os << "Normal"; break;
        case 0x0001: os << "High"; break;
        default: os << "(" << s << ")"; break;
        }

        if (count < 17) return os;
        s = val.value_[16];
        if (s != 0) {
            os << std::setw(30) << "\n   ISO ";
            switch (s) {
            case 15: os << "Auto"; break;
            case 16: os << "50"; break;
            case 17: os << "100"; break;
            case 18: os << "200"; break;
            case 19: os << "400"; break;
            default: os << "(" << s << ")"; break;
            }
        }

        if (count < 18) return os;
        s = val.value_[17];
        os << std::setw(30) << "\n   Metering mode ";
        switch (s) {
        case 3: os << "Evaluative"; break;
        case 4: os << "Partial"; break;
        case 5: os << "Center weighted"; break;
        default: os << "(" << s << ")"; break;
        }

        if (count < 19) return os;
        s = val.value_[18];
        os << std::setw(30) << "\n   Focus type ";
        switch (s) {
        case 0: os << "Manual"; break;
        case 1: os << "Auto"; break;
        case 3: os << "Close-up (macro)"; break;
        case 8: os << "Locked (pan mode)"; break;
        default: os << "(" << s << ")"; break;
        }

        if (count < 20) return os;
        s = val.value_[19];
        os << std::setw(30) << "\n   AF point selected ";
        switch (s) {
        case 0x3000: os << "None (MF)"; break;
        case 0x3001: os << "Auto-selected"; break;
        case 0x3002: os << "Right"; break;
        case 0x3003: os << "Center"; break;
        case 0x3004: os << "Left"; break;
        default: os << "(" << s << ")"; break;
        }

        if (count < 21) return os;
        s = val.value_[20];
        os << std::setw(30) << "\n   Exposure mode ";
        switch (s) {
        case 0: os << "Easy shooting"; break;
        case 1: os << "Program"; break;
        case 2: os << "Tv priority"; break;
        case 3: os << "Av priority"; break;
        case 4: os << "Manual"; break;
        case 5: os << "A-DEP"; break;
        default: os << "(" << s << ")"; break;
        }

        // Meaning of the 21st ushort is unknown - ignore it
        // Meaning of the 22nd ushort is unknown - ignore it

        if (count < 26) return os;
        float fu = val.value_[25];
        float len1 = val.value_[23] / fu;
        float len2 = val.value_[24] / fu;
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::setw(30) << "\n   Lens "
           << std::fixed << std::setprecision(1)
           << len2 << " - " << len1 << " mm";
        os.copyfmt(oss);

        // Meaning of the 26th ushort is unknown - ignore it
        // Meaning of the 27th ushort is unknown - ignore it

        if (count < 29) return os;
        s = val.value_[28];
        os << std::setw(30) << "\n   Flash activity ";
        switch (s) {
        case 0: os << "Did not fire"; break;
        case 1: os << "Fired"; break;
        default: os << "(" << s << ")"; break;
        }

        if (count < 30) return os;
        s = val.value_[29];
        // Todo: decode bitmask
        os << std::setw(30) << "\n   Flash details " 
           << std::dec << s << " (Todo: decode bitmask)";

        // Meaning of the 30th ushort is unknown - ignore it
        // Meaning of the 31st ushort is unknown - ignore it

        if (count < 33) return os;
        s = val.value_[32];
        os << std::setw(30) << "\n   Focus mode ";
        switch (s) {
        case 0: os << "Single"; break;
        case 1: os << "Continuous"; break;
        default: os << "(" << s << ")"; break;
        }

        // Meaning of any further ushorts is unknown - ignore them
        
        return os;

    } // CanonMakerNote::print0x0001

    std::ostream& CanonMakerNote::print0x0004(
        std::ostream& os, const Value& value) const
    {
        if (0 == dynamic_cast<const UShortValue*>(&value)) {
            return os << value;
        }
        const UShortValue& val = dynamic_cast<const UShortValue&>(value);
        uint32 count = val.count();

        // Meaning of ushorts 1-6 is unknown - ignore them

        if (count < 8) return os;
        uint16 s = val.value_[7];
        os << std::setw(30) << "\n   White balance ";
        switch (s) {
        case 0: os << "Auto"; break;
        case 1: os << "Sunny"; break;
        case 2: os << "Cloudy"; break;
        case 3: os << "Tungsten"; break;
        case 4: os << "Fluorescent"; break;
        case 5: os << "Flash"; break;
        case 6: os << "Custom"; break;
        default: os << "(" << s << ")"; break;
        }

        // Meaning of ushort 8 is unknown - ignore it

        if (count < 10) return os;
        s = val.value_[9];
        os << std::setw(30) << "\n   Sequence number " << s << "";

        // Meaning of ushorts 10-13 is unknown - ignore them

        if (count < 15) return os;
        s = val.value_[14];
        // Todo: decode bitmask
        os << std::setw(30) << "\n   AF point used " 
           << s << " (Todo: decode bitmask)";
        
        if (count < 16) return os;
        s = val.value_[15];
        os << std::setw(30) << "\n   Flash bias ";
        switch (s) {
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
        default: os << "(" << s << ")"; break;
        }

        // Meaning of ushorts 16-18 is unknown - ignore them

        if (count < 20) return os;
        s = val.value_[19];
        os << std::setw(30) << "\n   Subject distance (0.01m or 0.001m) ";
        if (s == 0xffff) {
            os << "Infinite";
        }
        else {
            os << s << "";
        }

        return os;

    } // CanonMakerNote::print0x0004

    std::ostream& CanonMakerNote::print0x0008(
        std::ostream& os, const Value& value) const
    {
        std::string n = value.toString();
        return os << n.substr(0, n.length() - 4) << "-" 
                  << n.substr(n.length() - 4);
    }

    std::ostream& CanonMakerNote::print0x000c(
        std::ostream& os, const Value& value) const
    {
        std::istringstream is(value.toString());
        uint32 l;
        is >> l;
        return os << std::setw(4) << std::setfill('0') << std::hex 
                  << ((l & 0xffff0000) >> 16)
                  << std::setw(5) << std::setfill('0') << std::dec
                  << (l & 0x0000ffff);
    }

    std::ostream& CanonMakerNote::print0x000f(
        std::ostream& os, const Value& value) const
    {
        // Todo: Decode EOS D30 Custom Functions
        return os << "EOS D30 Custom Functions "
                  << value << " (Todo: decode this field)";
    }

}                                       // namespace Exif
