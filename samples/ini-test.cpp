// SPDX-License-Identifier: GPL-2.0-or-later

/*
670 rmills@rmillsmbp:~/gnu/exiv2/trunk/samples $ gcc ../src/ini.cpp  ini-test.cpp -lstdc++ -o ini-test
671 rmills@rmillsmbp:~/gnu/exiv2/trunk/samples $ ./ini-test
Config loaded from : 'initest.ini' version=6, name=Bob Smith, email=bob@smith.com, pi=3.14159, active=1
169=Sigma 35mm F1.4 DG HSM ART, 170=UNDEFINED
672 rmills@rmillsmbp:~/gnu/exiv2/trunk/samples $
*/

// Example that shows simple usage of the INIReader class
#include <INIReader.h>
#include <exiv2/exiv2.hpp>
#include <iostream>

int main() {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  const char* ini = "ini-test.ini";
  INIReader reader(ini);

  if (reader.ParseError() < 0) {
    std::cerr << "Can't load '" << ini << "'" << '\n';
    return EXIT_FAILURE;
  }
  std::cout << "Config loaded from : '" << ini << "' "
            << "version=" << reader.GetInteger("protocol", "version", -1)
            << ", name=" << reader.Get("user", "name", "UNKNOWN")
            << ", email=" << reader.Get("user", "email", "UNKNOWN") << ", pi=" << reader.GetReal("user", "pi", -1)
            << ", active=" << reader.GetBoolean("user", "active", true) << '\n';

  std::cout << "169=" << reader.Get("canon", "169", "UNDEFINED") << ", 170=" << reader.Get("canon", "170", "UNDEFINED")
            << '\n';

  return EXIT_SUCCESS;
}
