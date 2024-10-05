// SPDX-License-Identifier: GPL-2.0-or-later
// Test access to preview images

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

  auto image = Exiv2::ImageFactory::open(filename);
  image->readMetadata();

  Exiv2::PreviewManager loader(*image);
  for (const auto& pos : loader.getPreviewProperties()) {
    std::cout << pos.mimeType_ << " preview, type " << pos.id_ << ", " << pos.size_ << " bytes, " << pos.width_ << 'x'
              << pos.height_ << " pixels"
              << "\n";

    Exiv2::PreviewImage preview = loader.getPreviewImage(pos);
    auto s = preview.writeFile(filename + "_" + std::to_string(pos.width_) + "x" + std::to_string(pos.height_));
    if (s == 0)
      return EXIT_FAILURE;
  }

  // Cleanup
  Exiv2::XmpParser::terminate();

  return EXIT_SUCCESS;
} catch (Exiv2::Error& e) {
  std::cout << "Caught Exiv2 exception '" << e << "'\n";
  return EXIT_FAILURE;
}
