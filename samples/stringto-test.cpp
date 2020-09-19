// ***************************************************************** -*- C++ -*-
// stringto-test.cpp
// Test conversions from string to long, float and Rational types.

#include <exiv2/exiv2.hpp>
#include <iomanip>
#include <iostream>

const char* testcases[] = {
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
    "text"
};

int main()
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    std::cout << std::setfill(' ');

    std::cout << std::setw(12) << std::left << "string";
    std::cout << std::setw(12) << std::left << "long";
    std::cout << std::setw(12) << std::left << "float";
    std::cout << std::setw(12) << std::left << "Rational";

    std::cout << std::endl;

    for (auto &testcase : testcases)
        try {
            std::string s(testcase);
            std::cout << std::setw(12) << std::left << s;
            bool ok;

            long l = Exiv2::parseLong(s, ok);
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

            std::cout << std::endl;
        } catch (Exiv2::AnyError &e) {
            std::cout << "Caught Exiv2 exception '" << e << "'\n";
            return -1;
        }

    return 0;
}
