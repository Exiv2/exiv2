// ***************************************************************** -*- C++ -*-
/*
  Abstract:  Print a simple comma separated list of tags defined in Exiv2

  File:      taglist.cpp
  Version:   $Name:  $ $Revision: 1.9 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   07-Jan-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.9 $ $RCSfile: taglist.cpp,v $");

#include "makernote.hpp"
#include "nikonmn.hpp"
#include "sigmamn.hpp"
#include "fujimn.hpp"
#include "canonmn.hpp"
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
        MakerNote* pMakerNote = 0;
        std::string item(argv[1]);
        if (item == "Iptc") {
            IptcDataSets::dataSetList(std::cout);
            break;
        }

        if (item == "Canon") {
            pMakerNote = new CanonMakerNote;
        }
        else if (item == "Fujifilm") {
            pMakerNote = new FujiMakerNote;
        }
        else if (item == "Sigma") {
            pMakerNote = new SigmaMakerNote;
        }
        else if (item == "Nikon1") {
            pMakerNote = new Nikon1MakerNote;
        }
        else if (item == "Nikon2") {
            pMakerNote = new Nikon2MakerNote;
        }
        else if (item == "Nikon3") {
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
        std::cout << "Usage: " << argv[0] 
                  << " [Canon|Fujifilm|Nikon1|Nikon2|Nikon3|Sigma|Iptc]\n"
                  << "Print Exif tags, MakerNote tags, or Iptc datasets\n";
    }
    return rc;
}
catch (Error& e) {
    std::cout << "Caught Exif exception '" << e << "'\n";
    return 1;
}
