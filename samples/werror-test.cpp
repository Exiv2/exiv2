// ***************************************************************** -*- C++ -*-
// werror-test.cpp, $Rev$
// Simple tests for the wide-string error class WError

#include <iostream>
#include <exiv2/error.hpp>

int main()
{
    try {
        throw Exiv2::Error(-1, "ARG1", "ARG2", "ARG3");
    }
    catch (const Exiv2::Error& e) {
        std::cout << "Caught Error '" << e.what() << "'\n";
    }

    try {
        throw Exiv2::WError(-1, L"WARG1", L"WARG2", L"WARG3");
    }
    catch (const Exiv2::WError& e) {
        std::wcout << "Caught WError '" << e.wwhat() << "'\n";
    }

    return 0;
}
