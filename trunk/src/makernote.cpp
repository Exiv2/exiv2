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
  Version:   $Name:  $ $Revision: 1.16 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Feb-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.16 $ $RCSfile: makernote.cpp,v $")

// Define DEBUG_MAKERNOTE to output debug information to std::cerr
#undef DEBUG_MAKERNOTE

// *****************************************************************************
// included header files
#include "makernote.hpp"
#include "tags.hpp"                         // for ExifTags::ifdItem
#include "error.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>

#ifdef DEBUG_MAKERNOTE
#   include <iostream>
#endif

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
        if (pMnTagInfo_) {
            for (int i = 0; pMnTagInfo_[i].tag_ != 0xffff; ++i) {
                if (pMnTagInfo_[i].tag_ == tag) {
                    tagName = pMnTagInfo_[i].name_;
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
        if (pMnTagInfo_) {
            for (int i = 0; pMnTagInfo_[i].tag_ != 0xffff; ++i) {
                if (pMnTagInfo_[i].name_ == tagName) {
                    tag = pMnTagInfo_[i].tag_;
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

    std::string MakerNote::tagDesc(uint16 tag) const
    {
        std::string tagDesc;
        if (pMnTagInfo_) {
            for (int i = 0; pMnTagInfo_[i].tag_ != 0xffff; ++i) {
                if (pMnTagInfo_[i].tag_ == tag) {
                    tagDesc = pMnTagInfo_[i].desc_;
                    break;
                }
            }
        }
        return tagDesc;
    } // MakerNote::tagDesc

    void MakerNote::taglist(std::ostream& os) const
    {
        if (pMnTagInfo_) {
            for (int i = 0; pMnTagInfo_[i].tag_ != 0xffff; ++i) {
                writeMnTagInfo(os, pMnTagInfo_[i].tag_) << std::endl;
            }
        }
    } // MakerNote::taglist

    std::ostream& MakerNote::writeMnTagInfo(std::ostream& os, uint16 tag) const
    {
        return os << tagName(tag) << ", "
                  << std::dec << tag << ", "
                  << "0x" << std::setw(4) << std::setfill('0') 
                  << std::right << std::hex << tag << ", "
                  << ExifTags::ifdItem(makerIfd) << ", "
                  << makeKey(tag) << ", " 
                  << tagDesc(tag);
    } // MakerNote::writeMnTagInfo

    int IfdMakerNote::read(const char* buf,
                           long len, 
                           ByteOrder byteOrder, 
                           long offset)
    {
        // Remember the offset
        offset_ = offset;
        // Set byte order if none is set yet
        if (byteOrder_ == invalidByteOrder) byteOrder_ = byteOrder;
        int rc = 0;
        if (!prefix_.empty()) {
            // Check if makernote is long enough and starts with prefix
            if (   len <= static_cast<long>(prefix_.size())
                || prefix_ != std::string(buf, prefix_.size())) rc = 2;
        }
        if (!absOffset_) {
            // Use offsets relative to the start of the Makernote field
            offset = 0;
        }
        if (rc == 0) {
            rc = ifd_.read(buf + prefix_.size(), 
                           byteOrder_, 
                           offset + prefix_.size());
        }
        if (rc == 0) {
            // IfdMakerNote does not support multiple IFDs
            if (ifd_.next() != 0) rc = 3;
        }
        if (rc == 0) {
            Entries::iterator end = ifd_.end();
            for (Entries::iterator i = ifd_.begin(); i != end; ++i) {
                i->setMakerNote(this);
            }
        }
#ifdef DEBUG_MAKERNOTE
        hexdump(std::cerr, buf, len, offset);
        if (rc == 0) ifd_.print(std::cerr);
        else std::cerr << "IfdMakerNote::read() failed, rc = " << rc << "\n";
#endif
        return rc;
    } // IfdMakerNote::read

    long IfdMakerNote::copy(char* buf, ByteOrder byteOrder, long offset)
    {
        // Remember the new offset
        offset_ = offset;
        // Set byte order if none is set yet
        if (byteOrder_ == invalidByteOrder) byteOrder_ = byteOrder;
        long len = 0;
        if (!prefix_.empty()) {
            // Write the prefix string to the Makernote buffer
            memcpy(buf, prefix_.data(), prefix_.size());
            len += prefix_.size();
        }
        if (!absOffset_) {
            // Use offsets relative to the start of the Makernote field
            offset = 0;
        }
        len += ifd_.copy(buf + len, byteOrder_, offset + len);
        return len;
    } // IfdMakerNote::copy

    Entries::const_iterator IfdMakerNote::findIdx(int idx) const 
    {
        return ifd_.findIdx(idx); 
    }

    long IfdMakerNote::size() const
    {
        return prefix_.size() + ifd_.size() + ifd_.dataSize();
    }

    MakerNoteFactory* MakerNoteFactory::pInstance_ = 0;

    MakerNoteFactory& MakerNoteFactory::instance()
    {
        if (0 == pInstance_) {
            pInstance_ = new MakerNoteFactory;
        }
        return *pInstance_;
    } // MakerNoteFactory::instance

    void MakerNoteFactory::registerMakerNote(const std::string& make, 
                                             const std::string& model, 
                                             CreateFct createMakerNote)
    {
#ifdef DEBUG_MAKERNOTE
        std::cerr << "Registering MakerNote create function for \"" 
                  << make << "\" and \"" << model << "\".\n";
#endif

        // Todo: use case insensitive make and model comparisons

        // Find or create a registry entry for make
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
        // Find or create a registry entry for model
        ModelRegistry::iterator end2 = modelRegistry->end();
        ModelRegistry::iterator pos2;
        for (pos2 = modelRegistry->begin(); pos2 != end2; ++pos2) {
            if (pos2->first == model) break;
        }
        if (pos2 != end2) {
            pos2->second = createMakerNote;
        }
        else {
            modelRegistry->push_back(std::make_pair(model, createMakerNote));
        }
    } // MakerNoteFactory::registerMakerNote

    MakerNote* MakerNoteFactory::create(const std::string& make, 
                                        const std::string& model,
                                        bool alloc) const
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
        CreateFct createMakerNote = 0;
        ModelRegistry::const_iterator end2 = modelRegistry->end();
        ModelRegistry::const_iterator pos2;
        for (pos2 = modelRegistry->begin(); pos2 != end2; ++pos2) {
            std::pair<bool, int> rc = match(pos2->first, model);
            if (rc.first && rc.second > matchCount) {
                matchCount = rc.second;
                createMakerNote = pos2->second;
            }
        }
        if (createMakerNote == 0) return 0;

        return createMakerNote(alloc);
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

}                                       // namespace Exif
