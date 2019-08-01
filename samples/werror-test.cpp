// ***************************************************************** -*- C++ -*-
// werror-test.cpp
// Simple tests for the wide-string error class WError

#include <exiv2/exiv2.hpp>

#include <iostream>

int main()
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    try {
        throw Exiv2::Error(Exiv2::kerGeneralError, "ARG1", "ARG2", "ARG3");
    }
    catch (const Exiv2::Error& e) {
        std::cout << "Caught Error '" << e.what() << "'\n";
    }

#ifdef EXV_UNICODE_PATH
    try {
        throw Exiv2::WError(Exiv2::kerGeneralError, L"WARG1", L"WARG2", L"WARG3");
    }
    catch (const Exiv2::WError& e) {
        std::wstring wmsg = e.wwhat();
        std::string msg(wmsg.begin(), wmsg.end());
        std::cout << "Caught WError '" << msg << "'\n";
    }
#endif

    return 0;
}
