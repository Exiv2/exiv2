/*
670 rmills@rmillsmbp:~/gnu/exiv2/trunk/samples $ gcc ../src/ini.cpp  ini-test.cpp -lstdc++ -o ini-test
671 rmills@rmillsmbp:~/gnu/exiv2/trunk/samples $ ./ini-test
Config loaded from : 'initest.ini' version=6, name=Bob Smith, email=bob@smith.com, pi=3.14159, active=1
169=Sigma 35mm F1.4 DG HSM ART, 170=UNDEFINED
672 rmills@rmillsmbp:~/gnu/exiv2/trunk/samples $
*/


// Example that shows simple usage of the INIReader class

#include <iostream>

// #include <exiv2/exiv2.h>
#include "config.h"
#include "ini.hpp"

int main()
{
    int              result = 0 ;
    const char*      ini    = "ini-test.ini";
    Exiv2::INIReader reader(ini);

    if (reader.ParseError() < 0) {
        std::cerr << "Can't load '" << ini << "'" << std::endl ;
        result = 1;
    } else {
        std::cout << "Config loaded from : '" << ini << "' "
                  << "version="  << reader.GetInteger("protocol", "version", -1)
                  << ", name="   << reader.Get("user", "name", "UNKNOWN")
                  << ", email="  << reader.Get("user", "email", "UNKNOWN")
                  << ", pi="     << reader.GetReal("user", "pi", -1)
                  << ", active=" << reader.GetBoolean("user", "active", true)
                  << std::endl ;

        std::cout << "169="      << reader.Get("canon",   "169","UNDEFINED")
                  << ", 170="    << reader.Get("canon",   "170","UNDEFINED")
                  << std::endl ;
    }

    return result ;
}
