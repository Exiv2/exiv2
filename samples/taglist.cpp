// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
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

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <string>
#include <sstream>

using namespace Exiv2;

int main(int argc, char* argv[])
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    int rc = EXIT_SUCCESS;
    std::ostringstream out;
    try {
        bool bHelp     = false;

        switch (argc) {
            case 2: {
                std::string item(argv[1]);

                if ( item == "--help" ) {
                    bHelp = true;
                    break;
                }

                if (item == "Groups") {
                    const GroupInfo* groupList = ExifTags::groupList();
                    if (groupList) {
                        while (groupList->tagList_) {
                            std::cout << groupList->groupName_ << std::endl;
                            groupList++;
                        }
                    }
                    break;
                }

                if (item == "all" || item == "ALL" ) {
                    const GroupInfo* groupList = ExifTags::groupList();
                    if (groupList) {
                        std::string line;
                        while (groupList->tagList_) {
                            std::ostringstream tags;
                            ExifTags::taglist(tags,groupList->groupName_);
                            std::istringstream input(tags.str()) ;
                            while (std::getline(input, line)) {
                                std::cout << groupList->groupName_ << "."
                                          << (item == "all" ? line.substr(0,line.find(",")) : line)
                                          << std::endl;
                            }
                            groupList++;
                        }
                    }
                    break;
                }


                if (item == "Exif") {
                    ExifTags::taglist(std::cout);
                    break;
                }

                if (item == "Iptc") {
                    IptcDataSets::dataSetList(std::cout);
                    break;
                }

                if (ExifTags::isExifGroup(item) || ExifTags::isMakerGroup(item)) {
                    ExifTags::taglist(std::cout, item);
                    break;
                }

                try {
                    XmpProperties::printProperties(std::cout, item);
                    break;
                } catch (const AnyError&) {
                    rc = 2;
                }
                std::cerr << "Unexpected argument "  << argv[1] << std::endl;

                break;
            }
            case 1:
                ExifTags::taglist(std::cout);
                break;

            case 3: {
                std::string item(argv[1]);
                std::string name(argv[2]);
                rc = EXIT_FAILURE;  // assume unhappy ending!

                if (item == "--group") {
                    if ( ExifTags::isExifGroup(name) ) {
                        ExifTags::taglist(std::cout,name);
                        rc = EXIT_SUCCESS;  // result is good
                    } else {
                        std::cerr << "warning:"
                                  << name
                                  << " is not a valid Exif group name "
                                  << std::endl
                                  ;
                        const GroupInfo* groupList = ExifTags::groupList();
                        if (groupList) {
                            while (rc && groupList->tagList_) {
                                if (name == groupList->groupName_) {
                                    const Exiv2::TagInfo* tagInfo = groupList->tagList_();
                                    while (tagInfo->tag_ != 0xFFFF) {
                                        std::cout << tagInfo->name_ << std::endl;
                                        tagInfo++;
                                    }
                                    rc = EXIT_SUCCESS;  // result is good
                                }
                                groupList++;
                            }
                        }
                    }
                }
            } break;

            default:
                rc = EXIT_FAILURE;
            break;
        }

        if (rc || bHelp) {
            std::cout << "Usage: taglist [--help]"                     << std::endl
                << "           [--group name|"                         << std::endl
                << "            Groups|Exif|Canon|CanonCs|CanonSi|CanonCf|Fujifilm|Minolta|Nikon1|Nikon2|Nikon3|Olympus|" << std::endl
                << "            Panasonic|Pentax|Sigma|Sony|Iptc|"                                                        << std::endl
                << "            dc|xmp|xmpRights|xmpMM|xmpBJ|xmpTPg|xmpDM|pdf|photoshop|crs|tiff|exif|aux|iptc|all|ALL"  << std::endl
                << "           ]"                                      << std::endl
                << "Print Exif tags, MakerNote tags, or Iptc datasets" << std::endl
                ;
        }
    } catch (AnyError& e) {
        std::cout << "Caught Exiv2 exception '" << e << "'\n";
        rc = EXIT_FAILURE ;
    }
    return rc;
}
