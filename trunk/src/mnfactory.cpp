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
  File:      mnfactory.cpp
  Version:   $Name:  $ $Revision: 1.1 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Feb-04, ahu: created
             07-Mar-04, ahu: isolated as a separate component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.1 $ $RCSfile: mnfactory.cpp,v $")

// *****************************************************************************
// included header files
#include "mnfactory.hpp"
#include "makernote.hpp"
#include "canonmn.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// class member definitions
namespace Exif {

    MakerNoteFactory* MakerNoteFactory::instance_ = 0;

    MakerNoteFactory::MakerNoteFactory()
    {
        // Register a prototype of each known MakerNote
        registerMakerNote("Canon", "*", new CanonMakerNote);
    } // MakerNoteFactory c'tor

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

}                                       // namespace Exif
