// ***************************************************************** -*- C++ -*-
/*
  Abstract:  Print a simple comma separated list of tags defined in Exiv2

  File:      taglist.cpp
  Version:   $Name:  $ $Revision: 1.5 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   07-Jan-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.5 $ $RCSfile: taglist.cpp,v $")

#include "makernote.hpp"
#include "tags.hpp"
#include "error.hpp"

#include <string>
#include <iostream>

using namespace Exiv2;

int main(int argc, char* argv[])
try {
    int rc = 0;

    std::string make("any");
    std::string model("any");
    switch (argc) {
    case 3:
        model = argv[2];
        // fallthrough
    case 2:
    {
        make = argv[1];
        MakerNoteFactory& mnf = MakerNoteFactory::instance();
        MakerNote* pMakerNote = mnf.create(make, model);
        if (pMakerNote) {
            pMakerNote->taglist(std::cout);
        }
        else {
            rc = -1;
        }
        break;
    }
    case 1:
        ExifTags::taglist(std::cout);
        break;
    default:
        std::cout << "Usage: " << argv[0] << " [Make [Model]]\n";
        rc = 1;
        break;
    }

    return rc;
}
catch (Error& e) {
    std::cout << "Caught Exif exception '" << e << "'\n";
    return 1;
}
