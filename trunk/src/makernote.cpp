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
/*
  File:      makernote.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Feb-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// Define DEBUG_* to output debug information to std::cerr, e.g, by calling
// make like this: make DEFS=-DDEBUG_MAKERNOTE makernote.o
//#define DEBUG_MAKERNOTE
//#define DEBUG_REGISTRY

// *****************************************************************************
// included header files
#include "makernote.hpp"
#include "error.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    MakerNote::MakerNote(bool alloc)
        : alloc_(alloc), offset_(0), byteOrder_(invalidByteOrder)
    {
    }

    MakerNote::AutoPtr MakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    MakerNote::AutoPtr MakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    IfdMakerNote::IfdMakerNote(IfdId ifdId, bool alloc, bool hasNext)
        : MakerNote(alloc),
          absShift_(true), shift_(0), start_(0), ifd_(ifdId, 0, alloc, hasNext)
    {
    }

    IfdMakerNote::IfdMakerNote(const IfdMakerNote& rhs)
        : MakerNote(rhs), absShift_(rhs.absShift_), shift_(rhs.shift_),
          start_(rhs.start_), header_(rhs.header_.size_), ifd_(rhs.ifd_)
    {
        memcpy(header_.pData_, rhs.header_.pData_, header_.size_);
    }

    int IfdMakerNote::read(const byte* buf,
                           long len,
                           long start,
                           ByteOrder byteOrder,
                           long shift)
    {
        // Remember the offset
        offset_ = start - shift;
        // Set byte order if none is set yet
        if (byteOrder_ == invalidByteOrder) byteOrder_ = byteOrder;
        // Read and check the header (and set offset adjustment)
        int rc = readHeader(buf + start, len - start, byteOrder);
        if (rc == 0) {
            rc = checkHeader();
        }
        // Adjust shift
        long newShift = absShift_ ? shift + shift_ : start + shift_;
        // Read the makernote IFD
        if (rc == 0) {
            rc = ifd_.read(buf, len, start + start_, byteOrder_, newShift);
        }
        if (rc == 0) {
            // IfdMakerNote currently does not support multiple IFDs
            if (ifd_.next() != 0) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Warning: Makernote IFD has a next pointer != 0 ("
                          << ifd_.next()
                          << "). Ignored.\n";
#endif
            }
        }
#ifdef DEBUG_MAKERNOTE
        hexdump(std::cerr, buf + start, len - start);
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
        offset = absShift_ ? offset + start_ - shift_ : start_ - shift_;

        long len = 0;
        len += copyHeader(buf);
        len += ifd_.copy(buf + len, byteOrder_, offset);

        return len;
    } // IfdMakerNote::copy

    int IfdMakerNote::readHeader(const byte* /*buf*/,
                                 long        /*len*/,
                                 ByteOrder   /*byteOrder*/)
    {
        // Default implementation does nothing, assuming there is no header
        return 0;
    }

    void IfdMakerNote::updateBase(byte* pNewBase)
    {
        if (absShift_) {
            ifd_.updateBase(pNewBase);
        }
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

    IfdMakerNote::AutoPtr IfdMakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    IfdMakerNote::AutoPtr IfdMakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    int MakerNoteFactory::Init::count = 0;

    MakerNoteFactory::Init::Init()
    {
        ++count;
    }

    MakerNoteFactory::Init::~Init()
    {
        if (--count == 0) {
            Exiv2::MakerNoteFactory::cleanup();
        }
    }

    MakerNoteFactory::Registry* MakerNoteFactory::pRegistry_ = 0;
    MakerNoteFactory::IfdIdRegistry* MakerNoteFactory::pIfdIdRegistry_ = 0;

    void MakerNoteFactory::cleanup()
    {
        if (pRegistry_ != 0) {
            Registry::iterator e = pRegistry_->end();
            for (Registry::iterator i = pRegistry_->begin(); i != e; ++i) {
                delete i->second;
            }
            delete pRegistry_;
        }

        if (pIfdIdRegistry_ != 0) {
            IfdIdRegistry::iterator e = pIfdIdRegistry_->end();
            for (IfdIdRegistry::iterator i = pIfdIdRegistry_->begin(); i != e; ++i) {
                delete i->second;
            }
            delete pIfdIdRegistry_;
        }
    }

    void MakerNoteFactory::init()
    {
        if (0 == pRegistry_) {
            pRegistry_ = new Registry;
        }
        if (0 == pIfdIdRegistry_) {
            pIfdIdRegistry_ = new IfdIdRegistry;
        }
    } // MakerNoteFactory::init

    void MakerNoteFactory::registerMakerNote(IfdId ifdId,
                                             MakerNote::AutoPtr makerNote)
    {
        init();
        MakerNote* pMakerNote = makerNote.release();
        assert(pMakerNote);
        IfdIdRegistry::iterator pos = pIfdIdRegistry_->find(ifdId);
        if (pos != pIfdIdRegistry_->end()) {
            delete pos->second;
            pos->second = 0;
        }
        (*pIfdIdRegistry_)[ifdId] = pMakerNote;
    } // MakerNoteFactory::registerMakerNote

    MakerNote::AutoPtr MakerNoteFactory::create(IfdId ifdId, bool alloc)
    {
        assert(pIfdIdRegistry_ != 0);
        IfdIdRegistry::const_iterator i = pIfdIdRegistry_->find(ifdId);
        if (i == pIfdIdRegistry_->end()) return MakerNote::AutoPtr(0);
        assert(i->second);
        return i->second->create(alloc);
    } // MakerNoteFactory::create

    void MakerNoteFactory::registerMakerNote(const std::string& make,
                                             const std::string& model,
                                             CreateFct createMakerNote)
    {
#ifdef DEBUG_REGISTRY
        std::cerr << "Registering MakerNote create function for \""
                  << make << "\" and \"" << model << "\".\n";
#endif
        init();
        // Todo: use case insensitive make and model comparisons

        // Find or create a registry entry for make
        ModelRegistry* pModelRegistry = 0;
        assert(pRegistry_ != 0);
        Registry::const_iterator end1 = pRegistry_->end();
        Registry::const_iterator pos1;
        for (pos1 = pRegistry_->begin(); pos1 != end1; ++pos1) {
            if (pos1->first == make) break;
        }
        if (pos1 != end1) {
            pModelRegistry = pos1->second;
        }
        else {
            pModelRegistry = new ModelRegistry;
            pRegistry_->push_back(std::make_pair(make, pModelRegistry));
        }
        // Find or create a registry entry for model
        ModelRegistry::iterator end2 = pModelRegistry->end();
        ModelRegistry::iterator pos2;
        for (pos2 = pModelRegistry->begin(); pos2 != end2; ++pos2) {
            if (pos2->first == model) break;
        }
        if (pos2 != end2) {
            pos2->second = createMakerNote;
        }
        else {
            pModelRegistry->push_back(std::make_pair(model, createMakerNote));
        }
    } // MakerNoteFactory::registerMakerNote

    MakerNote::AutoPtr MakerNoteFactory::create(const std::string& make,
                                                const std::string& model,
                                                bool alloc,
                                                const byte* buf,
                                                long len,
                                                ByteOrder byteOrder,
                                                long offset)
    {
#ifdef DEBUG_REGISTRY
        std::cerr << "Entering MakerNoteFactory::create(\""
                  << make << "\", \"" << model << "\", "
                  << (alloc == true ? "true" : "false") << ")\n";
#endif
        // loop through each make of the registry to find the best matching make
        int score = 0;
        ModelRegistry* pModelRegistry = 0;
#ifdef DEBUG_REGISTRY
        std::string makeMatch;
        std::cerr << "Searching make registry...\n";
#endif
        assert(pRegistry_ != 0);
        Registry::const_iterator end1 = pRegistry_->end();
        Registry::const_iterator pos1;
        for (pos1 = pRegistry_->begin(); pos1 != end1; ++pos1) {
            int rc = match(pos1->first, make);
            if (rc > score) {
                score = rc;
#ifdef DEBUG_REGISTRY
                makeMatch = pos1->first;
#endif
                pModelRegistry = pos1->second;
            }
        }
        if (pModelRegistry == 0) return MakerNote::AutoPtr(0);
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
        ModelRegistry::const_iterator end2 = pModelRegistry->end();
        ModelRegistry::const_iterator pos2;
        for (pos2 = pModelRegistry->begin(); pos2 != end2; ++pos2) {
            int rc = match(pos2->first, model);
            if (rc > score) {
                score = rc;
#ifdef DEBUG_REGISTRY
                modelMatch = pos2->first;
#endif
                createMakerNote = pos2->second;
            }
        }
        if (createMakerNote == 0) return MakerNote::AutoPtr(0);
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
                  << (int)regEntry.size() << ") with key \"" << key << "\" ("
                  << (int)key.size() << "): ";
#endif
        // Todo: make the comparisons case insensitive

        // Handle exact match (this is only necessary because of the different
        // return value - the following algorithm also finds exact matches)
        if (regEntry == key) {
#ifdef DEBUG_REGISTRY
            std::cerr << "Exact match (score: " << (int)key.size() + 2 << ")\n";
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
