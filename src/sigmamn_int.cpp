// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "sigmamn_int.hpp"
#include "i18n.h"  // NLS support.
#include "tags_int.hpp"
#include "types.hpp"
#include "value.hpp"

// + standard includes
#include <sstream>
#include <string>

// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {
// Sigma (Foveon) MakerNote Tag Info
constexpr TagInfo SigmaMakerNote::tagInfo_[] = {
    {0x0002, "SerialNumber", N_("Serial Number"), N_("Camera serial number"), IfdId::sigmaId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0003, "DriveMode", N_("Drive Mode"), N_("Drive mode"), IfdId::sigmaId, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x0004, "ResolutionMode", N_("Resolution Mode"), N_("Resolution mode"), IfdId::sigmaId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0005, "AutofocusMode", N_("Autofocus Mode"), N_("Autofocus mode"), IfdId::sigmaId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0006, "FocusSetting", N_("Focus Setting"), N_("Focus setting"), IfdId::sigmaId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0007, "WhiteBalance", N_("White Balance"), N_("White balance"), IfdId::sigmaId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0008, "ExposureMode", N_("Exposure Mode"), N_("Exposure mode"), IfdId::sigmaId, SectionId::makerTags,
     asciiString, -1, print0x0008},
    {0x0009, "MeteringMode", N_("Metering Mode"), N_("Metering mode"), IfdId::sigmaId, SectionId::makerTags,
     asciiString, -1, print0x0009},
    {0x000a, "LensRange", N_("Lens Range"), N_("Lens focal length range"), IfdId::sigmaId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x000b, "ColorSpace", N_("Color Space"), N_("Color space"), IfdId::sigmaId, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x000c, "Exposure", N_("Exposure"), N_("Exposure"), IfdId::sigmaId, SectionId::makerTags, asciiString, -1,
     printStripLabel},
    {0x000d, "Contrast", N_("Contrast"), N_("Contrast"), IfdId::sigmaId, SectionId::makerTags, asciiString, -1,
     printStripLabel},
    {0x000e, "Shadow", N_("Shadow"), N_("Shadow"), IfdId::sigmaId, SectionId::makerTags, asciiString, -1,
     printStripLabel},
    {0x000f, "Highlight", N_("Highlight"), N_("Highlight"), IfdId::sigmaId, SectionId::makerTags, asciiString, -1,
     printStripLabel},
    {0x0010, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::sigmaId, SectionId::makerTags, asciiString, -1,
     printStripLabel},
    {0x0011, "Sharpness", N_("Sharpness"), N_("Sharpness"), IfdId::sigmaId, SectionId::makerTags, asciiString, -1,
     printStripLabel},
    {0x0012, "FillLight", N_("Fill Light"), N_("X3 Fill light"), IfdId::sigmaId, SectionId::makerTags, asciiString, -1,
     printStripLabel},
    {0x0014, "ColorAdjustment", N_("Color Adjustment"), N_("Color adjustment"), IfdId::sigmaId, SectionId::makerTags,
     asciiString, -1, printStripLabel},
    {0x0015, "AdjustmentMode", N_("Adjustment Mode"), N_("Adjustment mode"), IfdId::sigmaId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0016, "Quality", N_("Quality"), N_("Quality"), IfdId::sigmaId, SectionId::makerTags, asciiString, -1,
     printStripLabel},
    {0x0017, "Firmware", N_("Firmware"), N_("Firmware"), IfdId::sigmaId, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x0018, "Software", N_("Software"), N_("Software"), IfdId::sigmaId, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x0019, "AutoBracket", N_("Auto Bracket"), N_("Auto bracket"), IfdId::sigmaId, SectionId::makerTags, asciiString,
     -1, printValue},
    // End of list marker
    {0xffff, "(UnknownSigmaMakerNoteTag)", "(UnknownSigmaMakerNoteTag)", N_("Unknown SigmaMakerNote tag"),
     IfdId::sigmaId, SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* SigmaMakerNote::tagList() {
  return tagInfo_;
}

std::ostream& SigmaMakerNote::printStripLabel(std::ostream& os, const Value& value, const ExifData*) {
  std::string v = value.toString();
  if (auto pos = v.find(':'); pos != std::string::npos) {
    if (v.at(pos + 1) == ' ')
      ++pos;
    v = v.substr(pos + 1);
  }
  return os << v;
}

std::ostream& SigmaMakerNote::print0x0008(std::ostream& os, const Value& value, const ExifData*) {
  switch (value.toString().at(0)) {
    case 'P':
      os << _("Program");
      break;
    case 'A':
      os << _("Aperture priority");
      break;
    case 'S':
      os << _("Shutter priority");
      break;
    case 'M':
      os << _("Manual");
      break;
    default:
      os << "(" << value << ")";
      break;
  }
  return os;
}

std::ostream& SigmaMakerNote::print0x0009(std::ostream& os, const Value& value, const ExifData*) {
  switch (value.toString().at(0)) {
    case 'A':
      os << _("Average");
      break;
    case 'C':
      os << _("Center");
      break;
    case '8':
      os << _("8-Segment");
      break;
    default:
      os << "(" << value << ")";
      break;
  }
  return os;
}

}  // namespace Exiv2::Internal
