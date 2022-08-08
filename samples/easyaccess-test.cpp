// SPDX-License-Identifier: GPL-2.0-or-later
// Sample program using high-level metadata access functions

#include <exiv2/exiv2.hpp>
#include <iomanip>
#include <iostream>

using EasyAccessFct = Exiv2::ExifData::const_iterator (*)(const Exiv2::ExifData&);
using EasyAccess = std::pair<const char*, EasyAccessFct>;

static const EasyAccess easyAccess[] = {
    {"Orientation", Exiv2::orientation},
    {"ISO speed", Exiv2::isoSpeed},
    {"Date & time original", Exiv2::dateTimeOriginal},
    {"Flash bias", Exiv2::flashBias},
    {"Exposure mode", Exiv2::exposureMode},
    {"Scene mode", Exiv2::sceneMode},
    {"Macro mode", Exiv2::macroMode},
    {"Image quality", Exiv2::imageQuality},
    {"White balance", Exiv2::whiteBalance},
    {"Lens name", Exiv2::lensName},
    {"Saturation", Exiv2::saturation},
    {"Sharpness", Exiv2::sharpness},
    {"Contrast", Exiv2::contrast},
    {"Scene capture type", Exiv2::sceneCaptureType},
    {"Metering mode", Exiv2::meteringMode},
    {"Camera make", Exiv2::make},
    {"Camera model", Exiv2::model},
    {"Exposure time", Exiv2::exposureTime},
    {"FNumber", Exiv2::fNumber},
    {"Shutter speed value", Exiv2::shutterSpeedValue},
    {"Aperture value", Exiv2::apertureValue},
    {"Brightness value", Exiv2::brightnessValue},
    {"Exposure bias", Exiv2::exposureBiasValue},
    {"Max aperture value", Exiv2::maxApertureValue},
    {"Subject distance", Exiv2::subjectDistance},
    {"Light source", Exiv2::lightSource},
    {"Flash", Exiv2::flash},
    {"Camera serial number", Exiv2::serialNumber},
    {"Focal length", Exiv2::focalLength},
    {"Subject location/area", Exiv2::subjectArea},
    {"Flash energy", Exiv2::flashEnergy},
    {"Exposure index", Exiv2::exposureIndex},
    {"Sensing method", Exiv2::sensingMethod},
    {"AF point", Exiv2::afPoint},
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
