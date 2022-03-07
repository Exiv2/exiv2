// SPDX-License-Identifier: GPL-2.0-or-later
// Simple tests for the wide-string error class WError

#include <exiv2/exiv2.hpp>
#include <iostream>

int main()
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    try {
        throw Exiv2::Error(Exiv2::kerGeneralError, "ARG1", "ARG2", "ARG3");
    }
    catch (const Exiv2::Error& e) {
        std::cout << "Caught Error '" << e.what() << "'\n";
    }

    return 0;
}
