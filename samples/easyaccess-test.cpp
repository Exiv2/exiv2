// SPDX-License-Identifier: GPL-2.0-or-later
// Sample program using high-level metadata access functions

#include <exiv2/exiv2.hpp>
#include <iomanip>
#include <iostream>

// Type for an Exiv2 Easy access function
using EasyAccessFct = Exiv2::ExifData::const_iterator (*)(const Exiv2::ExifData&);

static const struct {
  const char* l;
  EasyAccessFct f;
  const char* n;
} easyAccess[] = {
    {"Orientation", &Exiv2::orientation, "Orientation"},
    {"ISO speed", &Exiv2::isoSpeed, "ISOspeed"},
    {"Date & time original", &Exiv2::dateTimeOriginal, "DateTimeOriginal"},
    {"Flash bias", &Exiv2::flashBias, "FlashBias"},
    {"Exposure mode", &Exiv2::exposureMode, "ExposureMode"},
    {"Scene mode", &Exiv2::sceneMode, "SceneMode"},
    {"Macro mode", &Exiv2::macroMode, "MacroMode"},
    {"Image quality", &Exiv2::imageQuality, "ImageQuality"},
    {"White balance", &Exiv2::whiteBalance, "WhiteBalance"},
    {"Lens name", &Exiv2::lensName, "LensName"},
    {"Saturation", &Exiv2::saturation, "Saturation"},
    {"Sharpness", &Exiv2::sharpness, "Sharpness"},
    {"Contrast", &Exiv2::contrast, "Contrast"},
    {"Scene capture type", &Exiv2::sceneCaptureType, "SceneCaptureType"},
    {"Metering mode", &Exiv2::meteringMode, "MeteringMode"},
    {"Camera make", &Exiv2::make, "Make"},
    {"Camera model", &Exiv2::model, "Model"},
    {"Exposure time", &Exiv2::exposureTime, "ExposureTime"},
    {"FNumber", &Exiv2::fNumber, "FNumber"},
    {"Shutter speed value", &Exiv2::shutterSpeedValue, "ShutterSpeed"},
    {"Aperture value", &Exiv2::apertureValue, "Aperture"},
    {"Brightness value", &Exiv2::brightnessValue, "Brightness"},
    {"Exposure bias", &Exiv2::exposureBiasValue, "ExposureBias"},
    {"Max aperture value", &Exiv2::maxApertureValue, "MaxAperture"},
    {"Subject distance", &Exiv2::subjectDistance, "SubjectDistance"},
    {"Light source", &Exiv2::lightSource, "LightSource"},
    {"Flash", &Exiv2::flash, "Flash"},
    {"Camera serial number", &Exiv2::serialNumber, "SerialNumber"},
    {"Focal length", &Exiv2::focalLength, "FocalLength"},
    {"Subject location/area", &Exiv2::subjectArea, "SubjectArea"},
    {"Flash energy", &Exiv2::flashEnergy, "FlashEnergy"},
    {"Exposure index", &Exiv2::exposureIndex, "ExposureIndex"},
    {"Sensing method", &Exiv2::sensingMethod, "SensingMethod"},
    {"AF point", &Exiv2::afPoint, "AFpoint"},
};

static void printFct(EasyAccessFct fct, Exiv2::ExifData ed, const char* label) {
  auto pos = fct(ed);
  std::cout << std::setw(21) << std::left << label;
  if (pos != ed.end()) {
    std::cout << " (" << std::setw(35) << pos->key() << ") : " << pos->print(&ed) << "\n";
  } else {
    std::cout << " (" << std::setw(35) << " "
              << ") : \n";
  }
}

int main(int argc, char** argv) {
  try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    if (argc < 2) {
      int count = 0;
      std::cout << "Usage: " << argv[0] << " file [category [category ...]]\nCategories: ";
      for (auto&& [label, fct, name] : easyAccess) {
        if (count > 0)
          std::cout << " | ";
        if (count == 6 || count == 12 || count == 19 || count == 25 || count == 31)
          std::cout << "\n            ";

        std::cout << name;
        count++;
      }
      std::cout << "\n";
      return EXIT_FAILURE;
    }

    auto image = Exiv2::ImageFactory::open(argv[1]);
    image->readMetadata();
    Exiv2::ExifData& ed = image->exifData();

    if (argc > 2) {
      for (int i = 2; i < argc; i++) {
        bool categoryOk = false;
        for (auto&& [label, fct, name] : easyAccess) {
          if (strcmp(argv[i], name) == 0) {
            printFct(fct, ed, label);
            categoryOk = true;
            break;
          }
        }
        if (!categoryOk) {
          std::cout << "Category >" << argv[i] << "< is invalid.\n";
          return EXIT_FAILURE;
        }
      }
    } else {
      for (auto&& [label, fct, name] : easyAccess) {
        printFct(fct, ed, label);
      }

      return EXIT_SUCCESS;
    }
  } catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return EXIT_FAILURE;
  }
}
