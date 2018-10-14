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

using namespace Exiv2;

int main(int argc, char* argv[])
{
    try {
        int rc = 0;

        switch (argc) {
            case 2: {
                std::string item(argv[1]);

                if (item == "Groups") {
                   /*
                    https://cgit.kde.org/digikam.git/tree/core/libs/dmetadata/metaengine_exif.cpp#n1077
                    const Exiv2::GroupInfo* gi = Exiv2::ExifTags::groupList();

                    while (gi->tagList_ != 0)
                    {
                       // NOTE: See BUG #375809 : MPF tags = exception Exiv2 0.26

                       if (QLatin1String(gi->ifdName_) != QLatin1String("Makernote"))
                       {
                           Exiv2::TagListFct tl     = gi->tagList_;
                           const Exiv2::TagInfo* ti = tl();

                           while (ti->tag_ != 0xFFFF)
                           {
                               tags << ti;
                               ++ti;
                           }
                       }

                       ++gi;
                    }
                   */
                    const GroupInfo* groupList = ExifTags::groupList();
                    if (groupList) {
                        while (groupList->tagList_) {
                            std::cout << groupList->groupName_ << std::endl;
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

        if (rc) {
            std::cout << "Usage: " << argv[0]
                      << " [--group "
                         "name|Groups|Exif|Canon|CanonCs|CanonSi|CanonCf|Fujifilm|Minolta|Nikon1|Nikon2|Nikon3|Olympus|"
                         "Panasonic|Pentax|Sigma|Sony|Iptc"
                      << "|dc|xmp|xmpRights|xmpMM|xmpBJ|xmpTPg|xmpDM|pdf|photoshop|crs|tiff|exif|aux|iptc]\n"
                      << "Print Exif tags, MakerNote tags, or Iptc datasets\n";
        }
        return rc;
    } catch (AnyError& e) {
        std::cout << "Caught Exiv2 exception '" << e << "'\n";
        return 1;
    }
}
