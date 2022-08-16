// SPDX-License-Identifier: GPL-2.0-or-later
// Sample program using high-level metadata access functions

#include <exiv2/exiv2.hpp>
#include <iomanip>
#include <iostream>

static constexpr auto easyAccess = std::array{
    std::pair("Orientation", &Exiv2::orientation),
    std::pair("ISO speed", &Exiv2::isoSpeed),
    std::pair("Date & time original", &Exiv2::dateTimeOriginal),
    std::pair("Flash bias", &Exiv2::flashBias),
    std::pair("Exposure mode", &Exiv2::exposureMode),
    std::pair("Scene mode", &Exiv2::sceneMode),
    std::pair("Macro mode", &Exiv2::macroMode),
    std::pair("Image quality", &Exiv2::imageQuality),
    std::pair("White balance", &Exiv2::whiteBalance),
    std::pair("Lens name", &Exiv2::lensName),
    std::pair("Saturation", &Exiv2::saturation),
    std::pair("Sharpness", &Exiv2::sharpness),
    std::pair("Contrast", &Exiv2::contrast),
    std::pair("Scene capture type", &Exiv2::sceneCaptureType),
    std::pair("Metering mode", &Exiv2::meteringMode),
    std::pair("Camera make", &Exiv2::make),
    std::pair("Camera model", &Exiv2::model),
    std::pair("Exposure time", &Exiv2::exposureTime),
    std::pair("FNumber", &Exiv2::fNumber),
    std::pair("Shutter speed value", &Exiv2::shutterSpeedValue),
    std::pair("Aperture value", &Exiv2::apertureValue),
    std::pair("Brightness value", &Exiv2::brightnessValue),
    std::pair("Exposure bias", &Exiv2::exposureBiasValue),
    std::pair("Max aperture value", &Exiv2::maxApertureValue),
    std::pair("Subject distance", &Exiv2::subjectDistance),
    std::pair("Light source", &Exiv2::lightSource),
    std::pair("Flash", &Exiv2::flash),
    std::pair("Camera serial number", &Exiv2::serialNumber),
    std::pair("Focal length", &Exiv2::focalLength),
    std::pair("Subject location/area", &Exiv2::subjectArea),
    std::pair("Flash energy", &Exiv2::flashEnergy),
    std::pair("Exposure index", &Exiv2::exposureIndex),
    std::pair("Sensing method", &Exiv2::sensingMethod),
    std::pair("AF point", &Exiv2::afPoint),
};

int main(int argc, char** argv) {
  try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    if (argc != 2) {
      std::cout << "Usage: " << argv[0] << " file\n";
      return EXIT_FAILURE;
    }

    auto image = Exiv2::ImageFactory::open(argv[1]);
    image->readMetadata();
    Exiv2::ExifData& ed = image->exifData();

    for (auto&& [label, fct] : easyAccess) {
      auto pos = fct(ed);
      std::cout << std::setw(21) << std::left << label;
      if (pos != ed.end()) {
        std::cout << " (" << std::setw(35) << pos->key() << ") : " << pos->print(&ed) << "\n";
      } else {
        std::cout << " (" << std::setw(35) << " "
                  << ") : \n";
      }
    }

    return EXIT_SUCCESS;
  } catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return EXIT_FAILURE;
  }
}
