// SPDX-License-Identifier: GPL-2.0-or-later
// Simple mmap tests

#include <exiv2/exiv2.hpp>

#include <iostream>

using namespace Exiv2;

int main(int argc, char* const argv[]) {
  try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    if (argc != 2) {
      std::cout << "Usage: " << argv[0] << " file\n";
      return EXIT_FAILURE;
    }
    const char* path = argv[1];

    FileIo file(path);
    // Open the file in read mode
    if (file.open("rb") != 0) {
      throw Error(ErrorCode::kerFileOpenFailed, path, "rb", strError());
    }
    // Map it to memory
    const Exiv2::byte* pData = file.mmap();
    std::vector<byte> buf(file.size());

    // Read from the memory mapped region
    std::copy_n(pData, buf.size(), buf.begin());

    // Reopen file in write mode and write to it
    file.write(buf.data(), buf.size());

    // Read from the mapped region again
    std::copy_n(pData, buf.size(), buf.begin());
    file.close();

    return EXIT_SUCCESS;
  } catch (const Error& e) {
    std::cout << e << "\n";
    return EXIT_FAILURE;
  }
}
