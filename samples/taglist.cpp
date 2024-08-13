// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/exiv2.hpp>

#include <iostream>

using namespace Exiv2;

int main(int argc, char* argv[]) {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  int rc = EXIT_SUCCESS;
  std::ostringstream out;
  try {
    bool bHelp = false;

    switch (argc) {
      case 2: {
        std::string item(argv[1]);

        if (item == "--help") {
          bHelp = true;
          break;
        }

        if (item == "Groups") {
          const GroupInfo* groupList = ExifTags::groupList();
          if (groupList) {
            while (groupList->tagList_) {
              std::cout << groupList->groupName_ << '\n';
              groupList++;
            }
          }
          break;
        }

        if (item == "all" || item == "ALL") {
          const GroupInfo* groupList = ExifTags::groupList();
          if (groupList) {
            std::string line;
            while (groupList->tagList_) {
              std::ostringstream tags;
              ExifTags::taglist(tags, groupList->groupName_);
              std::istringstream input(tags.str());
              while (std::getline(input, line)) {
                std::cout << groupList->groupName_ << "." << (item == "all" ? line.substr(0, line.find(',')) : line)
                          << '\n';
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
        } catch (const Error&) {
          rc = 2;
        }
        std::cerr << "Unexpected argument " << argv[1] << '\n';

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
          if (ExifTags::isExifGroup(name)) {
            ExifTags::taglist(std::cout, name);
            rc = EXIT_SUCCESS;  // result is good
          } else {
            std::cerr << "warning:" << name << " is not a valid Exif group name " << '\n';
            const GroupInfo* groupList = ExifTags::groupList();
            if (groupList) {
              while (rc && groupList->tagList_) {
                if (name == groupList->groupName_) {
                  const Exiv2::TagInfo* tagInfo = groupList->tagList_();
                  while (tagInfo->tag_ != 0xFFFF) {
                    std::cout << tagInfo->name_ << '\n';
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
      std::cout << "Usage: taglist [--help]" << '\n'
                << "           [--group name|" << '\n'
                << "            "
                   "Groups|Exif|Canon|CanonCs|CanonSi|CanonCf|CanonHdr|Fujifilm|Minolta|Nikon1|Nikon2|Nikon3|Olympus|"
                << '\n'
                << "            Panasonic|Pentax|Sigma|Sony|Iptc|" << '\n'
                << "            dc|xmp|xmpRights|xmpMM|xmpBJ|xmpTPg|xmpDM|pdf|photoshop|crs|tiff|exif|aux|iptc|all|ALL"
                << '\n'
                << "           ]" << '\n'
                << "Print Exif tags, MakerNote tags, or Iptc datasets" << '\n';
    }
  } catch (Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    rc = EXIT_FAILURE;
  }
  return rc;
}
