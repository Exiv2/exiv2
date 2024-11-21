// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/exiv2.hpp>

#include <fstream>
#include <iostream>

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

int main(int argc, char* const argv[]) {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " file\n";
    return EXIT_FAILURE;
  }
  std::ifstream file(argv[1]);
  if (!file) {
    std::cerr << *argv[1] << ": Failed to open file for reading\n";
    return EXIT_FAILURE;
  }
  std::string line;
  while (std::getline(file, line)) {
    std::string path, dir, base;
    std::istringstream is(line);
    is >> path >> dir >> base;
    auto p = fs::path(path);
    auto d = p.parent_path();
    auto b = p.filename();

    if (d != dir || b != base) {
      std::cout << path << "\t'" << d << "'\t '" << b << "'\t ==> Testcase failed\n";
    }
  }

  return EXIT_SUCCESS;
}
