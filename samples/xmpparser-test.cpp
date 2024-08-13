// SPDX-License-Identifier: GPL-2.0-or-later
// Read an XMP packet from a file, parse and re-serialize it.

#include <exiv2/exiv2.hpp>
#include <iostream>

int main(int argc, char* const argv[]) try {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " file\n";
    return EXIT_FAILURE;
  }
  std::string filename(argv[1]);
  Exiv2::DataBuf buf = Exiv2::readFile(filename);
  std::string xmpPacket;
  xmpPacket.assign(buf.c_str(), buf.size());
  std::cerr << "-----> Decoding XMP data read from " << filename << " <-----\n";
  Exiv2::XmpData xmpData;
  if (0 != Exiv2::XmpParser::decode(xmpData, xmpPacket)) {
    std::string error(argv[1]);
    error += ": Failed to parse file contents (XMP packet)";
    throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, error);
  }
  if (xmpData.empty()) {
    std::string error(argv[1]);
    error += ": No XMP properties found in the XMP packet";
    throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, error);
  }
  for (auto&& md : xmpData) {
    std::cout << std::setfill(' ') << std::left << std::setw(44) << md.key() << " " << std::setw(9) << std::setfill(' ')
              << std::left << md.typeName() << " " << std::dec << std::setw(3) << std::setfill(' ') << std::right
              << md.count() << "  " << std::dec << md.toString() << '\n';
  }
  filename += "-new";
  std::cerr << "-----> Encoding XMP data to write to " << filename << " <-----\n";
  if (0 != Exiv2::XmpParser::encode(xmpPacket, xmpData)) {
    std::string error(argv[1]);
    error += ": Failed to encode the XMP data";
    throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, error);
  }
  Exiv2::FileIo file(filename);
  if (file.open("wb") != 0) {
    throw Exiv2::Error(Exiv2::ErrorCode::kerFileOpenFailed, filename, "wb", Exiv2::strError());
  }
  if (file.write(reinterpret_cast<const Exiv2::byte*>(xmpPacket.data()), xmpPacket.size()) == 0) {
    throw Exiv2::Error(Exiv2::ErrorCode::kerCallFailed, filename, Exiv2::strError(), "FileIo::write");
  }
  Exiv2::XmpParser::terminate();
  return EXIT_SUCCESS;
} catch (Exiv2::Error& e) {
  std::cout << "Caught Exiv2 exception '" << e << "'\n";
  return EXIT_FAILURE;
}
