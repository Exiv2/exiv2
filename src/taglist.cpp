// ***************************************************************** -*- C++ -*-
/*
  Abstract:  Print a simple comma separated list of tags defined in Exiv2

  File:      taglist.cpp
  Version:   $Name:  $ $Revision: 1.6 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   07-Jan-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.6 $ $RCSfile: taglist.cpp,v $")

#include "makernote.hpp"
#include "nikonmn.hpp"
#include "sigmamn.hpp"
#include "fujimn.hpp"
#include "canonmn.hpp"
#include "tags.hpp"
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
        MakerNote* pMakerNote = 0;
        std::string section(argv[1]);
        if (section == "Canon") {
            pMakerNote = new CanonMakerNote;
        }
        if (section == "Fuji") {
            pMakerNote = new FujiMakerNote;
        }
        if (section == "Sigma") {
            pMakerNote = new SigmaMakerNote;
        }
        if (section == "Nikon1") {
            pMakerNote = new Nikon1MakerNote;
        }
        if (section == "Nikon2") {
            pMakerNote = new Nikon2MakerNote;
        }
        if (section == "Nikon3") {
            pMakerNote = new Nikon3MakerNote;
        }

        if (pMakerNote) {
            pMakerNote->taglist(std::cout);
            delete pMakerNote;
            pMakerNote = 0;
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
        std::cout << "Usage: " << argv[0] << " [SectionName]\n"
                  << "Print Exif tags or MakerNote tags\n";
    }
    return rc;
}
catch (Error& e) {
    std::cout << "Caught Exif exception '" << e << "'\n";
    return 1;
}
