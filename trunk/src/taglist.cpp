// ***************************************************************** -*- C++ -*-
/*
  Abstract:  Print a simple comma separated list of tags defined in Exiv2

  File:      taglist.cpp
  Version:   $Name:  $ $Revision: 1.3 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   07-Jan-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.3 $ $RCSfile: taglist.cpp,v $")

#include "tags.hpp"
#include "error.hpp"

#include <iostream>

using namespace Exif;

int main()
try {
    ExifTags::taglist(std::cout);
    return 0;
}
catch (Error& e) {
    std::cout << "Caught Exif exception '" << e << "'\n";
    return 1;
}
