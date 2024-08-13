// SPDX-License-Identifier: GPL-2.0-or-later
// Test conversions from string to long, float and Rational types.

#include <exiv2/exiv2.hpp>

#include <iostream>

static constexpr const char* testcases[] = {
    // bool
    "True",
    "False",
    "t",
    "f",
    // long
    "-1",
    "0",
    "1",
    // float
    "0.0",
    "0.1",
    "0.01",
    "0.001",
    "-1.49999",
    "-1.5",
    "1.49999",
    "1.5",
    // Rational
    "0/1",
    "1/1",
    "1/3",
    "-1/3",
    "4/3",
    "-4/3",
    "0/0",
    // nok
    "text",
};

int main() {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  std::cout << std::setfill(' ');

  std::cout << std::setw(12) << std::left << "string";
  std::cout << std::setw(12) << std::left << "long";
  std::cout << std::setw(12) << std::left << "float";
  std::cout << std::setw(12) << std::left << "Rational";

  std::cout << '\n';

  for (auto&& testcase : testcases) {
    try {
      std::string s(testcase);
      std::cout << std::setw(12) << std::left << s;
      bool ok = false;

      const auto l = Exiv2::parseInt64(s, ok);
      std::cout << std::setw(12) << std::left;
      if (ok)
        std::cout << l;
      else
        std::cout << "nok";

      float f = Exiv2::parseFloat(s, ok);
      std::cout << std::setw(12) << std::left;
      if (ok)
        std::cout << f;
      else
        std::cout << "nok";

      Exiv2::Rational r = Exiv2::parseRational(s, ok);
      if (ok)
        std::cout << r.first << "/" << r.second;
      else
        std::cout << "nok";

      std::cout << '\n';
    } catch (Exiv2::Error& e) {
      std::cout << "Caught Exiv2 exception '" << e << "'\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
