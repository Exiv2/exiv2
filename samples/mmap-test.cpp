// SPDX-License-Identifier: GPL-2.0-or-later
// Simple mmap tests

#include <exiv2/exiv2.hpp>

#include <iostream>

using namespace Exiv2;

int main(int argc, char* const argv[])
try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    const char* path = argv[1];

    FileIo file(path);
    // Open the file in read mode
    if (file.open("rb") != 0) {
        throw Error(ErrorCode::kerFileOpenFailed, path, "rb", strError());
    }
    // Map it to memory
    const Exiv2::byte* pData = file.mmap();
    DataBuf buf(file.size());
    // Read from the memory mapped region
    buf.copyBytes(0, pData, buf.size());
    // Reopen file in write mode and write to it
    file.write(buf.c_data(), buf.size());
    // Read from the mapped region again
    buf.copyBytes(0, pData, buf.size());
    file.close();

    return 0;
}
catch (const AnyError& e) {
    std::cout << e << "\n";
}
