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
  Version:   $Name:  $ $Revision: 1.27 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Feb-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.27 $ $RCSfile: makernote.cpp,v $");

// Define DEBUG_* to output debug information to std::cerr
#undef DEBUG_MAKERNOTE
#undef DEBUG_REGISTRY

// *****************************************************************************
// included header files
#include "makernote.hpp"
#include "tags.hpp"                         // for ExifTags::ifdItem
#include "error.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>

#if defined DEBUG_MAKERNOTE || defined DEBUG_REGISTRY
#   include <iostream>
#endif

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    MakerNote::MakerNote(const MnTagInfo* pMnTagInfo, bool alloc) 
        : pMnTagInfo_(pMnTagInfo), alloc_(alloc),
          offset_(0), byteOrder_(invalidByteOrder)
    {
    }

    std::string MakerNote::makeKey(uint16_t tag) const
    {
        return std::string(ExifTags::familyName())
            + "." + std::string(ifdItem())
            + "." + tagName(tag);
    } // MakerNote::makeKey

    uint16_t MakerNote::decomposeKey(const std::string& key) const
    {
        // Get the family, item and tag name parts of the key
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string familyName = key.substr(0, pos1);
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string ifdItem = key.substr(pos0, pos1 - pos0);
        pos0 = pos1 + 1;
        std::string tagName = key.substr(pos0);
        if (tagName == "") throw Error("Invalid key");

        if (familyName != ExifTags::familyName()) return 0xffff;
        uint16_t tag = this->tag(tagName);
        if (tag == 0xffff) return tag;
        if (ifdItem != this->ifdItem()) return 0xffff;

        return tag;
    } // MakerNote::decomposeKey

    std::string MakerNote::tagName(uint16_t tag) const
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

    uint16_t MakerNote::tag(const std::string& tagName) const
    {
        uint16_t tag = 0xffff;
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

    std::string MakerNote::tagDesc(uint16_t tag) const
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

    std::ostream& MakerNote::writeMnTagInfo(std::ostream& os, uint16_t tag) const
    {
        return os << tagName(tag) << ", "
                  << std::dec << tag << ", "
                  << "0x" << std::setw(4) << std::setfill('0') 
                  << std::right << std::hex << tag << ", "
                  << ExifTags::ifdItem(makerIfdId) << ", "
                  << makeKey(tag) << ", " 
                  << tagDesc(tag);
    } // MakerNote::writeMnTagInfo


    IfdMakerNote::IfdMakerNote(const MakerNote::MnTagInfo* pMnTagInfo,
                               bool alloc)
        : MakerNote(pMnTagInfo, alloc),
          absOffset_(true), adjOffset_(0), ifd_(makerIfdId, 0, alloc)
    {
    }

    int IfdMakerNote::read(const byte* buf,
                           long len, 
                           ByteOrder byteOrder, 
                           long offset)
    {
        // Remember the offset
        offset_ = offset;
        // Set byte order if none is set yet
        if (byteOrder_ == invalidByteOrder) byteOrder_ = byteOrder;
        // Read and check the header (and set offset adjustment)
        int rc = readHeader(buf, len, byteOrder);
        if (rc == 0) {
            rc = checkHeader();
        }
        // Adjust the offset
        offset = absOffset_ ? offset + adjOffset_ : adjOffset_;
        // Read the makernote IFD
        if (rc == 0) {
            rc = ifd_.read(buf + headerSize(), 
                           len - headerSize(),
                           byteOrder_,
                           offset);
        }
        if (rc == 0) {
            // IfdMakerNote currently does not support multiple IFDs
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
#endif

        return rc;
    } // IfdMakerNote::read

    long IfdMakerNote::copy(byte* buf, ByteOrder byteOrder, long offset)
    {
        // Remember the new offset
        offset_ = offset;
        // Set byte order if none is set yet
        if (byteOrder_ == invalidByteOrder) byteOrder_ = byteOrder;
        // Adjust the offset
        offset = absOffset_ ? offset + adjOffset_ : adjOffset_;

        long len = 0;
        len += copyHeader(buf);
        len += ifd_.copy(buf + len, byteOrder_, offset);

        return len;
    } // IfdMakerNote::copy

    int IfdMakerNote::readHeader(const byte* buf, 
                                 long len,
                                 ByteOrder byteOrder)
    {
        // Default implementation does nothing, assuming there is no header
        return 0;
    }

    int IfdMakerNote::checkHeader() const
    {
        // Default implementation does nothing, assuming there is no header
        return 0;
    }

    long IfdMakerNote::copyHeader(byte* buf) const
    {
        if (header_.size_ != 0) memcpy(buf, header_.pData_, header_.size_);
        return header_.size_;
    }

    long IfdMakerNote::headerSize() const
    {
        return header_.size_;
    }

    Entries::const_iterator IfdMakerNote::findIdx(int idx) const 
    {
        return ifd_.findIdx(idx); 
    }

    long IfdMakerNote::size() const
    {
        return headerSize() + ifd_.size() + ifd_.dataSize();
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
#ifdef DEBUG_REGISTRY
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
                                        bool alloc,    
                                        const byte* buf, 
                                        long len, 
                                        ByteOrder byteOrder, 
                                        long offset) const
    {
#ifdef DEBUG_REGISTRY
        std::cerr << "Entering MakerNoteFactory::create(\"" 
                  << make << "\", \"" << model << "\", "
                  << (alloc == true ? "true" : "false") << ")\n";
#endif
        // loop through each make of the registry to find the best matching make
        int score = 0;
        ModelRegistry* modelRegistry = 0;
#ifdef DEBUG_REGISTRY
        std::string makeMatch;
        std::cerr << "Searching make registry...\n"; 
#endif
        Registry::const_iterator end1 = registry_.end();
        Registry::const_iterator pos1;
        for (pos1 = registry_.begin(); pos1 != end1; ++pos1) {
            int rc = match(pos1->first, make);
            if (rc > score) {
                score = rc;
#ifdef DEBUG_REGISTRY
                makeMatch = pos1->first;
#endif
                modelRegistry = pos1->second;
            }
        }
        if (modelRegistry == 0) return 0;
#ifdef DEBUG_REGISTRY
        std::cerr << "Best match is \"" << makeMatch << "\".\n";
#endif

        // loop through each model of the model registry to find the best match
        score = 0;
        CreateFct createMakerNote = 0;
#ifdef DEBUG_REGISTRY
        std::string modelMatch;
        std::cerr << "Searching model registry...\n";
#endif
        ModelRegistry::const_iterator end2 = modelRegistry->end();
        ModelRegistry::const_iterator pos2;
        for (pos2 = modelRegistry->begin(); pos2 != end2; ++pos2) {
            int rc = match(pos2->first, model);
            if (rc > score) {
                score = rc;
#ifdef DEBUG_REGISTRY
                modelMatch = pos2->first;
#endif
                createMakerNote = pos2->second;
            }
        }
        if (createMakerNote == 0) return 0;
#ifdef DEBUG_REGISTRY
        std::cerr << "Best match is \"" << modelMatch << "\".\n";
#endif

        return createMakerNote(alloc, buf, len, byteOrder, offset);
    } // MakerNoteFactory::create

    int MakerNoteFactory::match(const std::string& regEntry,
                                const std::string& key)
    {
#ifdef DEBUG_REGISTRY
        std::cerr << "   Matching registry entry \"" << regEntry << "\" (" 
                  << regEntry.size() << ") with key \"" << key << "\" ("
                  << key.size() << "): ";
#endif
        // Todo: make the comparisons case insensitive

        // Handle exact match (this is only necessary because of the different
        // return value - the following algorithm also finds exact matches)
        if (regEntry == key) {
#ifdef DEBUG_REGISTRY
            std::cerr << "Exact match (score: " << key.size() + 2 << ")\n";
#endif
            return static_cast<int>(key.size()) + 2;
        }
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
#ifdef DEBUG_REGISTRY
                    std::cerr << "Not a match.\n";
#endif
                    return 0;
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
                    count += static_cast<int>(ss.size());
                }
                else {
#ifdef DEBUG_REGISTRY
                    std::cerr << "Not a match.\n";
#endif
                    return 0;
                }
            } // if the substr is not empty

            ei = pos == std::string::npos ? std::string::npos : pos + 1;

        } // while ei doesn't point to the end of the registry entry

#ifdef DEBUG_REGISTRY
        std::cerr << "Match (score: " << count + 1 << ")\n";
#endif
        return count + 1;
        
    } // MakerNoteFactory::match

}                                       // namespace Exiv2
