// ***************************************************************** -*- C++ -*-
/*
  Abstract:  Print a simple comma separated list of tags defined in Exiv2

  File:      taglist.cpp
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   07-Jan-04, ahu: created
 */
// *****************************************************************************

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <string>
#include <sstream>

using namespace Exiv2;

int main(int argc, char* argv[])
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    int rc = 0;
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
                rc = 1;  // assume unhappy ending!

                if (item == "--group") {
                    if ( ExifTags::isExifGroup(name) ) {
                        ExifTags::taglist(std::cout,name);
                        rc = 0;  // result is good
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
                                    rc = 0;  // result is good
                                }
                                groupList++;
                            }
                        }
                    }
                }
            } break;

            default:
                rc = 1;
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
        rc = 1 ;
    }
    return rc;
}
