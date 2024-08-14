// SPDX-License-Identifier: GPL-2.0-or-later
// First and very simple TIFF write test.

#include <enforce.hpp>
#include <exiv2/exiv2.hpp>

#include <iomanip>
#include <iostream>
#include <string>

using namespace Exiv2;

void print(const ExifData& exifData);

void mini1(const char* path);
void mini9(const char* path);

int main(int argc, char* const argv[]) {
  try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    if (argc != 2) {
      std::cout << "Usage: " << argv[0] << " file\n";
      return EXIT_FAILURE;
    }

    const char* path = argv[1];
    mini1(path);
    mini9(path);

    return EXIT_SUCCESS;
  } catch (const Error& e) {
    std::cout << e << "\n";
    return EXIT_FAILURE;
  }
}

void mini1(const char* path) {
  ExifData exifData;
  Blob blob;
  WriteMethod wm;

  // Write nothing to a new structure, without a previous binary image
  wm = ExifParser::encode(blob, nullptr, 0, bigEndian, exifData);
  Internal::enforce(wm == wmIntrusive, Exiv2::ErrorCode::kerErrorMessage, "encode returned an unexpected value");
  std::cout << "Test 1: Writing empty Exif data without original binary data: ok.\n";

  // Write nothing, this time with a previous binary image
  DataBuf buf = readFile(path);
  wm = ExifParser::encode(blob, buf.c_data(), buf.size(), bigEndian, exifData);
  Internal::enforce(wm == wmIntrusive, Exiv2::ErrorCode::kerErrorMessage, "encode returned an unexpected value");
  std::cout << "Test 2: Writing empty Exif data with original binary data: ok.\n";

  // Write something to a new structure, without a previous binary image
  exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
  wm = ExifParser::encode(blob, nullptr, 0, bigEndian, exifData);
  Internal::enforce(wm == wmIntrusive, Exiv2::ErrorCode::kerErrorMessage, "encode returned an unexpected value");
  std::cout << "Test 3: Wrote non-empty Exif data without original binary data:\n";
  exifData.clear();
  ByteOrder bo = ExifParser::decode(exifData, blob.data(), blob.size());
  Internal::enforce(bo == bigEndian, Exiv2::ErrorCode::kerErrorMessage, "decode returned an unexpected value");
  print(exifData);
}

void mini9(const char* path) {
  TiffImage tiffImage(std::make_unique<FileIo>(path), false);
  tiffImage.readMetadata();

  std::cout << "MIME type:  " << tiffImage.mimeType() << "\n";
  std::cout << "Image size: " << tiffImage.pixelWidth() << " x " << tiffImage.pixelHeight() << "\n";

  ExifData& exifData = tiffImage.exifData();
  std::cout << "Before\n";
  print(exifData);
  std::cout << "======\n";

  exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";

  std::cout << "After\n";
  print(exifData);
  tiffImage.writeMetadata();
}

void print(const ExifData& exifData) {
  if (exifData.empty()) {
    std::string error("No Exif data found in the file");
    throw Exiv2::Error(ErrorCode::kerErrorMessage, error);
  }
  for (const auto& i : exifData) {
    std::cout << std::setw(44) << std::setfill(' ') << std::left << i.key() << " "
              << "0x" << std::setw(4) << std::setfill('0') << std::right << std::hex << i.tag() << " " << std::setw(9)
              << std::setfill(' ') << std::left << i.typeName() << " " << std::dec << std::setw(3) << std::setfill(' ')
              << std::right << i.count() << "  " << std::dec << i.value() << "\n";
  }
}
