// SPDX-License-Identifier: GPL-2.0-or-later
// Sample program using high-level metadata access functions

#include <exiv2/exiv2.hpp>
#include <iomanip>
#include <iostream>

// Type for an Exiv2 Easy access function
using EasyAccessFct = Exiv2::ExifData::const_iterator (*)(const Exiv2::ExifData&);

static constexpr auto easyAccess = std::array{
    std::make_tuple("Orientation", &Exiv2::orientation, "Orientation"),
    std::make_tuple("ISO speed", &Exiv2::isoSpeed, "ISOspeed"),
    std::make_tuple("Date & time original", &Exiv2::dateTimeOriginal, "DateTimeOriginal"),
    std::make_tuple("Flash bias", &Exiv2::flashBias, "FlashBias"),
    std::make_tuple("Exposure mode", &Exiv2::exposureMode, "ExposureMode"),
    std::make_tuple("Scene mode", &Exiv2::sceneMode, "SceneMode"),
    std::make_tuple("Macro mode", &Exiv2::macroMode, "MacroMode"),
    std::make_tuple("Image quality", &Exiv2::imageQuality, "ImageQuality"),
    std::make_tuple("White balance", &Exiv2::whiteBalance, "WhiteBalance"),
    std::make_tuple("Lens name", &Exiv2::lensName, "LensName"),
    std::make_tuple("Saturation", &Exiv2::saturation, "Saturation"),
    std::make_tuple("Sharpness", &Exiv2::sharpness, "Sharpness"),
    std::make_tuple("Contrast", &Exiv2::contrast, "Contrast"),
    std::make_tuple("Scene capture type", &Exiv2::sceneCaptureType, "SceneCaptureType"),
    std::make_tuple("Metering mode", &Exiv2::meteringMode, "MeteringMode"),
    std::make_tuple("Camera make", &Exiv2::make, "Make"),
    std::make_tuple("Camera model", &Exiv2::model, "Model"),
    std::make_tuple("Exposure time", &Exiv2::exposureTime, "ExposureTime"),
    std::make_tuple("FNumber", &Exiv2::fNumber, "FNumber"),
    std::make_tuple("Shutter speed value", &Exiv2::shutterSpeedValue, "ShutterSpeed"),
    std::make_tuple("Aperture value", &Exiv2::apertureValue, "Aperture"),
    std::make_tuple("Brightness value", &Exiv2::brightnessValue, "Brightness"),
    std::make_tuple("Exposure bias", &Exiv2::exposureBiasValue, "ExposureBias"),
    std::make_tuple("Max aperture value", &Exiv2::maxApertureValue, "MaxAperture"),
    std::make_tuple("Subject distance", &Exiv2::subjectDistance, "SubjectDistance"),
    std::make_tuple("Light source", &Exiv2::lightSource, "LightSource"),
    std::make_tuple("Flash", &Exiv2::flash, "Flash"),
    std::make_tuple("Camera serial number", &Exiv2::serialNumber, "SerialNumber"),
    std::make_tuple("Focal length", &Exiv2::focalLength, "FocalLength"),
    std::make_tuple("Subject location/area", &Exiv2::subjectArea, "SubjectArea"),
    std::make_tuple("Flash energy", &Exiv2::flashEnergy, "FlashEnergy"),
    std::make_tuple("Exposure index", &Exiv2::exposureIndex, "ExposureIndex"),
    std::make_tuple("Sensing method", &Exiv2::sensingMethod, "SensingMethod"),
    std::make_tuple("AF point", &Exiv2::afPoint, "AFpoint"),
};

static const void printFct(EasyAccessFct fct, Exiv2::ExifData ed, const char* label) {
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
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

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
          std::cout << "Categoy >" << argv[i] << "< is invalid.\n";
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
