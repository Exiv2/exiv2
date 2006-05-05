// ***************************************************************** -*- C++ -*-
/*
  Abstract:  Print a simple comma separated list of tags defined in Exiv2

  File:      taglist.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   07-Jan-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

#include "makernote.hpp"
#include "tags.hpp"
#include "datasets.hpp"
#include "error.hpp"

#include <string>
#include <iostream>

using namespace Exiv2;

int main(int argc, char* argv[])
try {
    int rc = 0;

    switch (argc) {
    case 2:
    {
        std::string item(argv[1]);

        if (item == "Exif") {
            ExifTags::taglist(std::cout);
            break;
        }

        if (item == "Iptc") {
            IptcDataSets::dataSetList(std::cout);
            break;
        }

        IfdId ifdId = ExifTags::ifdIdByIfdItem(item);
        if (ExifTags::isMakerIfd(ifdId)) {
            ExifTags::makerTaglist(std::cout, ifdId);
        }
        else {
            rc = 2;
        }
        break;
    }
    case 1:
        ExifTags::taglist(std::cout);
        break;
    default:
        rc = 1;
        break;
    }
    if (rc) {
        std::cout << "Usage: " << argv[0]
                  << " [Exif|Canon|CanonCs1|CanonCs2|CanonCf|Fujifilm|Nikon1|Nikon2|Nikon3|Olympus|Sigma|Sony|Iptc]\n"
                  << "Print Exif tags, MakerNote tags, or Iptc datasets\n";
    }
    return rc;
}
catch (AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return 1;
}
