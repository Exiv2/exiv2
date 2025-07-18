// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "nikonmn_int.hpp"

#include "exif.hpp"
#include "i18n.h"  // NLS support.
#include "image_int.hpp"
#include "makernote_int.hpp"
#include "tags_int.hpp"
#include "utils.hpp"
#include "value.hpp"

// + standard includes
#include <array>
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {
//! OffOn, multiple tags
constexpr TagDetails nikonOffOn[] = {
    {0, N_("Off")},
    {1, N_("On")},
};

//! Off, Low, Normal, High, multiple tags
constexpr TagDetails nikonOlnh[] = {
    {0, N_("Off")},
    {1, N_("Low")},
    {3, N_("Normal")},
    {5, N_("High")},
};

//! Off, Low, Normal, High, multiple tags
constexpr TagDetails nikonActiveDLighting[] = {
    {0, N_("Off")}, {1, N_("Low")}, {3, N_("Normal")}, {5, N_("High")}, {7, N_("Extra High")}, {65535, N_("Auto")},
};

//! Focus area for Nikon cameras.
constexpr auto nikonFocusarea = std::array{
    N_("Single area"),   N_("Dynamic area"),       N_("Dynamic area, closest subject"),
    N_("Group dynamic"), N_("Single area (wide)"), N_("Dynamic area (wide)"),
};

// Roger Larsson: My guess is that focuspoints will follow autofocus sensor
// module. Note that relative size and position will vary depending on if
// "wide" or not
//! Focus points for Nikon cameras, used for Nikon 1 and Nikon 3 makernotes.
constexpr auto nikonFocuspoints = std::array{
    N_("Center"),      N_("Top"),        N_("Bottom"),      N_("Left"),      N_("Right"),      N_("Upper-left"),
    N_("Upper-right"), N_("Lower-left"), N_("Lower-right"), N_("Left-most"), N_("Right-most"),
};

//! Shutter Modes (credits to exiftool)
constexpr TagDetails nikonShutterModes[] = {
    {0, "Mechanical"},          {16, "Electronic"},        {48, "Electronic Front Curtain"},
    {64, "Electronic (Movie)"}, {80, "Auto (Mechanical)"}, {81, "Auto (Electronic Front Curtain)"},
};

//! FlashComp, tag 0x0012
constexpr TagDetails nikonFlashComp[] = {
    // From the PHP JPEG Metadata Toolkit
    {0x06, "+1.0 EV"}, {0x04, "+0.7 EV"}, {0x03, "+0.5 EV"}, {0x02, "+0.3 EV"}, {0x00, "0.0 EV"},  {0xfe, "-0.3 EV"},
    {0xfd, "-0.5 EV"}, {0xfc, "-0.7 EV"}, {0xfa, "-1.0 EV"}, {0xf8, "-1.3 EV"}, {0xf7, "-1.5 EV"}, {0xf6, "-1.7 EV"},
    {0xf4, "-2.0 EV"}, {0xf2, "-2.3 EV"}, {0xf1, "-2.5 EV"}, {0xf0, "-2.7 EV"}, {0xee, "-3.0 EV"},
};

//! ColorSpace, tag 0x001e
constexpr TagDetails nikonColorSpace[] = {
    {1, N_("sRGB")},
    {2, N_("Adobe RGB")},
};

//! FlashMode, tag 0x0087
constexpr TagDetails nikonFlashMode[] = {
    {0, N_("Did not fire")},   {1, N_("Fire, manual")},         {3, N_("Not ready")},      {6, N_("Off")},
    {7, N_("Fire, external")}, {8, N_("Fire, commander mode")}, {9, N_("Fire, TTL mode")}, {18, N_("LED Light")},
};

//! ShootingMode, tag 0x0089
constexpr TagDetailsBitmask nikonShootingMode[] = {
    {0x0001, N_("Continuous")},
    {0x0002, N_("Delay")},
    {0x0004, N_("PC Control")},
    {0x0008, N_("Self-timer")},
    {0x0010, N_("Exposure Bracketing")},
    {0x0020, N_("Auto ISO")},
    {0x0040, N_("White-Balance Bracketing")},
    {0x0080, N_("IR Control")},
    {0x0100, N_("D-Lighting Bracketing")},
};

//! ShootingMode D70, tag 0x0089
constexpr TagDetailsBitmask nikonShootingModeD70[] = {
    {0x0001, N_("Continuous")},
    {0x0002, N_("Delay")},
    {0x0004, N_("PC control")},
    {0x0010, N_("Exposure bracketing")},
    {0x0020, N_("Unused LE-NR slowdown")},
    {0x0040, N_("White balance bracketing")},
    {0x0080, N_("IR control")},
};

//! AutoBracketRelease, tag 0x008a
constexpr TagDetails nikonAutoBracketRelease[] = {
    {0, N_("None")},
    {1, N_("Auto release")},
    {2, N_("Manual release")},
};

//! NEFCompression, tag 0x0093
constexpr TagDetails nikonNefCompression[] = {
    {1, N_("Lossy (type 1)")},
    {2, N_("Uncompressed")},
    {3, N_("Lossless")},
    {4, N_("Lossy (type 2)")},
};

//! RetouchHistory, tag 0x009e
constexpr TagDetails nikonRetouchHistory[] = {
    {0, N_("None")},          {3, N_("B & W")},      {4, N_("Sepia")},         {5, N_("Trim")},
    {6, N_("Small picture")}, {7, N_("D-Lighting")}, {8, N_("Red eye")},       {9, N_("Cyanotype")},
    {10, N_("Sky light")},    {11, N_("Warm tone")}, {12, N_("Color custom")}, {13, N_("Image overlay")},
};

//! HighISONoiseReduction, tag 0x00b1
constexpr TagDetails nikonHighISONoiseReduction[] = {
    {0, N_("Off")}, {1, N_("Minimal")}, {2, N_("Low")}, {4, N_("Normal")}, {6, N_("High")},
};

// Nikon1 MakerNote Tag Info
constexpr TagInfo Nikon1MakerNote::tagInfo_[] = {
    {0x0001, "Version", N_("Version"), N_("Nikon Makernote version"), IfdId::nikon1Id, SectionId::makerTags, undefined,
     -1, printValue},
    {0x0002, "ISOSpeed", N_("ISO Speed"), N_("ISO speed setting"), IfdId::nikon1Id, SectionId::makerTags, unsignedShort,
     -1, print0x0002},
    {0x0003, "ColorMode", N_("Color Mode"), N_("Color mode"), IfdId::nikon1Id, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x0004, "Quality", N_("Quality"), N_("Image quality setting"), IfdId::nikon1Id, SectionId::makerTags, asciiString,
     -1, printValue},
    {0x0005, "WhiteBalance", N_("White Balance"), N_("White balance"), IfdId::nikon1Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0006, "Sharpening", N_("Sharpening"), N_("Image sharpening setting"), IfdId::nikon1Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0007, "Focus", N_("Focus"), N_("Focus mode"), IfdId::nikon1Id, SectionId::makerTags, asciiString, -1,
     print0x0007},
    {0x0008, "FlashSetting", N_("Flash Setting"), N_("Flash setting"), IfdId::nikon1Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x000a, "0x000a", "0x000a", N_("Unknown"), IfdId::nikon1Id, SectionId::makerTags, unsignedRational, -1,
     printValue},
    {0x000f, "ISOSelection", N_("ISO Selection"), N_("ISO selection"), IfdId::nikon1Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0010, "DataDump", N_("Data Dump"), N_("Data dump"), IfdId::nikon1Id, SectionId::makerTags, undefined, -1,
     printValue},
    {0x0080, "ImageAdjustment", N_("Image Adjustment"), N_("Image adjustment setting"), IfdId::nikon1Id,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x0082, "AuxiliaryLens", N_("Auxiliary Lens"), N_("Auxiliary lens (adapter)"), IfdId::nikon1Id,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x0085, "FocusDistance", N_("Focus Distance"), N_("Manual focus distance"), IfdId::nikon1Id, SectionId::makerTags,
     unsignedRational, -1, print0x0085},
    {0x0086, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom setting"), IfdId::nikon1Id, SectionId::makerTags,
     unsignedRational, -1, print0x0086},
    {0x0088, "AFFocusPos", N_("AF Focus Position"), N_("AF focus position information"), IfdId::nikon1Id,
     SectionId::makerTags, undefined, -1, print0x0088},
    // End of list marker
    {0xffff, "(UnknownNikon1MnTag)", "(UnknownNikon1MnTag)", N_("Unknown Nikon1MakerNote tag"), IfdId::nikon1Id,
     SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* Nikon1MakerNote::tagList() {
  return tagInfo_;
}

std::ostream& Nikon1MakerNote::print0x0002(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() > 1) {
    os << value.toInt64(1);
  } else {
    os << "(" << value << ")";
  }
  return os;
}

static std::string getKeyString(const std::string& key, const ExifData* metadata) {
  std::string result;
  if (metadata->findKey(ExifKey(key)) != metadata->end()) {
    result = metadata->findKey(ExifKey(key))->toString();
  }
  return result;
}

std::ostream& Nikon1MakerNote::printBarValue(std::ostream& os, const Value& value, const ExifData* exifData) {
  if (!exifData)
    return os << "undefined";

  if (value.count() >= 9) {
    ByteOrder bo = getKeyString("Exif.MakerNote.ByteOrder", exifData) == "MM" ? bigEndian : littleEndian;
    byte p[4];
    for (int n = 0; n < 4; n++)
      p[n] = static_cast<byte>(value.toInt64(6 + n));
    os << getLong(p, bo);
  }

  return os;
}

std::ostream& Nikon1MakerNote::print0x0007(std::ostream& os, const Value& value, const ExifData*) {
  std::string focus = value.toString();
  if (focus == "AF-C  ")
    os << _("Continuous autofocus");
  else if (focus == "AF-S  ")
    os << _("Single autofocus");
  else if (focus == "AF-A  ")
    os << _("Automatic");
  else
    os << "(" << value << ")";
  return os;
}

std::ostream& Nikon1MakerNote::print0x0085(std::ostream& os, const Value& value, const ExifData*) {
  auto [r, s] = value.toRational();
  if (r == 0) {
    return os << _("Unknown");
  }
  if (s != 0) {
    return os << stringFormat("{:.2f} m", static_cast<float>(r) / s);
  }
  return os << "(" << value << ")";
}

std::ostream& Nikon1MakerNote::print0x0086(std::ostream& os, const Value& value, const ExifData*) {
  auto [r, s] = value.toRational();
  if (r == 0) {
    return os << _("Not used");
  }
  if (s == 0) {
    return os << "(" << value << ")";
  }
  return os << stringFormat("{:.1f}x", static_cast<float>(r) / s);
}

std::ostream& Nikon1MakerNote::print0x0088(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() >= 1) {
    const uint32_t focusArea = value.toUint32(0);
    if (focusArea >= nikonFocusarea.size()) {
      os << "Invalid value";
    } else {
      os << nikonFocusarea[focusArea];
    }
  }
  if (value.count() >= 2) {
    os << "; ";
    const uint32_t focusPoint = value.toUint32(1);

    if (focusPoint <= 4) {
      os << nikonFocuspoints[focusPoint];
    } else {
      os << value;
      if (focusPoint < nikonFocuspoints.size()) {
        os << " " << _("guess") << " " << nikonFocuspoints[focusPoint];
      }
    }
  }
  if (value.count() >= 3) {
    const uint32_t focusPointsUsed1 = value.toUint32(2);
    const uint32_t focusPointsUsed2 = value.toUint32(3);

    if (focusPointsUsed1 != 0 && focusPointsUsed2 != 0) {
      os << "; [";

      if (focusPointsUsed1 & 1)
        os << nikonFocuspoints[0] << " ";
      if (focusPointsUsed1 & 2)
        os << nikonFocuspoints[1] << " ";
      if (focusPointsUsed1 & 4)
        os << nikonFocuspoints[2] << " ";
      if (focusPointsUsed1 & 8)
        os << nikonFocuspoints[3] << " ";
      if (focusPointsUsed1 & 16)
        os << nikonFocuspoints[4] << " ";
      if (focusPointsUsed1 & 32)
        os << nikonFocuspoints[5] << " ";
      if (focusPointsUsed1 & 64)
        os << nikonFocuspoints[6] << " ";
      if (focusPointsUsed1 & 128)
        os << nikonFocuspoints[7] << " ";

      if (focusPointsUsed2 & 1)
        os << nikonFocuspoints[8] << " ";
      if (focusPointsUsed2 & 2)
        os << nikonFocuspoints[9] << " ";
      if (focusPointsUsed2 & 4)
        os << nikonFocuspoints[10] << " ";

      os << "]";
    }
  } else {
    os << "(" << value << ")";
  }
  return os;
}

//! Quality, tag 0x0003
constexpr TagDetails nikon2Quality[] = {
    {1, N_("VGA Basic")},  {2, N_("VGA Normal")},  {3, N_("VGA Fine")},
    {4, N_("SXGA Basic")}, {5, N_("SXGA Normal")}, {6, N_("SXGA Fine")},
};

//! ColorMode, tag 0x0004
constexpr TagDetails nikon2ColorMode[] = {
    {1, N_("Color")},
    {2, N_("Monochrome")},
};

//! ImageAdjustment, tag 0x0005
constexpr TagDetails nikon2ImageAdjustment[] = {
    {0, N_("Normal")}, {1, N_("Bright+")}, {2, N_("Bright-")}, {3, N_("Contrast+")}, {4, N_("Contrast-")},
};

//! ISOSpeed, tag 0x0006
constexpr TagDetails nikon2IsoSpeed[] = {
    {0, "80"},
    {2, "160"},
    {4, "320"},
    {5, "100"},
};

//! WhiteBalance, tag 0x0007
constexpr TagDetails nikon2WhiteBalance[] = {
    {0, N_("Auto")},        {1, N_("Preset")}, {2, N_("Daylight")},   {3, N_("Incandescent")},
    {4, N_("Fluorescent")}, {5, N_("Cloudy")}, {6, N_("Speedlight")},
};

// Nikon2 MakerNote Tag Info
constexpr TagInfo Nikon2MakerNote::tagInfo_[] = {
    {0x0002, "0x0002", "0x0002", N_("Unknown"), IfdId::nikon2Id, SectionId::makerTags, asciiString, -1, printValue},
    {0x0003, "Quality", N_("Quality"), N_("Image quality setting"), IfdId::nikon2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(nikon2Quality)},
    {0x0004, "ColorMode", N_("Color Mode"), N_("Color mode"), IfdId::nikon2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(nikon2ColorMode)},
    {0x0005, "ImageAdjustment", N_("Image Adjustment"), N_("Image adjustment setting"), IfdId::nikon2Id,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(nikon2ImageAdjustment)},
    {0x0006, "ISOSpeed", N_("ISO Speed"), N_("ISO speed setting"), IfdId::nikon2Id, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(nikon2IsoSpeed)},
    {0x0007, "WhiteBalance", N_("White Balance"), N_("White balance"), IfdId::nikon2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(nikon2WhiteBalance)},
    {0x0008, "Focus", N_("Focus Mode"), N_("Focus mode"), IfdId::nikon2Id, SectionId::makerTags, unsignedRational, -1,
     printValue},
    {0x0009, "0x0009", "0x0009", N_("Unknown"), IfdId::nikon2Id, SectionId::makerTags, asciiString, -1, printValue},
    {0x000a, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom setting"), IfdId::nikon2Id, SectionId::makerTags,
     unsignedRational, -1, print0x000a},
    {0x000b, "AuxiliaryLens", N_("Auxiliary Lens"), N_("Auxiliary lens (adapter)"), IfdId::nikon2Id,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0f00, "0x0f00", "0x0f00", N_("Unknown"), IfdId::nikon2Id, SectionId::makerTags, unsignedLong, -1, printValue},
    // End of list marker
    {0xffff, "(UnknownNikon2MnTag)", "(UnknownNikon2MnTag)", N_("Unknown Nikon2MakerNote tag"), IfdId::nikon2Id,
     SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* Nikon2MakerNote::tagList() {
  return tagInfo_;
}

std::ostream& Nikon2MakerNote::print0x000a(std::ostream& os, const Value& value, const ExifData*) {
  auto [r, s] = value.toRational();
  if (r == 0)
    return os << _("Not used");
  if (s == 0)
    return os << "(" << value << ")";
  return os << stringFormat("{:.1f}x", static_cast<float>(r) / s);
}

// Nikon3 MakerNote Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfo_[] = {
    {0x0001, "Version", N_("Version"), N_("Nikon Makernote version"), IfdId::nikon3Id, SectionId::makerTags, undefined,
     -1, printExifVersion},
    {0x0002, "ISOSpeed", N_("ISO Speed"), N_("ISO speed setting"), IfdId::nikon3Id, SectionId::makerTags, unsignedShort,
     -1, print0x0002},
    {0x0003, "ColorMode", N_("Color Mode"), N_("Color mode"), IfdId::nikon3Id, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x0004, "Quality", N_("Quality"), N_("Image quality setting"), IfdId::nikon3Id, SectionId::makerTags, asciiString,
     -1, printValue},
    {0x0005, "WhiteBalance", N_("White Balance"), N_("White balance"), IfdId::nikon3Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0006, "Sharpening", N_("Sharpening"), N_("Image sharpening setting"), IfdId::nikon3Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0007, "Focus", N_("Focus"), N_("Focus mode"), IfdId::nikon3Id, SectionId::makerTags, asciiString, -1,
     print0x0007},
    {0x0008, "FlashSetting", N_("Flash Setting"), N_("Flash setting"), IfdId::nikon3Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0009, "FlashDevice", N_("Flash Device"), N_("Flash device"), IfdId::nikon3Id, SectionId::makerTags, asciiString,
     -1, printValue},
    {0x000a, "0x000a", "0x000a", N_("Unknown"), IfdId::nikon3Id, SectionId::makerTags, unsignedRational, -1,
     printValue},
    {0x000b, "WhiteBalanceBias", N_("White Balance Bias"), N_("White balance bias"), IfdId::nikon3Id,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x000c, "WB_RBLevels", N_("WB RB Levels"), N_("WB RB levels"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedRational, -1, printValue},
    {0x000d, "ProgramShift", N_("Program Shift"), N_("Program shift"), IfdId::nikon3Id, SectionId::makerTags, undefined,
     -1, EXV_PRINT_TAG(nikonFlashComp)},
    {0x000e, "ExposureDiff", N_("Exposure Difference"), N_("Exposure difference"), IfdId::nikon3Id,
     SectionId::makerTags, undefined, -1, EXV_PRINT_TAG(nikonFlashComp)},
    {0x000f, "ISOSelection", N_("ISO Selection"), N_("ISO selection"), IfdId::nikon3Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0010, "DataDump", N_("Data Dump"), N_("Data dump"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1,
     printValue},
    {0x0011, "Preview", N_("Pointer to a preview image"), N_("Offset to an IFD containing a preview image"),
     IfdId::nikon3Id, SectionId::makerTags, undefined, -1, printValue},
    {0x0012, "FlashComp", N_("Flash Comp"), N_("Flash compensation setting"), IfdId::nikon3Id, SectionId::makerTags,
     undefined, -1, EXV_PRINT_TAG(nikonFlashComp)},
    {0x0013, "ISOSettings", N_("ISO Settings"), N_("ISO setting"), IfdId::nikon3Id, SectionId::makerTags, unsignedShort,
     -1, print0x0002},  // use 0x0002 print fct
    {0x0016, "ImageBoundary", N_("Image Boundary"), N_("Image boundary"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0017, "FlashExposureComp", "Flash Exposure Comp", N_("Flash exposure comp"), IfdId::nikon3Id,
     SectionId::makerTags, undefined, -1, EXV_PRINT_TAG(nikonFlashComp)},
    {0x0018, "FlashBracketComp", N_("Flash Bracket Comp"), N_("Flash bracket compensation applied"), IfdId::nikon3Id,
     SectionId::makerTags, undefined, -1, EXV_PRINT_TAG(nikonFlashComp)},  // use 0x0012 print fct
    {0x0019, "ExposureBracketComp", N_("Exposure Bracket Comp"), N_("AE bracket compensation applied"), IfdId::nikon3Id,
     SectionId::makerTags, signedRational, -1, printValue},
    {0x001a, "ImageProcessing", N_("Image Processing"), N_("Image processing"), IfdId::nikon3Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x001b, "CropHiSpeed", N_("Crop High Speed"), N_("Crop high speed"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x001c, "ExposureTuning", N_("Exposure Tuning"), N_("Exposure tuning"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x001d, "SerialNumber", N_("Serial Number"), N_("Serial Number"), IfdId::nikon3Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x001e, "ColorSpace", N_("Color Space"), N_("Color space"), IfdId::nikon3Id, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(nikonColorSpace)},
    {0x001f, "VRInfo", N_("VR Info"), N_("VR info"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1, printValue},
    {0x0020, "ImageAuthentication", N_("Image Authentication"), N_("Image authentication"), IfdId::nikon3Id,
     SectionId::makerTags, unsignedByte, -1, EXV_PRINT_TAG(nikonOffOn)},
    {0x0022, "ActiveDLighting", N_("ActiveD-Lighting"), N_("ActiveD-lighting"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(nikonActiveDLighting)},
    {0x0023, "PictureControl", N_("Picture Control"), N_(" Picture control"), IfdId::nikon3Id, SectionId::makerTags,
     undefined, -1, printValue},
    {0x0024, "WorldTime", N_("World Time"), N_("World time"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1,
     printValue},
    {0x0025, "ISOInfo", N_("ISO Info"), N_("ISO info"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1,
     printValue},
    {0x002a, "VignetteControl", N_("Vignette Control"), N_("Vignette control"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(nikonOlnh)},
    {0x0034, "ShutterMode", N_("Shutter Mode"), N_("Shutter mode"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(nikonShutterModes)},
    {0x0037, "MechanicalShutterCount", N_("Mechanical Shutter Count"), N_("Mechanical shutter count"), IfdId::nikon3Id,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x003f, "WhiteBalanceBias2", N_("White Balance Bias 2"), N_("White balance bias 2"), IfdId::nikon3Id,
     SectionId::makerTags, signedRational, -1, printValue},
    {0x0080, "ImageAdjustment", N_("Image Adjustment"), N_("Image adjustment setting"), IfdId::nikon3Id,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x0081, "ToneComp", N_("Tone Compensation"), N_("Tone compensation"), IfdId::nikon3Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0082, "AuxiliaryLens", N_("Auxiliary Lens"), N_("Auxiliary lens (adapter)"), IfdId::nikon3Id,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x0083, "LensType", N_("Lens Type"), N_("Lens type"), IfdId::nikon3Id, SectionId::makerTags, unsignedByte, -1,
     print0x0083},
    {0x0084, "Lens", N_("Lens"), N_("Lens"), IfdId::nikon3Id, SectionId::makerTags, unsignedRational, -1,
     printLensSpecification},
    {0x0085, "FocusDistance", N_("Focus Distance"), N_("Manual focus distance"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedRational, -1, print0x0085},
    {0x0086, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom setting"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedRational, -1, print0x0086},
    {0x0087, "FlashMode", N_("Flash Mode"), N_("Mode of flash used"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedByte, -1, EXV_PRINT_TAG(nikonFlashMode)},
    {0x0088, "AFInfo", N_("AF Info"), N_("AF info"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1, printValue},
    {0x0089, "ShootingMode", N_("Shooting Mode"), N_("Shooting mode"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedShort, -1, print0x0089},
    {0x008a, "AutoBracketRelease", N_("Auto Bracket Release"), N_("Auto bracket release"), IfdId::nikon3Id,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(nikonAutoBracketRelease)},
    {0x008b, "LensFStops", N_("Lens FStops"), N_("Lens FStops"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1,
     print0x008b},
    {0x008c, "ContrastCurve", N_("Contrast Curve"), N_("Contrast curve"), IfdId::nikon3Id, SectionId::makerTags,
     undefined, -1, printValue},
    {0x008d, "ColorHue", N_("Color Hue"), N_("Color hue"), IfdId::nikon3Id, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x008f, "SceneMode", N_("Scene Mode"), N_("Scene mode"), IfdId::nikon3Id, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x0090, "LightSource", N_("Light Source"), N_("Light source"), IfdId::nikon3Id, SectionId::makerTags, asciiString,
     -1, printValue},
    {0x0091, "ShotInfo", "Shot Info", N_("Shot info"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1,
     printValue},
    {0x0092, "HueAdjustment", N_("Hue Adjustment"), N_("Hue adjustment"), IfdId::nikon3Id, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x0093, "NEFCompression", N_("NEF Compression"), N_("NEF compression"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(nikonNefCompression)},
    {0x0094, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::nikon3Id, SectionId::makerTags, signedShort, -1,
     printValue},
    {0x0095, "NoiseReduction", N_("Noise Reduction"), N_("Noise reduction"), IfdId::nikon3Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0096, "LinearizationTable", N_("Linearization Table"), N_("Linearization table"), IfdId::nikon3Id,
     SectionId::makerTags, undefined, -1, printValue},
    {0x0097, "ColorBalance", N_("Color Balance"), N_("Color balance"), IfdId::nikon3Id, SectionId::makerTags, undefined,
     -1, printValue},
    {0x0098, "LensData", N_("Lens Data"), N_("Lens data settings"), IfdId::nikon3Id, SectionId::makerTags, undefined,
     -1, printValue},
    {0x0099, "RawImageCenter", N_("Raw Image Center"), N_("Raw image center"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x009a, "SensorPixelSize", N_("Sensor Pixel Size"), N_("Sensor pixel size"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedRational, -1, print0x009a},
    {0x009b, "0x009b", "0x009b", N_("Unknown"), IfdId::nikon3Id, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x009c, "SceneAssist", N_("Scene Assist"), N_("Scene assist"), IfdId::nikon3Id, SectionId::makerTags, asciiString,
     -1, printValue},
    {0x009e, "RetouchHistory", N_("Retouch History"), N_("Retouch history"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedShort, -1, print0x009e},
    {0x009f, "0x009f", "0x009f", N_("Unknown"), IfdId::nikon3Id, SectionId::makerTags, signedShort, -1, printValue},
    {0x00a0, "SerialNO", N_("Serial NO"), N_("Camera serial number, usually starts with \"NO= \""), IfdId::nikon3Id,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x00a2, "ImageDataSize", N_("Image Data Size"), N_("Image data size"), IfdId::nikon3Id, SectionId::makerTags,
     unsignedLong, -1, printValue},
    {0x00a3, "0x00a3", "0x00a3", N_("Unknown"), IfdId::nikon3Id, SectionId::makerTags, unsignedByte, -1, printValue},
    {0x00a5, "ImageCount", N_("Image Count"), N_("Image count"), IfdId::nikon3Id, SectionId::makerTags, unsignedLong,
     -1, printValue},
    {0x00a6, "DeletedImageCount", N_("Deleted Image Count"), N_("Deleted image count"), IfdId::nikon3Id,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x00a7, "ShutterCount", N_("Shutter Count"), N_("Number of shots taken by camera"), IfdId::nikon3Id,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x00a8, "FlashInfo", "Flash Info", N_("Flash info"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1,
     printValue},
    {0x00a9, "ImageOptimization", N_("Image Optimization"), N_("Image optimization"), IfdId::nikon3Id,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x00aa, "Saturation2", N_("Saturation 2"), N_("Saturation 2"), IfdId::nikon3Id, SectionId::makerTags, asciiString,
     -1, printValue},
    {0x00ab, "VariProgram", N_("Program Variation"), N_("Program variation"), IfdId::nikon3Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x00ac, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), IfdId::nikon3Id,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x00ad, "AFResponse", N_("AF Response"), N_("AF response"), IfdId::nikon3Id, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x00b0, "MultiExposure", "Multi Exposure", N_("Multi exposure"), IfdId::nikon3Id, SectionId::makerTags, undefined,
     -1, printValue},
    {0x00b1, "HighISONoiseReduction", N_("High ISO Noise Reduction"), N_("High ISO Noise Reduction"), IfdId::nikon3Id,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(nikonHighISONoiseReduction)},
    {0x00b3, "ToningEffect", "Toning Effect", N_("Toning effect"), IfdId::nikon3Id, SectionId::makerTags, asciiString,
     -1, printValue},
    {0x00b7, "AFInfo2", "AF Info 2", N_("AF info 2"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1, printValue},
    {0x00b8, "FileInfo", "File Info", N_("File info"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1,
     printValue},
    {0x00b9, "AFTune", "AF Tune", N_("AF tune"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1, printValue},
    {0x00c3, "BarometerInfo", "Barometer Info", N_("Barometer Info"), IfdId::nikon3Id, SectionId::makerTags, signedLong,
     -1, Nikon1MakerNote::printBarValue},
    {0x0e00, "PrintIM", N_("Print IM"), N_("PrintIM information"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1,
     printValue},
    // TODO: Add Capture Data decoding implementation.
    {0x0e01, "CaptureData", N_("Capture Data"), N_("Capture data"), IfdId::nikon3Id, SectionId::makerTags, undefined,
     -1, printValue},
    {0x0e09, "CaptureVersion", N_("Capture Version"), N_("Capture version"), IfdId::nikon3Id, SectionId::makerTags,
     asciiString, -1, printValue},
    // TODO: Add Capture Offsets decoding implementation.
    {0x0e0e, "CaptureOffsets", N_("Capture Offsets"), N_("Capture offsets"), IfdId::nikon3Id, SectionId::makerTags,
     undefined, -1, printValue},
    {0x0e10, "ScanIFD", "Scan IFD", N_("Scan IFD"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1, printValue},
    {0x0e1d, "ICCProfile", "ICC Profile", N_("ICC profile"), IfdId::nikon3Id, SectionId::makerTags, undefined, -1,
     printValue},
    {0x0e1e, "CaptureOutput", "Capture Output", N_("Capture output"), IfdId::nikon3Id, SectionId::makerTags, undefined,
     -1, printValue},
    // End of list marker
    {0xffff, "(UnknownNikon3MnTag)", "(UnknownNikon3MnTag)", N_("Unknown Nikon3MakerNote tag"), IfdId::nikon3Id,
     SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* Nikon3MakerNote::tagList() {
  return tagInfo_;
}

//! YesNo, used for DaylightSavings, tag index 2, et al.
constexpr TagDetails nikonYesNo[] = {
    {0, N_("No")},
    {1, N_("Yes")},
};

//! DateDisplayFormat, tag index 3
constexpr TagDetails nikonDateDisplayFormat[] = {
    {0, N_("Y/M/D")},
    {1, N_("M/D/Y")},
    {2, N_("D/M/Y")},
};

//! OnOff
constexpr TagDetails nikonOnOff[] = {
    {1, N_("On")},
    {2, N_("Off")},
};

// Nikon3 Vibration Reduction Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoVr_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonVrId, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {4, "VibrationReduction", N_("Vibration Reduction"), N_("Vibration reduction"), IfdId::nikonVrId,
     SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonOnOff)},
    // End of list marker
    {0xffff, "(UnknownNikonVrTag)", "(UnknownNikonVrTag)", N_("Unknown Nikon Vibration Reduction Tag"),
     IfdId::nikonVrId, SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListVr() {
  return tagInfoVr_;
}

//! Adjust
constexpr TagDetails nikonAdjust[] = {
    {0, N_("Default Settings")},
    {1, N_("Quick Adjust")},
    {2, N_("Full Control")},
};

//! FilterEffect
constexpr TagDetails nikonFilterEffect[] = {
    {0x80, N_("Off")}, {0x81, N_("Yellow")}, {0x82, N_("Orange")},
    {0x83, N_("Red")}, {0x84, N_("Green")},  {0xff, N_("n/a")},
};

//! ToningEffect
constexpr TagDetails nikonToningEffect[] = {
    {0x80, N_("B&W")},         {0x81, N_("Sepia")},      {0x82, N_("Cyanotype")},  {0x83, N_("Red")},
    {0x84, N_("Yellow")},      {0x85, N_("Green")},      {0x86, N_("Blue-green")}, {0x87, N_("Blue")},
    {0x88, N_("Purple-blue")}, {0x89, N_("Red-purple")}, {0xff, N_("n/a")},
};

// Nikon3 Picture Control Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoPc_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonPcId, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {4, "Name", N_("Name"), N_("Name"), IfdId::nikonPcId, SectionId::makerTags, asciiString, 20, printValue},
    {24, "Base", N_("Base"), N_("Base"), IfdId::nikonPcId, SectionId::makerTags, asciiString, 20, printValue},
    {48, "Adjust", N_("Adjust"), N_("Adjust"), IfdId::nikonPcId, SectionId::makerTags, unsignedByte, 1,
     EXV_PRINT_TAG(nikonAdjust)},
    {49, "QuickAdjust", N_("Quick Adjust"), N_("Quick adjust"), IfdId::nikonPcId, SectionId::makerTags, unsignedByte, 1,
     printPictureControl},
    {50, "Sharpness", N_("Sharpness"), N_("Sharpness"), IfdId::nikonPcId, SectionId::makerTags, unsignedByte, 1,
     printPictureControl},
    {51, "Contrast", N_("Contrast"), N_("Contrast"), IfdId::nikonPcId, SectionId::makerTags, unsignedByte, 1,
     printPictureControl},
    {52, "Brightness", N_("Brightness"), N_("Brightness"), IfdId::nikonPcId, SectionId::makerTags, unsignedByte, 1,
     printPictureControl},
    {53, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::nikonPcId, SectionId::makerTags, unsignedByte, 1,
     printPictureControl},
    {54, "HueAdjustment", N_("Hue Adjustment"), N_("Hue adjustment"), IfdId::nikonPcId, SectionId::makerTags,
     unsignedByte, 1, printPictureControl},
    {55, "FilterEffect", N_("Filter Effect"), N_("Filter effect"), IfdId::nikonPcId, SectionId::makerTags, unsignedByte,
     1, EXV_PRINT_TAG(nikonFilterEffect)},
    {56, "ToningEffect", N_("Toning Effect"), N_("Toning effect"), IfdId::nikonPcId, SectionId::makerTags, unsignedByte,
     1, EXV_PRINT_TAG(nikonToningEffect)},
    {57, "ToningSaturation", N_("Toning Saturation"), N_("Toning saturation"), IfdId::nikonPcId, SectionId::makerTags,
     unsignedByte, 1, printPictureControl},
    // End of list marker
    {0xffff, "(UnknownNikonPcTag)", "(UnknownNikonPcTag)", N_("Unknown Nikon Picture Control Tag"), IfdId::nikonPcId,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListPc() {
  return tagInfoPc_;
}

//! OnOff
constexpr TagDetails aftOnOff[] = {
    {0, N_("Off")},
    {1, N_("On")},
    {2, N_("On")},
};

// Nikon3 AF Fine Tune
constexpr TagInfo Nikon3MakerNote::tagInfoAFT_[] = {
    {0, "AFFineTune", N_("AF Fine Tune"), N_("AF fine tune"), IfdId::nikonAFTId, SectionId::makerTags, unsignedByte, 1,
     EXV_PRINT_TAG(aftOnOff)},
    {1, "AFFineTuneIndex", N_("AF Fine Tune Index"), N_("AF fine tune index"), IfdId::nikonAFTId, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {2, "AFFineTuneAdj", N_("AF Fine Tune Adjustment"), N_("AF fine tune adjustment"), IfdId::nikonAFTId,
     SectionId::makerTags, signedByte, 1, printValue},
    // End of list marker
    {0xffff, "(UnknownNikonAFTTag)", "(UnknownNikonAFTTag)", N_("Unknown Nikon AF Fine Tune Tag"), IfdId::nikonAFTId,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListAFT() {
  return tagInfoAFT_;
}

// Nikon3 World Time Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoWt_[] = {
    {0, "Timezone", N_("Timezone"), N_("Timezone"), IfdId::nikonWtId, SectionId::makerTags, signedShort, 1,
     printTimeZone},
    {2, "DaylightSavings", N_("Daylight Savings"), N_("Daylight savings"), IfdId::nikonWtId, SectionId::makerTags,
     unsignedByte, 1, EXV_PRINT_TAG(nikonYesNo)},
    {3, "DateDisplayFormat", N_("Date Display Format"), N_("Date display format"), IfdId::nikonWtId,
     SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonDateDisplayFormat)},
    // End of list marker
    {0xffff, "(UnknownNikonWtTag)", "(UnknownNikonWtTag)", N_("Unknown Nikon World Time Tag"), IfdId::nikonWtId,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListWt() {
  return tagInfoWt_;
}

//! ISOExpansion, tag index 4 and 10
constexpr TagDetails nikonIsoExpansion[] = {
    {0x000, N_("Off")},    {0x101, N_("Hi 0.3")}, {0x102, N_("Hi 0.5")}, {0x103, N_("Hi 0.7")}, {0x104, N_("Hi 1.0")},
    {0x105, N_("Hi 1.3")}, {0x106, N_("Hi 1.5")}, {0x107, N_("Hi 1.7")}, {0x108, N_("Hi 2.0")}, {0x109, N_("Hi 2.3")},
    {0x10a, N_("Hi 2.5")}, {0x10b, N_("Hi 2.7")}, {0x10c, N_("Hi 3.0")}, {0x10d, N_("Hi 3.3")}, {0x10e, N_("Hi 3.5")},
    {0x10f, N_("Hi 3.7")}, {0x110, N_("Hi 4.0")}, {0x111, N_("Hi 4.3")}, {0x112, N_("Hi 4.5")}, {0x113, N_("Hi 4.7")},
    {0x114, N_("Hi 5.0")}, {0x201, N_("Lo 0.3")}, {0x202, N_("Lo 0.5")}, {0x203, N_("Lo 0.7")}, {0x204, N_("Lo 1.0")},
};

// Nikon3 ISO Info Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoIi_[] = {
    {0, "ISO", N_("ISO"), N_("ISO"), IfdId::nikonIiId, SectionId::makerTags, unsignedByte, 1, printIiIso},
    {4, "ISOExpansion", N_("ISO Expansion"), N_("ISO expansion"), IfdId::nikonIiId, SectionId::makerTags, unsignedShort,
     1, EXV_PRINT_TAG(nikonIsoExpansion)},
    {6, "ISO2", N_("ISO 2"), N_("ISO 2"), IfdId::nikonIiId, SectionId::makerTags, unsignedByte, 1, printIiIso},
    {10, "ISOExpansion2", N_("ISO Expansion 2"), N_("ISO expansion 2"), IfdId::nikonIiId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(nikonIsoExpansion)},
    // End of list marker
    {0xffff, "(UnknownNikonIiTag)", "(UnknownNikonIiTag)", N_("Unknown Nikon Iso Info Tag"), IfdId::nikonIiId,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListIi() {
  return tagInfoIi_;
}

//! AfAreaMode
constexpr TagDetails nikonAfAreaMode[] = {
    {0, N_("Single Area")},   {1, N_("Dynamic Area")},       {2, N_("Dynamic Area, Closest Subject")},
    {3, N_("Group Dynamic")}, {4, N_("Single Area (wide)")}, {5, N_("Dynamic Area (wide)")},
};

//! AF2 Area Mode when Contrast Detect AF is off
constexpr TagDetails nikonAf2AreaModeContrastDetectAfOff[] = {
    {0, N_("Single-point AF")},
    {1, N_("Dynamic-area AF")},
    {2, N_("Closest Subject")},
    {3, N_("Group Dynamic AF")},
    {4, N_("Dynamic-area AF (9 points)")},
    {5, N_("Dynamic-area AF (21 points)")},
    {6, N_("Dynamic-area AF (51 points)")},
    {7, N_("Dynamic-area AF (51 points), 3D-tracking")},
    {8, N_("Auto-area AF")},
    {9, N_("3D-tracking")},
    {10, N_("Single Area AF, Wide")},
    {11, N_("Dynamic-area AF, Wide")},
    {12, N_("3D-tracking/Wide")},
    {13, N_("Group-area AF")},
    {14, N_("Dynamic-area AF (25 points)")},
    {15, N_("Dynamic-area AF (72 points)")},
    {16, N_("Group-area AF (HL)")},
    {17, N_("Group-area AF (VL)")},
};

//! AF2 Area Mode when Contrast Detect AF is on
constexpr TagDetails nikonAf2AreaModeContrastDetectAfOn[] = {
    {0, N_("Contrast AF")},      {1, N_("Normal-area AF")},      {2, N_("Wide-area AF")},
    {3, N_("Face-priority AF")}, {4, N_("Subject-tracking AF")}, {5, N_("Pinpoint AF")},
};

//! AfPoint
constexpr TagDetails nikonAfPoint[] = {
    {0, N_("Center")},      {1, N_("Top")},        {2, N_("Bottom")},      {3, N_("Mid-left")},
    {4, N_("Mid-right")},   {5, N_("Upper-left")}, {6, N_("Upper-right")}, {7, N_("Lower-left")},
    {8, N_("Lower-right")}, {9, N_("Far Left")},   {10, N_("Far Right")},
};

//! AfPointsInFocus
constexpr TagDetailsBitmask nikonAfPointsInFocus[] = {
    {0x0001, N_("Center")},      {0x0002, N_("Top")},        {0x0004, N_("Bottom")},      {0x0008, N_("Mid-left")},
    {0x0010, N_("Mid-right")},   {0x0020, N_("Upper-left")}, {0x0040, N_("Upper-right")}, {0x0080, N_("Lower-left")},
    {0x0100, N_("Lower-right")}, {0x0200, N_("Far Left")},   {0x0400, N_("Far Right")},
};

// Nikon3 Auto Focus Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoAf_[] = {
    {0, "AFAreaMode", N_("AF Area Mode"), N_("AF area mode"), IfdId::nikonAfId, SectionId::makerTags, unsignedByte, 1,
     EXV_PRINT_TAG(nikonAfAreaMode)},
    {1, "AFPoint", N_("AF Point"), N_("AF point"), IfdId::nikonAfId, SectionId::makerTags, unsignedByte, 1,
     EXV_PRINT_TAG(nikonAfPoint)},
    {2, "AFPointsInFocus", N_("AF Points In Focus"), N_("AF points in focus"), IfdId::nikonAfId, SectionId::makerTags,
     unsignedShort, 1, printAfPointsInFocus},
    // End of list marker
    {0xffff, "(UnknownNikonAfTag)", "(UnknownNikonAfTag)", N_("Unknown Nikon Auto Focus Tag"), IfdId::nikonAfId,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListAf() {
  return tagInfoAf_;
}

//! PhaseDetectAF
constexpr TagDetails nikonPhaseDetectAF[] = {
    {0, N_("Off")},           {1, N_("On (51-point)")},      {2, N_("On (11-point)")},  {3, N_("On (39-point)")},
    {4, N_("On (73-point)")}, {5, N_("On (73-point, new)")}, {6, N_("On (105-point)")}, {7, N_("On (153-point)")},
};

// Nikon3 Auto Focus Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoAf21_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonAf21Id, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {4, "ContrastDetectAF", N_("Contrast Detect AF"), N_("Contrast detect AF"), IfdId::nikonAf21Id,
     SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonOffOn)},
    {5, "AFAreaMode", N_("AF Area Mode"), N_("AF area mode"), IfdId::nikonAf21Id, SectionId::makerTags, unsignedByte, 1,
     printAf2AreaMode},
    {6, "PhaseDetectAF", N_("Phase Detect AF"), N_("Phase detect AF"), IfdId::nikonAf21Id, SectionId::makerTags,
     unsignedByte, 1, EXV_PRINT_TAG(nikonPhaseDetectAF)},
    {7, "PrimaryAFPoint", N_("Primary AF Point"), N_("Primary AF point"), IfdId::nikonAf21Id, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {8, "AFPointsUsed", N_("AF Points Used"), N_("AF points used"), IfdId::nikonAf21Id, SectionId::makerTags,
     unsignedByte, 7, printValue},
    {16, "AFImageWidth", N_("AF Image Width"), N_("AF image width"), IfdId::nikonAf21Id, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {18, "AFImageHeight", N_("AF Image Height"), N_("AF image height"), IfdId::nikonAf21Id, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {20, "AFAreaXPosition", N_("AF Area X Position"), N_("AF area x position"), IfdId::nikonAf21Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {22, "AFAreaYPosition", N_("AF Area Y Position"), N_("AF area y position"), IfdId::nikonAf21Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {24, "AFAreaWidth", N_("AF Area Width"), N_("AF area width"), IfdId::nikonAf21Id, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {26, "AFAreaHeight", N_("AF Area Height"), N_("AF area height"), IfdId::nikonAf21Id, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {28, "ContrastDetectAFInFocus", N_("Contrast Detect AF In Focus"), N_("Contrast detect AF in focus"),
     IfdId::nikonAf21Id, SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonYesNo)},
    // End of list marker
    {0xffff, "(UnknownNikonAf2Tag)", "(UnknownNikonAf2Tag)", N_("Unknown Nikon Auto Focus 2 Tag"), IfdId::nikonAf21Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListAf21() {
  return tagInfoAf21_;
}

// Nikon3 Auto Focus Tag Info Version 1.01 https://github.com/Exiv2/exiv2/pull/900
constexpr TagInfo Nikon3MakerNote::tagInfoAf22_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonAf22Id, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {4, "ContrastDetectAF", N_("Contrast Detect AF"), N_("Contrast detect AF"), IfdId::nikonAf22Id,
     SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonOffOn)},
    {5, "AFAreaMode", N_("AF Area Mode"), N_("AF area mode"), IfdId::nikonAf22Id, SectionId::makerTags, unsignedByte, 1,
     printValue},
    {6, "PhaseDetectAF", N_("Phase Detect AF"), N_("Phase detect AF"), IfdId::nikonAf22Id, SectionId::makerTags,
     unsignedByte, 1, EXV_PRINT_TAG(nikonPhaseDetectAF)},
    {7, "PrimaryAFPoint", N_("Primary AF Point"), N_("Primary AF point"), IfdId::nikonAf22Id, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {8, "AFPointsUsed", N_("AF Points Used"), N_("AF points used"), IfdId::nikonAf22Id, SectionId::makerTags,
     unsignedByte, 7, printValue},
    {70, "AFImageWidth", N_("AF Image Width"), N_("AF image width"), IfdId::nikonAf22Id, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {72, "AFImageHeight", N_("AF Image Height"), N_("AF image height"), IfdId::nikonAf22Id, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {74, "AFAreaXPosition", N_("AF Area X Position"), N_("AF area x position"), IfdId::nikonAf22Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {76, "AFAreaYPosition", N_("AF Area Y Position"), N_("AF area y position"), IfdId::nikonAf22Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {78, "AFAreaWidth", N_("AF Area Width"), N_("AF area width"), IfdId::nikonAf22Id, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {80, "AFAreaHeight", N_("AF Area Height"), N_("AF area height"), IfdId::nikonAf22Id, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {82, "ContrastDetectAFInFocus", N_("Contrast Detect AF In Focus"), N_("Contrast detect AF in focus"),
     IfdId::nikonAf22Id, SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonYesNo)},
    // End of list marker
    {0xffff, "(UnknownNikonAf2Tag)", "(UnknownNikonAf2Tag)", N_("Unknown Nikon Auto Focus 2 Tag"), IfdId::nikonAf22Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListAf22() {
  return tagInfoAf22_;
}

// Nikon3 File Info Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoFi_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonFiId, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {6, "DirectoryNumber", N_("Directory Number"), N_("Directory number"), IfdId::nikonFiId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {8, "FileNumber", N_("File Number"), N_("File number"), IfdId::nikonFiId, SectionId::makerTags, unsignedShort, 1,
     printValue},
    // End of list marker
    {0xffff, "(UnknownNikonFiTag)", "(UnknownNikonFiTag)", N_("Unknown Nikon File Info Tag"), IfdId::nikonFiId,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListFi() {
  return tagInfoFi_;
}

//! MultiExposureMode
constexpr TagDetails nikonMultiExposureMode[] = {
    {0, N_("Off")},
    {1, N_("Multiple Exposure")},
    {2, N_("Image Overlay")},
};

// Nikon3 Multi Exposure Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoMe_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonMeId, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {4, "MultiExposureMode", N_("Multi Exposure Mode"), N_("Multi exposure mode"), IfdId::nikonMeId,
     SectionId::makerTags, unsignedLong, 1, EXV_PRINT_TAG(nikonMultiExposureMode)},
    {8, "MultiExposureShots", N_("Multi Exposure Shots"), N_("Multi exposure shots"), IfdId::nikonMeId,
     SectionId::makerTags, unsignedLong, 1, printValue},
    {12, "MultiExposureAutoGain", N_("Multi Exposure Auto Gain"), N_("Multi exposure auto gain"), IfdId::nikonMeId,
     SectionId::makerTags, unsignedLong, 1, EXV_PRINT_TAG(nikonOffOn)},
    // End of list marker
    {0xffff, "(UnknownNikonMeTag)", "(UnknownNikonMeTag)", N_("Unknown Nikon Multi Exposure Tag"), IfdId::nikonMeId,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListMe() {
  return tagInfoMe_;
}

//! FlashSource
constexpr TagDetails nikonFlashSource[] = {
    {0, N_("None")},
    {1, N_("External")},
    {2, N_("Internal")},
};

//! FlashFirmware
constexpr TagDetails nikonFlashFirmware[] = {
    {0x0000, N_("n/a")},
    {0x0101, N_("1.01 (SB-800 or Metz 58 AF-1)")},
    {0x0103, "1.03 (SB-800)"},
    {0x0201, "2.01 (SB-800)"},
    {0x0204, "2.04 (SB-600)"},
    {0x0205, "2.05 (SB-600)"},
    {0x0301, "3.01 (SU-800 Remote Commander)"},
    {0x0401, "4.01 (SB-400)"},
    {0x0402, "4.02 (SB-400)"},
    {0x0404, "4.04 (SB-400)"},
    {0x0501, "5.01 (SB-900)"},
    {0x0502, "5.02 (SB-900)"},
    {0x0601, "6.01 (SB-700)"},
    {0x0701, "7.01 (SB-910)"},
    {0x0800, "8.01 (SB-N5)"},
    {0x0a00, "10.00 (SB-N7)"},
    {0x0b00, "11.00 (SB-300)"},
    {0x0d00, "13.00 (SB-500)"},
    {0x0e00, "14.00 (SB-5000)"},
};

//! FlashGNDistance
constexpr TagDetails nikonFlashGNDistance[] = {
    {0, N_("n/a")},   {1, "0.1 m"},     {2, "0.2 m"},     {3, "0.3 m"},     {4, "0.4 m"},     {5, "0.5 m"},
    {6, "0.6 m"},     {7, "0.7 m"},     {8, "0.8 m"},     {9, "0.9 m"},     {10, "1.0 m"},    {11, "1.1 m"},
    {12, "1.3 m"},    {13, "1.4 m"},    {14, "1.6 m"},    {15, "1.8 m"},    {16, "2.0 m"},    {17, "2.2 m"},
    {18, "2.5 m"},    {19, "2.8 m"},    {20, "3.2 m"},    {21, "3.6 m"},    {22, "4.0 m"},    {23, "4.5 m"},
    {24, "5.0 m"},    {25, "5.6 m"},    {26, "6.3 m"},    {27, "7.1 m"},    {28, "8.0 m"},    {29, "9.0 m"},
    {30, "10.0 m"},   {31, "11.0 m"},   {32, "13.0 m"},   {33, "14.0 m"},   {34, "16.0 m"},   {35, "18.0 m"},
    {36, "20.0 m"},   {37, "23.0 m"},   {38, "25.0 m"},   {39, "29.0 m"},   {128, N_("n/a")}, {129, "0.3 ft"},
    {130, "0.7 ft"},  {131, "1.0 ft"},  {132, "1.3 ft"},  {133, "1.7 ft"},  {134, "2.0 ft"},  {135, "2.3 ft"},
    {136, "2.6 ft"},  {137, "2.9 ft"},  {138, "3.3 ft"},  {139, "3.7 ft"},  {140, "4.1 ft"},  {141, "4.6 ft"},
    {142, "5.2 ft"},  {143, "5.8 ft"},  {144, "6.6 ft"},  {145, "7.4 ft"},  {146, "8.3 ft"},  {147, "9.3 ft"},
    {148, "10.0 ft"}, {149, "12.0 ft"}, {150, "13.0 ft"}, {151, "15.0 ft"}, {152, "17.0 ft"}, {153, "19.0 ft"},
    {154, "21.0 ft"}, {155, "23.0 ft"}, {156, "26.0 ft"}, {157, "29.0 ft"}, {158, "33.0 ft"}, {159, "37.0 ft"},
    {160, "42.0 ft"}, {161, "47.0 ft"}, {162, "52.0 ft"}, {163, "59.0 ft"}, {164, "66.0 ft"}, {165, "74.0 ft"},
    {166, "83.0 ft"}, {167, "94.0 ft"}, {255, N_("n/a")},
};

//! FlashControlMode
constexpr TagDetails nikonFlashControlMode[] = {
    {0, N_("Off")},       {1, N_("TTL")},
    {2, N_("iTTL")},      {3, N_("Auto Aperture")},
    {4, N_("Automatic")}, {5, N_("GN (distance priority)")},
    {6, N_("Manual")},    {7, N_("Repeating Flash")},
    // 8: When used with Group A (and possibly Group B), may have an additional "Quick Wireless Flash Control [A:B]"
    //    mode that applies to Groups A and B. May only be available with Radio AWL modes?
    // ?: Possibly includes "iTTL-BL", "i-AA" and "i-A" modes.
};

static constexpr bool flashModeUsesManualScale(const int64_t mode) {
  return (mode == 6 || mode == 7);
}

//! ExternalFlashFlags
constexpr TagDetails nikonExternalFlashFlags[] = {
    {0, N_("Fired")},
    {2, N_("Bounce Flash")},
    {4, N_("Wide Flash Adapter")},
};

//! FlashColorFilter
constexpr TagDetails nikonFlashColorFilter[] = {
    {0, N_("None")},
    {1, N_("FL-GL1 (for fluorescent light)")},
    {2, N_("FL-GL2 (for fluorescent light)")},
    {9, N_("TN-A1 (for incandescent light)")},
    {10, N_("TN-A2 (for incandescent light)")},
    {65, N_("Red")},
    {66, N_("Blue")},
    {67, N_("Yellow")},
    {68, N_("Amber")},
    {79, N_("Other")},
};

//! FlashWirelessOptions
constexpr TagDetails nikonFlashAWLMode[] = {
    {0, N_("AWL Off")},
    {1, N_("Optical AWL")},
    {2, N_("Optical/Radio AWL")},
    {3, N_("Radio AWL")},
};

//! FlashExposureComp
constexpr TagDetails nikonFlashExposureComp[] = {
    {0, N_("Exposure Comp.: Entire frame")},
    {4, N_("Exposure Comp.: Background Only")},
};

//! FlashIlluminationPattern
constexpr TagDetails nikonFlashIlluminationPat[] = {
    {0, N_("Illumination Pat.: Standard")},
    {1, N_("Illumination Pat.: Center-weighted")},
    {2, N_("Illumination Pat.: Even")},
};

//! FlashAdaptors
constexpr TagDetailsBitmask nikonFlashAdaptors[] = {
    {0x04, N_("Bounce Flash adaptor")},
    {0x10, N_("Wide Flash adaptor")},
    {0x20, N_("Nikon Diffusion Dome")},
};

static void printFlashCompensationValue(std::ostream& os, const unsigned char value, const bool manualScale) {
  std::ios::fmtflags f(os.flags());
  std::ostringstream oss;
  oss.copyfmt(os);

  if (manualScale) {
    /*
       // Format:
       0 = "1/1"
       1 = "1/1 (-1/6EV)"
       2 = "1/1 (-1/3EV)"
       3 = "1/1 (-1/2EV)"
       4 = "1/1 (-2/3EV)"
       5 = "1/1 (-5/6EV)"
       6 = "1/2"
       7 = "1/2 (-1/6EV)"
       ...
       48 = "1/256"
    */
    if (value > 48) {
      os << "(" << value << ")";
      os.flags(f);
      return;
    }
    const auto mod = value % 6;
    auto temp = (value < 6) ? 0 : (value - mod) / 6;
    os << "1/" << std::exp2(temp);
    if (mod != 0) {
      os << " (-";
      switch (mod) {
        case 1:
          os << "1/6 EV)";
          break;
        case 2:
          os << "1/3 EV)";
          break;
        case 3:
          os << "1/2 EV)";
          break;
        case 4:
          os << "2/3 EV)";
          break;
        case 5:
          os << "5/6 EV)";
          break;
      }
    }
  } else {
    /*
    // Format uses 127 as boundary between +/- then values descend:
    0   = "0.0EV"
    1   = "-0.2EV"
    2   = "-0.3EV"
    3   = "-0.5EV"
    4   = "-0.7EV"
    5   = "-0.8EV"
    6   = "-1.0EV"
    7   = "-1.2EV"
    ...
    127 = "-21.2EV"
    128 = "+21.3EV"
    129 = "+21.2EV"
    130 = "+21.0EV"
    ...
    255 = "+0.2"
     */
    auto output = 0.0f;
    if (value < 128) {
      if (value != 0)
        output = static_cast<float>(value) * -1.0f;
    } else {
      output = 256.0f - static_cast<float>(value);
    }
    os.precision(1);
    if (value != 0)
      os << std::showpos;
    os << std::fixed << (output / 6) << " EV";
  }
  os.copyfmt(os);
  os.flags(f);
}

// Nikon3 Flash Info 1 Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoFl1_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonFl1Id, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {4, "FlashSource", N_("Flash Source"), N_("Flash source"), IfdId::nikonFl1Id, SectionId::makerTags, unsignedByte, 1,
     EXV_PRINT_TAG(nikonFlashSource)},
    {5, "0x0005", N_("0x0005"), N_("Unknown"), IfdId::nikonFl1Id, SectionId::makerTags, unsignedByte, 1, printValue},
    {6, "ExternalFlashFirmware", N_("External Flash Firmware"), N_("External flash firmware"), IfdId::nikonFl1Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(nikonFlashFirmware)},
    {8, "ExternalFlashFlags", N_("External Flash Flags"), N_("External flash flags"), IfdId::nikonFl1Id,
     SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonExternalFlashFlags)},
    {11, "FlashFocalLength", N_("Flash Focal Length"), N_("Flash focal length"), IfdId::nikonFl1Id,
     SectionId::makerTags, unsignedByte, 1, printFlashFocalLength},
    {12, "RepeatingFlashRate", N_("Repeating Flash Rate"), N_("Repeating flash rate"), IfdId::nikonFl1Id,
     SectionId::makerTags, unsignedByte, 1, printRepeatingFlashRate},
    {13, "RepeatingFlashCount", N_("Repeating Flash Count"), N_("Repeating flash count"), IfdId::nikonFl1Id,
     SectionId::makerTags, unsignedByte, 1, printRepeatingFlashCount},
    {14, "FlashGNDistance", N_("Flash GN Distance"), N_("Flash GN distance"), IfdId::nikonFl1Id, SectionId::makerTags,
     unsignedByte, 1, EXV_PRINT_TAG(nikonFlashGNDistance)},
    {15, "FlashGroupAControlMode", N_("Flash Group A Control Mode"), N_("Flash group a control mode"),
     IfdId::nikonFl1Id, SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonFlashControlMode)},
    {16, "FlashGroupBControlMode", N_("Flash Group B Control Mode"), N_("Flash group b control mode"),
     IfdId::nikonFl1Id, SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonFlashControlMode)},
    // End of list marker
    {0xffff, "(UnknownNikonMeTag)", "(UnknownNikonMeTag)", N_("Unknown Nikon Multi Exposure Tag"), IfdId::nikonFl1Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListFl1() {
  return tagInfoFl1_;
}

// Nikon3 Flash Info 2 Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoFl2_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonFl2Id, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {4, "FlashSource", N_("Flash Source"), N_("Flash source"), IfdId::nikonFl2Id, SectionId::makerTags, unsignedByte, 1,
     EXV_PRINT_TAG(nikonFlashSource)},
    {5, "0x0005", N_("0x0005"), N_("Unknown"), IfdId::nikonFl2Id, SectionId::makerTags, unsignedByte, 1, printValue},
    {6, "ExternalFlashFirmware", N_("External Flash Firmware"), N_("External flash firmware"), IfdId::nikonFl2Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(nikonFlashFirmware)},
    {8, "ExternalFlashFlags", N_("External Flash Flags"), N_("External flash flags"), IfdId::nikonFl2Id,
     SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonExternalFlashFlags)},
    {12, "FlashFocalLength", N_("Flash Focal Length"), N_("Flash focal length"), IfdId::nikonFl2Id,
     SectionId::makerTags, unsignedByte, 1, printFlashFocalLength},
    {13, "RepeatingFlashRate", N_("Repeating Flash Rate"), N_("Repeating flash rate"), IfdId::nikonFl2Id,
     SectionId::makerTags, unsignedByte, 1, printRepeatingFlashRate},
    {14, "RepeatingFlashCount", N_("Repeating Flash Count"), N_("Repeating flash count"), IfdId::nikonFl2Id,
     SectionId::makerTags, unsignedByte, 1, printRepeatingFlashCount},
    {15, "FlashGNDistance", N_("Flash GN Distance"), N_("Flash GN distance"), IfdId::nikonFl2Id, SectionId::makerTags,
     unsignedByte, 1, EXV_PRINT_TAG(nikonFlashGNDistance)},
    // End of list marker
    {0xffff, "(UnknownNikonMeTag)", "(UnknownNikonMeTag)", N_("Unknown Nikon Multi Exposure Tag"), IfdId::nikonFl2Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListFl2() {
  return tagInfoFl2_;
}

// Nikon3 Flash Info 3 Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoFl3_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonFl3Id, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {4, "FlashSource", N_("Flash Source"), N_("Flash source"), IfdId::nikonFl3Id, SectionId::makerTags, unsignedByte, 1,
     EXV_PRINT_TAG(nikonFlashSource)},
    {6, "ExternalFlashFirmware", N_("External Flash Firmware"), N_("External flash firmware"), IfdId::nikonFl3Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(nikonFlashFirmware)},
    {8, "ExternalFlashFlags", N_("External Flash Flags"), N_("External flash flags"), IfdId::nikonFl3Id,
     SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonExternalFlashFlags)},
    {12, "FlashFocalLength", N_("Flash Focal Length"), N_("Flash focal length"), IfdId::nikonFl3Id,
     SectionId::makerTags, unsignedByte, 1, printFlashFocalLength},
    {13, "RepeatingFlashRate", N_("Repeating Flash Rate"), N_("Repeating flash rate"), IfdId::nikonFl3Id,
     SectionId::makerTags, unsignedByte, 1, printRepeatingFlashRate},
    {14, "RepeatingFlashCount", N_("Repeating Flash Count"), N_("Repeating flash count"), IfdId::nikonFl3Id,
     SectionId::makerTags, unsignedByte, 1, printRepeatingFlashCount},
    {15, "FlashGNDistance", N_("Flash GN Distance"), N_("Flash GN distance"), IfdId::nikonFl3Id, SectionId::makerTags,
     unsignedByte, 1, EXV_PRINT_TAG(nikonFlashGNDistance)},
    {16, "FlashColorFilter", N_("Flash Color Filter"), N_("Flash color filter"), IfdId::nikonFl3Id,
     SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonFlashColorFilter)},
    // End of list marker
    {0xffff, "(UnknownNikonMeTag)", "(UnknownNikonMeTag)", N_("Unknown Nikon Multi Exposure Tag"), IfdId::nikonFl3Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListFl3() {
  return tagInfoFl3_;
}

// Nikon3 Flash Info 6 (0106) Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoFl6_[] = {
    {0, "Version", N_("Version"), N_("Identifies the NikonFl version"), IfdId::nikonFl6Id, SectionId::makerTags,
     undefined, 4, printExifVersion},
    {4, "FlashSource", N_("Flash source"), N_("The type of flash used"), IfdId::nikonFl6Id, SectionId::makerTags,
     unsignedByte, 1, EXV_PRINT_TAG(nikonFlashSource)},
    // 5: May be connected to high-speed shutter setting?
    {6, "ExternalFlashFirmware", N_("External flash firmware"), N_("External flash firmware version"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(nikonFlashFirmware)},
    {8, "ExternalFlashData1", N_("External flash data 1"),
     N_("The Speedlight flash status and any attached flash adapters"), IfdId::nikonFl6Id, SectionId::makerTags,
     unsignedByte, 1, printExternalFlashData1Fl6},
    {9, "ExternalFlashData2", N_("External flash data 2"),
     N_("The external flash status and the mode used for the Master flash (see FlashSource)"), IfdId::nikonFl6Id,
     SectionId::makerTags, unsignedByte, 1, printExternalFlashData2Fl6},
    {10, "FlashCompensationMaster", N_("Flash compensation Master"),
     N_("Flash compensation for the Master flash (see FlashSource)"), IfdId::nikonFl6Id, SectionId::makerTags,
     unsignedByte, 1, printFlashMasterDataFl6},
    {12, "FlashFocalLength", N_("Flash focal length"), N_("Flash focal length"), IfdId::nikonFl6Id,
     SectionId::makerTags, unsignedByte, 1, printFlashFocalLength},
    {13, "RepeatingFlashRate", N_("Repeating flash rate"), N_("For repeating flash mode, the rate (in Hz)"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte, 1, printRepeatingFlashRate},
    {14, "RepeatingFlashCount", N_("Repeating flash count"),
     N_("For repeating flash mode, the number of flash firings"), IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte,
     1, printRepeatingFlashCount},
    {15, "FlashGNDistance", N_("Flash GN distance"),
     N_("For GN (distance priority) mode on the Master flash (see FlashSource), the distance to the subject"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonFlashGNDistance)},
    {16, "FlashColorFilter", N_("Flash color filter"), N_("The attached color filter"), IfdId::nikonFl6Id,
     SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonFlashColorFilter)},
    {17, "FlashGroupAControlData", N_("Flash Group A control data"), N_("The mode used for Group A flashes"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupAControlData},
    {18, "FlashGroupBCControlData", N_("Flash Group B/C control data"), N_("The modes used for Group B and C flashes"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupBCControlData},
    {19, "FlashCompensationGroupA", N_("Flash compensation Group A"), N_("Flash compensation for Group A flashes"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupADataFl6},
    {20, "FlashCompensationGroupB", N_("Flash compensation Group B data"), N_("Flash compensation for Group B flashes"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupBDataFl6},
    {21, "FlashCompensationGroupC", N_("Flash Compensation Group C data"), N_("Flash compensation for Group C flashes"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupCDataFl6},
    // 22: Possibly includes nikonFlashExposureComp array?
    {28, "CameraExposureCompensation", N_("Camera exposure compensation"), N_("The camera's exposure compensation"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte, 1, printCameraExposureCompensation},
    {29, "CameraFlashCompensation", N_("Camera flash compensation"), N_("The camera's flash compensation"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte, 1, printCameraExposureCompensation},
    {39, "FlashMasterOutput", N_("Flash Master output"),
     N_("Total flash output for the Master (see FlashSource). FlashMasterOutput = CameraExposureCompensation "
        "(if exposure uses \"Entire frame\" mode and Master uses a non-manual mode) + CameraFlashCompensation (if "
        "Master uses a non-manual mode) + FlashCompensationMaster"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte, 1, printFlashMasterDataFl6},
    {40, "FlashGroupAOutput", N_("Flash Group A output"),
     N_("Total flash output for Group A. FlashGroupAOutput = CameraExposureCompensation (if exposure uses \"Entire "
        "frame\" mode and Group A uses a non-manual mode) + CameraFlashCompensation (if Group A uses a non-manual "
        "mode) + "
        "FlashCompensationGroupA"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupADataFl6},
    {41, "FlashGroupBOutput", N_("Flash Group B output"),
     N_("Total flash output for Group B. FlashGroupBOutput = CameraExposureCompensation (if exposure uses \"Entire "
        "frame\" mode and Group B uses a non-manual mode) + CameraFlashCompensation (if Group B uses a non-manual "
        "mode) + "
        "FlashCompensationGroupB"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupBDataFl6},
    {42, "FlashGroupCOutput", N_("Flash Group C output"),
     N_("Total flash output for Group C. FlashGroupCOutput = CameraExposureCompensation (if exposure uses \"Entire "
        "frame\" mode and Group C uses a non-manual mode) + CameraFlashCompensation (if Group C uses a non-manual "
        "mode) + "
        "FlashCompensationGroupC"),
     IfdId::nikonFl6Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupCDataFl6},
    // End of list marker
    {0xffff, "(UnknownNikonFl6Tag)", "(UnknownNikonFl6Tag)", N_("Unknown Nikon Flash Info 6 Tag"), IfdId::nikonFl6Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListFl6() {
  return tagInfoFl6_;
}

// Nikon3 Flash Info 7 (0107 and 0108) Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoFl7_[] = {
    {0, "Version", N_("Version"), N_("Identifies the NikonFl version"), IfdId::nikonFl7Id, SectionId::makerTags,
     undefined, 4, printExifVersion},
    {4, "FlashSource", N_("Flash source"), N_("The type of flash used"), IfdId::nikonFl7Id, SectionId::makerTags,
     unsignedByte, 1, EXV_PRINT_TAG(nikonFlashSource)},
    // 5: Possibly connected to high-speed shutter setting?
    {6, "ExternalFlashFirmware", N_("External flash firmware"), N_("The firmware version used by the external flash"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(nikonFlashFirmware)},
    {8, "ExternalFlashData1", N_("External flash data 1"),
     N_("The external flash state, zoom status and attached flash adapters"), IfdId::nikonFl7Id, SectionId::makerTags,
     unsignedByte, 1, printExternalFlashData1Fl7},
    {9, "ExternalFlashData2", N_("External flash data 2"), N_("The mode used for the Master flash (see FlashSource)"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printExternalFlashData2},
    {10, "FlashCompensationMaster", N_("Flash compensation Master"),
     N_("Flash compensation for the Master flash (see FlashSource)"), IfdId::nikonFl7Id, SectionId::makerTags,
     unsignedByte, 1, printFlashMasterDataFl7},
    // 12: Purpose of tag is unclear as manual zoom value is found in tag 38. Possibly connected to automatic zoom?
    {12, "FlashFocalLength", N_("Flash focal length"), N_("Flash focal length"), IfdId::nikonFl7Id,
     SectionId::makerTags, unsignedByte, 1, printFlashFocalLength},
    {13, "RepeatingFlashRate", N_("Repeating flash rate"), N_("For repeating flash modes, the rate (in Hz)"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printRepeatingFlashRate},
    {14, "RepeatingFlashCount", N_("Repeating flash count"),
     N_("For repeating flash modes, the number of flash firings"), IfdId::nikonFl7Id, SectionId::makerTags,
     unsignedByte, 1, printRepeatingFlashCount},
    {15, "FlashGNDistance", N_("Flash GN distance"),
     N_("For GN (distance priority) mode on the Master flash (see FlashSource), the distance to the subject"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonFlashGNDistance)},
    {16, "FlashColorFilter", N_("Flash color filter"), N_("The attached color filters"), IfdId::nikonFl7Id,
     SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonFlashColorFilter)},
    // 17: Possibly also contains control data for Group D in upper nibble?
    {17, "FlashGroupAControlData", N_("Flash Group A control data"), N_("The mode used for Group A flashes"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupAControlData},
    {18, "FlashGroupBCControlData", N_("Flash Groups B/C control data"), N_("The modes used for Group B and C flashes"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupBCControlData},
    {19, "FlashCompensationGroupA", N_("Flash compensation Group A"), N_("Flash compensation for Group A flashes"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupADataFl7},
    {20, "FlashCompensationGroupB", N_("Flash compensation Group B data"), N_("Flash compensation for Group B flashes"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupBDataFl7},
    {21, "FlashCompensationGroupC", N_("Flash Compensation Group C data"), N_("Flash compensation for Group C flashes"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupCDataFl7},
    {22, "ExternalFlashData3", N_("External flash data 3"),
     N_("For the camera's exposure compensation, defines how the flash level is adjusted"), IfdId::nikonFl7Id,
     SectionId::makerTags, unsignedByte, 1, printExternalFlashData3},
    {27, "CameraFlashOutput", N_("Camera flash output"),
     N_("Total flash output for the camera. CameraFlashOutput = CameraExposureCompensation (if ExternalFlashData3 "
        "uses \"Entire frame\" mode) + CameraFlashCompensation"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printCameraExposureCompensation},
    {28, "CameraExposureCompensation", N_("Camera exposure compensation"), N_("The camera's exposure compensation"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printCameraExposureCompensation},
    {29, "CameraFlashCompensation", N_("Camera flash compensation"), N_("The camera's flash compensation"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printCameraExposureCompensation},
    // 34: Possible that FV Lock sets '0100 0000'?
    {37, "ExternalFlashData4", N_("External flash data 4"), N_("The flash illumination pattern"), IfdId::nikonFl7Id,
     SectionId::makerTags, unsignedByte, 1, printExternalFlashData4},
    {38, "FlashZoomHeadPosition", N_("Flash zoom head position"), N_("Flash zoom head position (in mm)"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printFlashZoomHeadPosition},
    {39, "FlashMasterOutput", N_("Flash Master output"),
     N_("Total flash output for the Master (see FlashSource). FlashMasterOutput = CameraFlashOutput (if Master uses a "
        "non-manual mode) + "
        "FlashCompensationMaster"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printFlashMasterDataFl7},
    {40, "FlashGroupAOutput", N_("Flash Group A output"),
     N_("Total flash output for Group A. FlashGroupAOutput = CameraFlashOutput (if Group A uses a non-manual mode) + "
        "FlashCompensationGroupA"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupADataFl7},
    {41, "FlashGroupBOutput", N_("Flash Group B output"),
     N_("Total flash output for Group B. FlashGroupBOutput = CameraFlashOutput (if Group B uses a non-manual mode) + "
        "FlashCompensationGroupB"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupBDataFl7},
    {42, "FlashGroupCOutput", N_("Flash Group C output"),
     N_("Total flash output for Group C. FlashGroupCOutput = CameraFlashOutput (if Group C uses a non-manual mode) + "
        "FlashCompensationGroupC"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, printFlashGroupCDataFl7},
    // 43-46: Each corresponds to M/A/B/C - set when using TTL modes, otherwise is zero. Could be connected to the
    //        monitor pre-flashes?
    {67, "WirelessFlashData", N_("Wireless flash data"), N_("Advanced Wireless Lighting (AWL) communication mode"),
     IfdId::nikonFl7Id, SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonFlashAWLMode)},
    // 68-72: Related to WirelessFlashData? Set if AWL is on, zero if AWL is off. 68-71 connected to M/A/B/C?
    // ?:     1. Groups D/E/F will have equivalent tags to Groups A/B/C (see note in tag 17). This could also include
    //           equivalent tags in 43-46?
    //        2. Compatible remote flashes using 'Radio AWL' could transmit settings/data back to the Speedflash?
    //        3. Automatic flash mode (A) uses an aperture value.
    //        4. The Speedlight can detect if the Master is not pointed straight ahead. The vertical and horizontal
    //           angles/positions of the flash could be recorded?
    // End of list marker
    {0xffff, "(UnknownNikonFl7Tag)", "(UnknownNikonFl7Tag)", N_("Unknown Nikon Flash Info 7 Tag"), IfdId::nikonFl7Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListFl7() {
  return tagInfoFl7_;
}

// Nikon3 Shot Info D80 Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoSi1_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonSi1Id, SectionId::makerTags, unsignedByte, 4,
     printExifVersion},
    {586, "ShutterCount", N_("Shutter Count"), N_("Shutter count"), IfdId::nikonSi1Id, SectionId::makerTags,
     unsignedLong, 1, printValue},
    // End of list marker
    {0xffff, "(UnknownNikonSi1Tag)", "(UnknownNikonSi1Tag)", N_("Unknown Nikon Shot Info D80 Tag"), IfdId::nikonSi1Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListSi1() {
  return tagInfoSi1_;
}

// Nikon3 Shot Info D40 Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoSi2_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonSi2Id, SectionId::makerTags, unsignedByte, 4,
     printExifVersion},
    {582, "ShutterCount", N_("Shutter Count"), N_("Shutter count"), IfdId::nikonSi2Id, SectionId::makerTags,
     unsignedLong, 1, printValue},
    {738, "FlashLevel", N_("Flash Level"), N_("Flash level"), IfdId::nikonSi2Id, SectionId::makerTags, unsignedByte, 1,
     printValue},
    // End of list marker
    {0xffff, "(UnknownNikonSi2Tag)", "(UnknownNikonSi2Tag)", N_("Unknown Nikon Shot Info D40 Tag"), IfdId::nikonSi2Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListSi2() {
  return tagInfoSi2_;
}

//! AfFineTuneAdj D300 (a)
constexpr TagDetails nikonAfFineTuneAdj1[] = {
    {0x0000, "0"},   {0x003a, "+1"},  {0x003b, "+2"},  {0x003c, "+4"},  {0x003d, "+8"},  {0x003e, "+16"},
    {0x00c2, "-16"}, {0x00c3, "-8"},  {0x00c4, "-4"},  {0x00c5, "-2"},  {0x00c6, "-1"},  {0x103e, "+17"},
    {0x10c2, "-17"}, {0x203d, "+9"},  {0x203e, "+18"}, {0x20c2, "-18"}, {0x20c3, "-9"},  {0x303e, "+19"},
    {0x30c2, "-19"}, {0x403c, "+5"},  {0x403d, "+10"}, {0x403e, "+20"}, {0x40c2, "-20"}, {0x40c3, "-10"},
    {0x40c4, "-5"},  {0x603d, "+11"}, {0x60c3, "-11"}, {0x803b, "+3"},  {0x803c, "+6"},  {0x803d, "+12"},
    {0x80c3, "-12"}, {0x80c4, "-6"},  {0x80c5, "-3"},  {0xa03d, "+13"}, {0xa0c3, "-13"}, {0xc03c, "+7"},
    {0xc03d, "+14"}, {0xc0c3, "-14"}, {0xc0c4, "-7"},  {0xe03d, "+15"}, {0xe0c3, "-15"},
};

// Nikon3 Shot Info D300 (a) Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoSi3_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonSi3Id, SectionId::makerTags, unsignedByte, 4,
     printExifVersion},
    {604, "ISO", N_("ISO"), N_("ISO"), IfdId::nikonSi3Id, SectionId::makerTags, unsignedByte, 1, printIiIso},
    {633, "ShutterCount", N_("Shutter Count"), N_("Shutter count"), IfdId::nikonSi3Id, SectionId::makerTags,
     unsignedLong, 1, printValue},
    {721, "AFFineTuneAdj", N_("AF Fine Tune Adj"), N_("AF fine tune adj"), IfdId::nikonSi3Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(nikonAfFineTuneAdj1)},
    // End of list marker
    {0xffff, "(UnknownNikonSi3Tag)", "(UnknownNikonSi3Tag)", N_("Unknown Nikon Shot Info D300 (a) Tag"),
     IfdId::nikonSi3Id, SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListSi3() {
  return tagInfoSi3_;
}

//! AfFineTuneAdj D300 (b)
constexpr TagDetails nikonAfFineTuneAdj2[] = {
    {0x0000, "0"},   {0x043e, "+13"}, {0x04c2, "-13"}, {0x183d, "+7"},  {0x183e, "+14"}, {0x18c2, "-14"},
    {0x18c3, "-7"},  {0x2c3e, "+15"}, {0x2cc2, "-15"}, {0x403a, "+1"},  {0x403b, "+2"},  {0x403c, "+4"},
    {0x403d, "+8"},  {0x403e, "+16"}, {0x40c2, "-16"}, {0x40c3, "-8"},  {0x40c4, "-4"},  {0x40c5, "-2"},
    {0x40c6, "-1"},  {0x543e, "+17"}, {0x54c2, "-17"}, {0x683d, "+9"},  {0x683e, "+18"}, {0x68c2, "-18"},
    {0x68c3, "-9"},  {0x7c3e, "+19"}, {0x7cc2, "-19"}, {0x903c, "+5"},  {0x903d, "+10"}, {0x903e, "+20"},
    {0x90c2, "-20"}, {0x90c3, "-10"}, {0x90c4, "-5"},  {0xb83d, "+11"}, {0xb8c3, "-11"}, {0xe03b, "+3"},
    {0xe03c, "+6"},  {0xe03d, "+12"}, {0xe0c3, "-12"}, {0xe0c4, "-6"},  {0xe0c5, "-3"},
};

// Nikon3 Shot Info D300 (b) Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoSi4_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonSi4Id, SectionId::makerTags, unsignedByte, 4,
     printExifVersion},
    {613, "ISO", N_("ISO"), N_("ISO"), IfdId::nikonSi4Id, SectionId::makerTags, unsignedByte, 1, printIiIso},
    {644, "ShutterCount", N_("Shutter Count"), N_("Shutter count"), IfdId::nikonSi4Id, SectionId::makerTags,
     unsignedLong, 1, printValue},
    {732, "AFFineTuneAdj", N_("AF Fine Tune Adj"), N_("AF fine tune adj"), IfdId::nikonSi4Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(nikonAfFineTuneAdj2)},
    // End of list marker
    {0xffff, "(UnknownNikonSi4Tag)", "(UnknownNikonSi4Tag)", N_("Unknown Nikon Shot Info D300 (b) Tag"),
     IfdId::nikonSi4Id, SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListSi4() {
  return tagInfoSi4_;
}

//! VibrationReduction
constexpr TagDetails nikonOffOn2[] = {
    {0, N_("Off")},
    {1, N_("On (1)")},
    {2, N_("On (2)")},
    {3, N_("On (3)")},
};

//! VibrationReduction2
constexpr TagDetails nikonOffOn3[] = {
    {0x0, N_("n/a")},
    {0xc, N_("Off")},
    {0xf, N_("On")},
};

// Nikon3 Shot Info Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoSi5_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonSi5Id, SectionId::makerTags, unsignedByte, 4,
     printExifVersion},
    {106, "ShutterCount1", N_("Shutter Count 1"), N_("Shutter count 1"), IfdId::nikonSi5Id, SectionId::makerTags,
     unsignedLong, 1, printValue},
    {110, "DeletedImageCount", N_("Deleted Image Count"), N_("Deleted image count"), IfdId::nikonSi5Id,
     SectionId::makerTags, unsignedLong, 1, printValue},
    {117, "VibrationReduction", N_("Vibration Reduction"), N_("Vibration reduction"), IfdId::nikonSi5Id,
     SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonOffOn2)},
    {130, "VibrationReduction1", N_("Vibration Reduction 1"), N_("Vibration reduction 1"), IfdId::nikonSi5Id,
     SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonOffOn)},
    {343, "ShutterCount2", N_("Shutter Count 2"), N_("Shutter count 2"), IfdId::nikonSi5Id, SectionId::makerTags,
     undefined, 2, printValue},
    {430, "VibrationReduction2", N_("Vibration Reduction 2"), N_("Vibration reduction 2"), IfdId::nikonSi5Id,
     SectionId::makerTags, unsignedByte, 1, EXV_PRINT_TAG(nikonOffOn3)},
    {598, "ISO", N_("ISO"), N_("ISO"), IfdId::nikonSi5Id, SectionId::makerTags, unsignedByte, 1, printIiIso},
    {630, "ShutterCount", N_("Shutter Count"), N_("Shutter count"), IfdId::nikonSi5Id, SectionId::makerTags,
     unsignedLong, 1, printValue},
    // End of list marker
    {0xffff, "(UnknownNikonSi5Tag)", "(UnknownNikonSi5Tag)", N_("Unknown Nikon Shot Info Tag"), IfdId::nikonSi5Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListSi5() {
  return tagInfoSi5_;
}

// Nikon3 Color Balance 1 Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoCb1_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonCb1Id, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {36, "WB_RBGGLevels", N_("WB RBGG Levels"), N_("WB RBGG levels"), IfdId::nikonCb1Id, SectionId::makerTags,
     unsignedShort, 4, printValue},
    // End of list marker
    {0xffff, "(UnknownNikonCb1Tag)", "(UnknownNikonCb1Tag)", N_("Unknown Nikon Color Balance 1 Tag"), IfdId::nikonCb1Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListCb1() {
  return tagInfoCb1_;
}

// Nikon3 Color Balance 2 Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoCb2_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonCb2Id, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {5, "WB_RGGBLevels", N_("WB RGGB Levels"), N_("WB RGGB levels"), IfdId::nikonCb2Id, SectionId::makerTags,
     unsignedShort, 4, printValue},
    // End of list marker
    {0xffff, "(UnknownNikonCb2Tag)", "(UnknownNikonCb2Tag)", N_("Unknown Nikon Color Balance 2 Tag"), IfdId::nikonCb2Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListCb2() {
  return tagInfoCb2_;
}

// Nikon3 Color Balance 2a Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoCb2a_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonCb2aId, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {9, "WB_RGGBLevels", N_("WB RGGB Levels"), N_("WB RGGB levels"), IfdId::nikonCb2aId, SectionId::makerTags,
     unsignedShort, 4, printValue},
    // End of list marker
    {0xffff, "(UnknownNikonCb2aTag)", "(UnknownNikonCb2aTag)", N_("Unknown Nikon Color Balance 2a Tag"),
     IfdId::nikonCb2aId, SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListCb2a() {
  return tagInfoCb2a_;
}

// Nikon3 Color Balance 2b Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoCb2b_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonCb2bId, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {145, "WB_RGGBLevels", N_("WB RGGB Levels"), N_("WB RGGB levels"), IfdId::nikonCb2bId, SectionId::makerTags,
     unsignedShort, 4, printValue},
    // End of list marker
    {0xffff, "(UnknownNikonCb2bTag)", "(UnknownNikonCb2bTag)", N_("Unknown Nikon Color Balance 2b Tag"),
     IfdId::nikonCb2bId, SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListCb2b() {
  return tagInfoCb2b_;
}

// Nikon3 Color Balance 3 Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoCb3_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonCb3Id, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {10, "WB_RGBGLevels", N_("WB RGBG Levels"), N_("WB RGBG levels"), IfdId::nikonCb3Id, SectionId::makerTags,
     unsignedShort, 4, printValue},
    // End of list marker
    {0xffff, "(UnknownNikonCb3Tag)", "(UnknownNikonCb3Tag)", N_("Unknown Nikon Color Balance 3 Tag"), IfdId::nikonCb3Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListCb3() {
  return tagInfoCb3_;
}

// Nikon3 Color Balance 4 Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoCb4_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonCb4Id, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {147, "WB_GRBGLevels", N_("WB GRBG Levels"), N_("WB GRBG levels"), IfdId::nikonCb4Id, SectionId::makerTags,
     unsignedShort, 4, printValue},
    // End of list marker
    {0xffff, "(UnknownNikonCb4Tag)", "(UnknownNikonCb4Tag)", N_("Unknown Nikon Color Balance 4 Tag"), IfdId::nikonCb4Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListCb4() {
  return tagInfoCb4_;
}

// Nikon3 Lens Data 1 Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoLd1_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonLd1Id, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {6, "LensIDNumber", N_("Lens ID Number"), N_("Lens ID number"), IfdId::nikonLd1Id, SectionId::makerTags,
     unsignedByte, 1, printLensId1},
    {7, "LensFStops", N_("Lens F-Stops"), N_("Lens F-stops"), IfdId::nikonLd1Id, SectionId::makerTags, unsignedByte, 1,
     printFStops},
    {8, "MinFocalLength", N_("Min Focal Length"), N_("Min focal length"), IfdId::nikonLd1Id, SectionId::makerTags,
     unsignedByte, 1, printFocal},
    {9, "MaxFocalLength", N_("Max Focal Length"), N_("Max focal length"), IfdId::nikonLd1Id, SectionId::makerTags,
     unsignedByte, 1, printFocal},
    {10, "MaxApertureAtMinFocal", N_("Max Aperture At Min Focal"), N_("Max aperture at min focal"), IfdId::nikonLd1Id,
     SectionId::makerTags, unsignedByte, 1, printAperture},
    {11, "MaxApertureAtMaxFocal", N_("Max Aperture At Max Focal"), N_("Max aperture at max focal"), IfdId::nikonLd1Id,
     SectionId::makerTags, unsignedByte, 1, printAperture},
    {12, "MCUVersion", N_("MCU Version"), N_("MCU version"), IfdId::nikonLd1Id, SectionId::makerTags, unsignedByte, 1,
     printValue},
    // End of list marker
    {0xffff, "(UnknownNikonLd1Tag)", "(UnknownNikonLd1Tag)", N_("Unknown Nikon Lens Data 1 Tag"), IfdId::nikonLd1Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListLd1() {
  return tagInfoLd1_;
}

// Nikon3 Lens Data 2 Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoLd2_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonLd2Id, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {4, "ExitPupilPosition", N_("Exit Pupil Position"), N_("Exit pupil position"), IfdId::nikonLd2Id,
     SectionId::makerTags, unsignedByte, 1, printExitPupilPosition},
    {5, "AFAperture", N_("AF Aperture"), N_("AF aperture"), IfdId::nikonLd2Id, SectionId::makerTags, unsignedByte, 1,
     printAperture},
    {8, "FocusPosition", N_("Focus Position"), N_("Focus position"), IfdId::nikonLd2Id, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {9, "FocusDistance", N_("Focus Distance"), N_("Focus distance"), IfdId::nikonLd2Id, SectionId::makerTags,
     unsignedByte, 1, printFocusDistance},
    {10, "FocalLength", N_("Focal Length"), N_("Focal length"), IfdId::nikonLd2Id, SectionId::makerTags, unsignedByte,
     1, printFocal},
    {11, "LensIDNumber", N_("Lens ID Number"), N_("Lens ID number"), IfdId::nikonLd2Id, SectionId::makerTags,
     unsignedByte, 1, printLensId2},
    {12, "LensFStops", N_("Lens F-Stops"), N_("Lens F-stops"), IfdId::nikonLd2Id, SectionId::makerTags, unsignedByte, 1,
     printFStops},
    {13, "MinFocalLength", N_("Min Focal Length"), N_("Min focal length"), IfdId::nikonLd2Id, SectionId::makerTags,
     unsignedByte, 1, printFocal},
    {14, "MaxFocalLength", N_("Max Focal Length"), N_("Max focal length"), IfdId::nikonLd2Id, SectionId::makerTags,
     unsignedByte, 1, printFocal},
    {15, "MaxApertureAtMinFocal", N_("Max Aperture At Min Focal"), N_("Max aperture at min focal"), IfdId::nikonLd2Id,
     SectionId::makerTags, unsignedByte, 1, printAperture},
    {16, "MaxApertureAtMaxFocal", N_("Max Aperture At Max Focal"), N_("Max aperture at max focal"), IfdId::nikonLd2Id,
     SectionId::makerTags, unsignedByte, 1, printAperture},
    {17, "MCUVersion", N_("MCU Version"), N_("MCU version"), IfdId::nikonLd2Id, SectionId::makerTags, unsignedByte, 1,
     printValue},
    {18, "EffectiveMaxAperture", N_("Effective Max Aperture"), N_("Effective max aperture"), IfdId::nikonLd2Id,
     SectionId::makerTags, unsignedByte, 1, printAperture},
    // End of list marker
    {0xffff, "(UnknownNikonLd2Tag)", "(UnknownNikonLd2Tag)", N_("Unknown Nikon Lens Data 2 Tag"), IfdId::nikonLd2Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListLd2() {
  return tagInfoLd2_;
}

// Nikon3 Lens Data 3 Tag Info
constexpr TagInfo Nikon3MakerNote::tagInfoLd3_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonLd3Id, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {4, "ExitPupilPosition", N_("Exit Pupil Position"), N_("Exit pupil position"), IfdId::nikonLd3Id,
     SectionId::makerTags, unsignedByte, 1, printExitPupilPosition},
    {5, "AFAperture", N_("AF Aperture"), N_("AF aperture"), IfdId::nikonLd3Id, SectionId::makerTags, unsignedByte, 1,
     printAperture},
    {8, "FocusPosition", N_("Focus Position"), N_("Focus position"), IfdId::nikonLd3Id, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {10, "FocusDistance", N_("Focus Distance"), N_("Focus distance"), IfdId::nikonLd3Id, SectionId::makerTags,
     unsignedByte, 1, printFocusDistance},
    {11, "FocalLength", N_("Focal Length"), N_("Focal length"), IfdId::nikonLd3Id, SectionId::makerTags, unsignedByte,
     1, printFocal},
    {12, "LensIDNumber", N_("Lens ID Number"), N_("Lens ID number"), IfdId::nikonLd3Id, SectionId::makerTags,
     unsignedByte, 1, printLensId3},
    {13, "LensFStops", N_("Lens F-Stops"), N_("Lens F-stops"), IfdId::nikonLd3Id, SectionId::makerTags, unsignedByte, 1,
     printFStops},
    {14, "MinFocalLength", N_("Min Focal Length"), N_("Min focal length"), IfdId::nikonLd3Id, SectionId::makerTags,
     unsignedByte, 1, printFocal},
    {15, "MaxFocalLength", N_("Max Focal Length"), N_("Max focal length"), IfdId::nikonLd3Id, SectionId::makerTags,
     unsignedByte, 1, printFocal},
    {16, "MaxApertureAtMinFocal", N_("Max Aperture At Min Focal"), N_("Max aperture at min focal length"),
     IfdId::nikonLd3Id, SectionId::makerTags, unsignedByte, 1, printAperture},
    {17, "MaxApertureAtMaxFocal", N_("Max Aperture At Max Focal"), N_("Max aperture at max focal length"),
     IfdId::nikonLd3Id, SectionId::makerTags, unsignedByte, 1, printAperture},
    {18, "MCUVersion", N_("MCU Version"), N_("MCU version"), IfdId::nikonLd3Id, SectionId::makerTags, unsignedByte, 1,
     printValue},
    {19, "EffectiveMaxAperture", N_("Effective Max Aperture"), N_("Effective max aperture"), IfdId::nikonLd3Id,
     SectionId::makerTags, unsignedByte, 1, printAperture},
    // End of list marker
    {0xffff, "(UnknownNikonLd3Tag)", "(UnknownNikonLd3Tag)", N_("Unknown Nikon Lens Data 3 Tag"), IfdId::nikonLd3Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListLd3() {
  return tagInfoLd3_;
}

//! LensID, tag index 48
// see https://github.com/exiftool/exiftool/blob/13.16/lib/Image/ExifTool/Nikon.pm#L5668
constexpr TagDetails nikonZMountLensId[] = {
    {0, N_("n/a")},
    {1, "Nikon Nikkor Z 24-70mm f/4 S"},
    {2, "Nikon Nikkor Z 14-30mm f/4 S"},
    {4, "Nikon Nikkor Z 35mm f/1.8 S"},
    {8, "Nikon Nikkor Z 58mm f/0.95 S Noct"},  // IB
    {9, "Nikon Nikkor Z 50mm f/1.8 S"},
    {11, "Nikon Nikkor Z DX 16-50mm f/3.5-6.3 VR"},
    {12, "Nikon Nikkor Z DX 50-250mm f/4.5-6.3 VR"},
    {13, "Nikon Nikkor Z 24-70mm f/2.8 S"},
    {14, "Nikon Nikkor Z 85mm f/1.8 S"},
    {15, "Nikon Nikkor Z 24mm f/1.8 S"},              // IB
    {16, "Nikon Nikkor Z 70-200mm f/2.8 VR S"},       // IB
    {17, "Nikon Nikkor Z 20mm f/1.8 S"},              // IB
    {18, "Nikon Nikkor Z 24-200mm f/4-6.3 VR"},       // IB
    {21, "Nikon Nikkor Z 50mm f/1.2 S"},              // IB
    {22, "Nikon Nikkor Z 24-50mm f/4-6.3"},           // IB
    {23, "Nikon Nikkor Z 14-24mm f/2.8 S"},           // IB
    {24, "Nikon Nikkor Z MC 105mm f/2.8 VR S"},       // IB
    {25, "Nikon Nikkor Z 40mm f/2"},                  // 28
    {26, "Nikon Nikkor Z DX 18-140mm f/3.5-6.3 VR"},  // IB
    {27, "Nikon Nikkor Z MC 50mm f/2.8"},             // IB
    {28, "Nikon Nikkor Z 100-400mm f/4.5-5.6 VR S"},  // 28
    {29, "Nikon Nikkor Z 28mm f/2.8"},                // IB
    {30, "Nikon Nikkor Z 400mm f/2.8 TC VR S"},       // 28
    {31, "Nikon Nikkor Z 24-120mm f/4 S"},            // 28
    {32, "Nikon Nikkor Z 800mm f/6.3 VR S"},          // 28
    {35, "Nikon Nikkor Z 28-75mm f/2.8"},             // IB
    {36, "Nikon Nikkor Z 400mm f/4.5 VR S"},          // IB
    {37, "Nikon Nikkor Z 600mm f/4 TC VR S"},         // 28
    {38, "Nikon Nikkor Z 85mm f/1.2 S"},              // 28
    {39, "Nikon Nikkor Z 17-28mm f/2.8"},             // IB
    {40, "Nikon Nikkor Z 26mm f/2.8"},
    {41, "Nikon Nikkor Z DX 12-28mm f/3.5-5.6 PZ VR"},
    {42, "Nikon Nikkor Z 180-600mm f/5.6-6.3 VR"},
    {43, "Nikon Nikkor Z DX 24mm f/1.7"},
    {44, "Nikon Nikkor Z 70-180mm f/2.8"},
    {45, "Nikon Nikkor Z 600mm f/6.3 VR S"},
    {46, "Nikon Nikkor Z 135mm f/1.8 S Plena"},
    {47, "Nikon Nikkor Z 35mm f/1.2 S"},
    {48, "Nikon Nikkor Z 28-400mm f/4-8 VR"},
    {49, "Nikon Nikkor Z 28-135mm f/4 PZ"},
    {51, "Nikon Nikkor Z 35mm f/1.4"},
    {52, "Nikon Nikkor Z 50mm f/1.4"},
    {2305, "Laowa FFII 10mm F2.8 C&D Dreamer"},
    {53251, "Sigma 56mm F1.4 DC DN | C"},
    {57346, "Tamron 35-150mm F/2-2.8 Di III VXD"},
};

// Nikon3 Lens Data 4 Tag Info
// based on https://exiftool.org/TagNames/Nikon.html#LensData0800
constexpr TagInfo Nikon3MakerNote::tagInfoLd4_[] = {
    {0, "Version", N_("Version"), N_("Version"), IfdId::nikonLd4Id, SectionId::makerTags, undefined, 4,
     printExifVersion},
    {4, "ExitPupilPosition", N_("Exit Pupil Position"), N_("Exit pupil position"), IfdId::nikonLd4Id,
     SectionId::makerTags, unsignedByte, 1, printExitPupilPosition},
    {5, "AFAperture", N_("AF Aperture"), N_("AF aperture"), IfdId::nikonLd4Id, SectionId::makerTags, unsignedByte, 1,
     printAperture},
    {9, "FocusPosition", N_("Focus Position"), N_("Focus position"), IfdId::nikonLd4Id, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {11, "FocusDistance", N_("Focus Distance"), N_("Focus distance"), IfdId::nikonLd4Id, SectionId::makerTags,
     unsignedByte, 1, printFocusDistance},
    {12, "FocalLength", N_("Focal Length"), N_("Focal length"), IfdId::nikonLd4Id, SectionId::makerTags, unsignedByte,
     1, printFocal},
    {13, "LensIDNumber", N_("Lens ID Number"), N_("Lens ID number"), IfdId::nikonLd4Id, SectionId::makerTags,
     unsignedByte, 1, printLensId4},
    {14, "LensFStops", N_("Lens F-Stops"), N_("Lens F-stops"), IfdId::nikonLd4Id, SectionId::makerTags, unsignedByte, 1,
     printFStops},
    {15, "MinFocalLength", N_("Min Focal Length"), N_("Min focal length"), IfdId::nikonLd4Id, SectionId::makerTags,
     unsignedByte, 1, printFocal},
    {16, "MaxFocalLength", N_("Max Focal Length"), N_("Max focal length"), IfdId::nikonLd4Id, SectionId::makerTags,
     unsignedByte, 1, printFocal},
    {17, "MaxApertureAtMinFocal", N_("Max Aperture At Min Focal"), N_("Max aperture at min focal length"),
     IfdId::nikonLd4Id, SectionId::makerTags, unsignedByte, 1, printAperture},
    {18, "MaxApertureAtMaxFocal", N_("Max Aperture At Max Focal"), N_("Max aperture at max focal length"),
     IfdId::nikonLd4Id, SectionId::makerTags, unsignedByte, 1, printAperture},
    {19, "MCUVersion", N_("MCU Version"), N_("MCU version"), IfdId::nikonLd4Id, SectionId::makerTags, unsignedByte, 1,
     printValue},
    {20, "EffectiveMaxAperture", N_("Effective Max Aperture"), N_("Effective max aperture"), IfdId::nikonLd4Id,
     SectionId::makerTags, unsignedByte, 1, printAperture},
    {48, "LensID", N_("LensID"), N_("Lens ID"), IfdId::nikonLd4Id, SectionId::makerTags, unsignedShort, 1,
     EXV_PRINT_TAG(nikonZMountLensId)},
    {54, "MaxAperture", N_("Max Aperture"), N_("Max aperture"), IfdId::nikonLd4Id, SectionId::makerTags, unsignedShort,
     1, printApertureLd4},
    {56, "FNumber", N_("F-Number"), N_("F-Number"), IfdId::nikonLd4Id, SectionId::makerTags, unsignedShort, 1,
     printApertureLd4},
    {60, "FocalLength2", N_("Focal Length 2"), N_("Focal length 2"), IfdId::nikonLd4Id, SectionId::makerTags,
     unsignedShort, 1, printFocalLd4},
    {78, "FocusDistance2", N_("Focus Distance 2"), N_("Focus distance 2"), IfdId::nikonLd4Id, SectionId::makerTags,
     unsignedShort, 1, printFocusDistanceLd4},
    // End of list marker
    {0xffff, "(UnknownNikonLd4Tag)", "(UnknownNikonLd4Tag)", N_("Unknown Nikon Lens Data 3 Tag"), IfdId::nikonLd4Id,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* Nikon3MakerNote::tagListLd4() {
  return tagInfoLd4_;
}

std::ostream& Nikon3MakerNote::printIiIso(std::ostream& os, const Value& value, const ExifData*) {
  auto v = std::lround(100.0 * std::exp2((value.toInt64() / 12.0) - 5));
  return os << v;
}

std::ostream& Nikon3MakerNote::print0x0002(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() > 1) {
    os << value.toInt64(1);
  } else {
    os << "(" << value << ")";
  }
  return os;
}

std::ostream& Nikon3MakerNote::printAf2AreaMode(std::ostream& os, const Value& value, const ExifData* metadata) {
  int contrastDetectAF = 0;
  if (metadata) {
    auto pos = metadata->findKey(ExifKey("Exif.NikonAf2.ContrastDetectAF"));
    if (pos != metadata->end() && pos->count() != 0) {
      contrastDetectAF = pos->toUint32();
    }
  }

  if (contrastDetectAF == 0)
    return EXV_PRINT_TAG(nikonAf2AreaModeContrastDetectAfOff)(os, value, nullptr);
  return EXV_PRINT_TAG(nikonAf2AreaModeContrastDetectAfOn)(os, value, nullptr);
}

std::ostream& Nikon3MakerNote::print0x0007(std::ostream& os, const Value& value, const ExifData*) {
  std::string focus = value.toString();
  if (focus == "AF-C  ")
    os << _("Continuous autofocus");
  else if (focus == "AF-S  ")
    os << _("Single autofocus");
  else if (focus == "AF-A  ")
    os << _("Automatic");
  else
    os << "(" << value << ")";
  return os;
}

std::ostream& Nikon3MakerNote::print0x0083(std::ostream& os, const Value& value, const ExifData*) {
  const uint32_t lensType = value.toUint32();

  bool valid = false;
  if (lensType & 1) {
    os << "MF ";
    valid = true;
  }
  if (lensType & 2) {
    os << "D ";
    valid = true;
  }
  if (lensType & 4) {
    os << "G ";
    valid = true;
  }
  if (lensType & 8) {
    os << "VR";
    valid = true;
  }

  if (!valid)
    os << "(" << lensType << ")";

  return os;
}

std::ostream& Nikon3MakerNote::print0x0084(std::ostream& os, const Value& value, const ExifData*) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 4 || value.toRational(0).second == 0 || value.toRational(1).second == 0) {
    os << "(" << value << ")";
    return os;
  }
  const int64_t len1 = value.toInt64(0);
  const int64_t len2 = value.toInt64(1);

  auto [r1, s1] = value.toRational(2);
  auto [r2, s2] = value.toRational(3);
  os << len1;
  if (len2 != len1) {
    os << "-" << len2;
  }
  os << "mm ";
  std::ostringstream oss;
  oss.copyfmt(os);
  os << "F" << std::setprecision(2) << static_cast<float>(r1) / s1;
  if (r2 != r1) {
    os << "-" << std::setprecision(2) << static_cast<float>(r2) / s2;
  }
  os.copyfmt(oss);
  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::print0x0085(std::ostream& os, const Value& value, const ExifData*) {
  auto [r, s] = value.toRational();
  if (r == 0)
    return os << _("Unknown");
  if (s == 0)
    return os << "(" << value << ")";
  return os << stringFormat("{:.2f} m", static_cast<float>(r) / s);
}

std::ostream& Nikon3MakerNote::print0x0086(std::ostream& os, const Value& value, const ExifData*) {
  auto [r, s] = value.toRational();
  if (r == 0)
    return os << _("Not used");
  if (s == 0)
    return os << "(" << value << ")";
  return os << stringFormat("{:.1f}x", static_cast<float>(r) / s);
}

std::ostream& Nikon3MakerNote::print0x0088(std::ostream& os, const Value& value, const ExifData*) {
  if (value.size() != 4) {  // Size is 4 even for those who map this way...
    os << "(" << value << ")";
  } else {
    // Mapping by Roger Larsson
    const uint32_t focusmetering = value.toUint32(0);
    const uint32_t focuspoint = value.toUint32(1);
    const uint32_t focusused = (value.toUint32(2) << 8) + value.toUint32(3);
    // TODO: enum {standard, wide} combination = standard;

    if (focusmetering == 0 && focuspoint == 0 && focusused == 0) {
      // Special case, in Manual focus and with Nikon compacts
      // this indicates that the field has no meaning.
      // But when actually in "Single area, Center" this can mean
      // that focus was not found (try this in AF-C mode)
      // TODO: handle the meaningful case (interacts with other fields)
      os << _("n/a");
      return os;
    }

    switch (focusmetering) {
      case 0x00:
        os << _("Single area");
        break;  // D70, D200
      case 0x01:
        os << _("Dynamic area");
        break;  // D70, D200
      case 0x02:
        os << _("Closest subject");
        break;  // D70, D200
      case 0x03:
        os << _("Group dynamic-AF");
        break;  // D200
      case 0x04:
        os << _("Single area (wide)"); /* TODO: combination = wide; */
        break;                         // D200
      case 0x05:
        os << _("Dynamic area (wide)"); /* TODO: combination = wide; */
        break;                          // D200
      default:
        os << "(" << focusmetering << ")";
        break;
    }

    char sep = ';';
    if (focusmetering != 0x02) {  //  No user selected point for Closest subject
      os << sep << ' ';

      // What focuspoint did the user select?
      if (focuspoint < nikonFocuspoints.size()) {
        os << nikonFocuspoints[focuspoint];
        // TODO: os << position[focuspoint][combination]
      } else
        os << "(" << focuspoint << ")";

      sep = ',';
    }

    // What focuspoints(!) did the camera use? add if differs
    if (focusused == 0)
      os << sep << " " << _("none");
    else if (focusused != 1U << focuspoint) {
      // selected point was not the actually used one
      // (Roger Larsson: my interpretation, verify)
      os << sep;
      for (size_t fpid = 0; fpid < nikonFocuspoints.size(); fpid++)
        if (focusused & 1 << fpid)
          os << ' ' << nikonFocuspoints[fpid];
    }

    os << " " << _("used");
  }

  return os;
}

std::ostream& Nikon3MakerNote::printAfPointsInFocus(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.typeId() != unsignedShort)
    return os << "(" << value << ")";

  bool dModel = false;
  if (metadata) {
    auto pos = metadata->findKey(ExifKey("Exif.Image.Model"));
    if (pos != metadata->end() && pos->count() != 0) {
      std::string model = pos->toString();
      if (Internal::contains(model, "NIKON D")) {
        dModel = true;
      }
    }
  }

  auto val = static_cast<uint16_t>(value.toInt64());
  if (dModel)
    val = (val >> 8) | ((val & 0x00ffU) << 8);

  if (val == 0x07ff)
    return os << _("All 11 Points");

  UShortValue v;
  v.value_.push_back(val);
  return EXV_PRINT_TAG_BITMASK(nikonAfPointsInFocus)(os, v, nullptr);
}

std::ostream& Nikon3MakerNote::print0x0089(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.count() != 1 || value.typeId() != unsignedShort) {
    return os << "(" << value << ")";
  }
  const auto l = value.toInt64(0);
  if (l == 0)
    return os << _("Single-frame");
  if (!(l & 0x87))
    os << _("Single-frame") << ", ";
  bool d70 = false;
  if (metadata) {
    ExifKey key("Exif.Image.Model");
    auto pos = metadata->findKey(key);
    if (pos != metadata->end() && pos->count() != 0) {
      std::string model = pos->toString();
      if (Internal::contains(model, "D70")) {
        d70 = true;
      }
    }
  }
  if (d70) {
    EXV_PRINT_TAG_BITMASK(nikonShootingModeD70)(os, value, nullptr);
  } else {
    EXV_PRINT_TAG_BITMASK(nikonShootingMode)(os, value, nullptr);
  }
  return os;
}

std::ostream& Nikon3MakerNote::print0x008b(std::ostream& os, const Value& value, const ExifData*) {
  // Decoded by Robert Rottmerhusen <email@rottmerhusen.com>
  if (value.size() != 4 || value.typeId() != undefined) {
    return os << "(" << value << ")";
  }
  float a = value.toFloat(0);
  const auto b = value.toInt64(1);
  const auto c = value.toInt64(2);
  if (c == 0)
    return os << "(" << value << ")";
  return os << a * b / c;
}

static bool testConfigFile(std::ostream& os, const Value& value);
static bool testConfigFile(std::ostream& os, const Value& value) {
  bool result = false;
  const std::string undefined("undefined");
  const std::string section("nikon");
  if (Internal::readExiv2Config(section, value.toString(), undefined) != undefined) {
    os << Internal::readExiv2Config(section, value.toString(), undefined);
    result = true;
  }
  return result;
}

std::ostream& Nikon3MakerNote::printLensId1(std::ostream& os, const Value& value, const ExifData* metadata) {
  return testConfigFile(os, value) ? os : printLensId(os, value, metadata, "NikonLd1");
}

std::ostream& Nikon3MakerNote::printLensId2(std::ostream& os, const Value& value, const ExifData* metadata) {
  return testConfigFile(os, value) ? os : printLensId(os, value, metadata, "NikonLd2");
}

std::ostream& Nikon3MakerNote::printLensId3(std::ostream& os, const Value& value, const ExifData* metadata) {
  return testConfigFile(os, value) ? os : printLensId(os, value, metadata, "NikonLd3");
}

std::ostream& Nikon3MakerNote::printLensId4(std::ostream& os, const Value& value, const ExifData* metadata) {
  return testConfigFile(os, value) ? os : printLensId(os, value, metadata, "NikonLd4");
}

std::ostream& Nikon3MakerNote::printLensId(std::ostream& os, const Value& value, const ExifData* metadata,
                                           [[maybe_unused]] const std::string& group) {
#ifdef EXV_HAVE_LENSDATA
// 8< - - - 8< do not remove this line >8 - - - >8
//------------------------------------------------------------------------------
#ifndef FMOUNTLH
#define FMOUNTLH
//------------------------------------------------------------------------------
// List of AF F-Mount lenses - Version 4.4.550.02                    2018-03-03
//------------------------------------------------------------------------------
#define FMLVERSION "4.4.550.02"
#define FMLDATE "2018-03-03"
  //------------------------------------------------------------------------------
  //
  // Created by Robert Rottmerhusen 2005 - 2018
  // http://www.rottmerhusen.com (lens_id@rottmerhusen.com)
  //
  // Eight misidentified lenses due to double LensIDs:
  //
  // 2F 48 30 44 24 24 29 02.1: Nikon AF Zoom-Nikkor 20-35mm f/2.8D IF
  // 2F 48 30 44 24 24 29 02.2: Tokina AT-X 235 AF PRO (AF 20-35mm f/2.8)
  //
  // 32 54 6A 6A 24 24 35 02.1: Nikon AF Micro-Nikkor 105mm f/2.8D
  // 32 54 6A 6A 24 24 35 02.2: Sigma Macro 105mm F2.8 EX DG
  //
  // 7A 3C 1F 37 30 30 7E 06.1: Nikon AF-S DX Zoom-Nikkor 12-24mm f/4G IF-ED
  // 7A 3C 1F 37 30 30 7E 06.2: Tokina AT-X 124 AF PRO DX II (AF 12-24mm f/4)
  //
  // 8B 40 2D 80 2C 3C FD 0E.1: Nikon AF-S DX VR Zoom-Nikkor 18-200mm f/3.5-5.6G IF-ED
  // 8B 40 2D 80 2C 3C FD 0E.2: Nikon AF-S DX VR Zoom-Nikkor 18-200mm f/3.5-5.6G IF-ED II
  //
  // 32 53 64 64 24 24 35 02.1: Tamron SP AF 90mm F/2.8 Macro 1:1 (172E)
  // 32 53 64 64 24 24 35 02.2: Tamron SP AF 90mm F/2.8 Di Macro 1:1 (272E)
  //
  // 2F 40 30 44 2C 34 29 02.1: Tokina AF 235 II (AF 20-35mm f/3.5-4.5)
  // 2F 40 30 44 2C 34 29 02.2: Tokina AF 193 (AF 19-35mm f/3.5-4.5)
  //
  // 25 48 3C 5C 24 24 1B 02.1: Tokina AT-X 287 AF PRO SV (AF 28-70mm f/2.8)
  // 25 48 3C 5C 24 24 1B 02.2: Tokina AT-X 270 AF PRO II (AF 28-70mm f/2.6-2.8)
  //
  // 4A 48 24 24 24 0C 4D 02.1: Samyang AE 14mm f/2.8 ED AS IF UMC
  // 4A 48 24 24 24 0C 4D 02.2: Samyang 10mm f/2.8 ED AS NCS CS
  //
  //
  // product number/order code not complete
  //
  // Free use in non-commercial, GPL or open source software only!
  // Please contact me for adding lenses or use in commercial software.
  //
  //"data from TAG 0x98" "ltyp" " "TC" "MID" "maker" "PN" "lens name from manuf";
  //
  //------------------------------------------------------------------------------
  // EXIF data necessary for new entries
  //------------------------------------------------------------------------------
  //
  // lid:   LensIDNumber
  // stps:  LensFStops
  // focs:  MinFocalLength
  // focl:  MaxFocalLength
  // aps:   MaxApertureAtMinFocal
  // apl:   MaxApertureAtMaxFocal
  // lfw:   MCUVersion
  // ltype: LensType
  //
  // The tcinfo, dblid and mid fields are being ignored.
  //
  // Please note that all fields except LensType have to be looked up in the
  // Exif.NikonLd* prefix and not other Exif.Nikon* prefixes. For example: for modern
  // Nikon bodies with modern lenses, there will be both a Exif.Nikon3.LensFStops and a
  // Exif.NikonLd3.LensFStops entry in the EXIF data. You are looking for
  // Exif.NikonLd3.LensFStops.
  //
  // In most cases the necessary hex values should be extracted from a test image using
  // the following command:
  //
  // exiv2 -ph -g NikonLd3.LensIDNumber -g NikonLd3.LensFStops
  // -g NikonLd3.MinFocalLength -g NikonLd3.MaxFocalLength
  // -g NikonLd3.MaxApertureAtMinFocal -g NikonLd3.MaxApertureAtMaxFocal
  // -g NikonLd3.MCUVersion -g Nikon3.LensType test.NEF
  //
  // Please consider, that sequence of output is sligthly different from sequence in
  // data structure: LensType (ltype) is printed first, but has to be entered after
  // MCUVersion (lfw).
  //
  //------------------------------------------------------------------------------
  // Nikkor lenses by their LensID
  //------------------------------------------------------------------------------

  static constexpr struct FMntLens {
    unsigned char lid, stps, focs, focl, aps, apl, lfw, ltype, tcinfo, dblid, mid;
    std::string_view manuf;
    std::string_view lnumber;
    std::string_view lensname;

    bool operator==(unsigned char l) const {
      return lid == l;
    }
  } fmountlens[] = {
      {0x01, 0x58, 0x50, 0x50, 0x14, 0x14, 0x02, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA00901", "AF Nikkor 50mm f/1.8"},
      {0x01, 0x58, 0x50, 0x50, 0x14, 0x14, 0x05, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA00901", "AF Nikkor 50mm f/1.8"},
      {0x02, 0x42, 0x44, 0x5C, 0x2A, 0x34, 0x02, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA72701",
       "AF Zoom-Nikkor 35-70mm f/3.3-4.5"},
      {0x02, 0x42, 0x44, 0x5C, 0x2A, 0x34, 0x08, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA72701",
       "AF Zoom-Nikkor 35-70mm f/3.3-4.5"},
      {0x03, 0x48, 0x5C, 0x81, 0x30, 0x30, 0x02, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA72801",
       "AF Zoom-Nikkor 70-210mm f/4"},
      {0x04, 0x48, 0x3C, 0x3C, 0x24, 0x24, 0x03, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA12001", "AF Nikkor 28mm f/2.8"},
      {0x05, 0x54, 0x50, 0x50, 0x0C, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA01001", "AF Nikkor 50mm f/1.4"},
      {0x06, 0x54, 0x53, 0x53, 0x24, 0x24, 0x06, 0x00, 0x40, 0x00, 0x00, "Nikon", "JAA62101",
       "AF Micro-Nikkor 55mm f/2.8"},
      {0x07, 0x40, 0x3C, 0x62, 0x2C, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA72901",
       "AF Zoom-Nikkor 28-85mm f/3.5-4.5"},
      {0x08, 0x40, 0x44, 0x6A, 0x2C, 0x34, 0x04, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA73001",
       "AF Zoom-Nikkor 35-105mm f/3.5-4.5"},
      {0x09, 0x48, 0x37, 0x37, 0x24, 0x24, 0x04, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA12101", "AF Nikkor 24mm f/2.8"},
      {0x0A, 0x48, 0x8E, 0x8E, 0x24, 0x24, 0x03, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA322AA",
       "AF Nikkor 300mm f/2.8 IF-ED"},
      {0x0A, 0x48, 0x8E, 0x8E, 0x24, 0x24, 0x05, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA322AB",
       "AF Nikkor 300mm f/2.8 IF-ED"},
      {0x0B, 0x48, 0x7C, 0x7C, 0x24, 0x24, 0x05, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA32101",
       "AF Nikkor 180mm f/2.8 IF-ED"},
      // 0C
      {0x0D, 0x40, 0x44, 0x72, 0x2C, 0x34, 0x07, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA73101",
       "AF Zoom-Nikkor 35-135mm f/3.5-4.5"},
      {0x0E, 0x48, 0x5C, 0x81, 0x30, 0x30, 0x05, 0x00, 0x00, 0x00, 0x00, "Nikon", "", "AF Zoom-Nikkor 70-210mm f/4"},
      {0x0F, 0x58, 0x50, 0x50, 0x14, 0x14, 0x05, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA009AD", "AF Nikkor 50mm f/1.8 N"},
      {0x10, 0x48, 0x8E, 0x8E, 0x30, 0x30, 0x08, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA32301",
       "AF Nikkor 300mm f/4 IF-ED"},
      {0x11, 0x48, 0x44, 0x5C, 0x24, 0x24, 0x08, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA73301",
       "AF Zoom-Nikkor 35-70mm f/2.8"},
      {0x11, 0x48, 0x44, 0x5C, 0x24, 0x24, 0x15, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA73301",
       "AF Zoom-Nikkor 35-70mm f/2.8"},
      {0x12, 0x48, 0x5C, 0x81, 0x30, 0x3C, 0x09, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA73201",
       "AF Nikkor 70-210mm f/4-5.6"},
      {0x13, 0x42, 0x37, 0x50, 0x2A, 0x34, 0x0B, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA73401",
       "AF Zoom-Nikkor 24-50mm f/3.3-4.5"},
      {0x14, 0x48, 0x60, 0x80, 0x24, 0x24, 0x0B, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA73501",
       "AF Zoom-Nikkor 80-200mm f/2.8 ED"},
      {0x15, 0x4C, 0x62, 0x62, 0x14, 0x14, 0x0C, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA32401", "AF Nikkor 85mm f/1.8"},
      // 16
      {0x17, 0x3C, 0xA0, 0xA0, 0x30, 0x30, 0x0F, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA518AA",
       "Nikkor 500mm f/4 P ED IF"},
      {0x17, 0x3C, 0xA0, 0xA0, 0x30, 0x30, 0x11, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA518AA",
       "Nikkor 500mm f/4 P ED IF"},
      {0x18, 0x40, 0x44, 0x72, 0x2C, 0x34, 0x0E, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA736AA",
       "AF Zoom-Nikkor 35-135mm f/3.5-4.5 N"},
      // 19
      {0x1A, 0x54, 0x44, 0x44, 0x18, 0x18, 0x11, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA12201", "AF Nikkor 35mm f/2"},
      {0x1B, 0x44, 0x5E, 0x8E, 0x34, 0x3C, 0x10, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA738AA",
       "AF Zoom-Nikkor 75-300mm f/4.5-5.6"},
      {0x1C, 0x48, 0x30, 0x30, 0x24, 0x24, 0x12, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA12301", "AF Nikkor 20mm f/2.8"},
      {0x1D, 0x42, 0x44, 0x5C, 0x2A, 0x34, 0x12, 0x00, 0x00, 0x00, 0x00, "Nikon", "",
       "AF Zoom-Nikkor 35-70mm f/3.3-4.5 N"},
      {0x1E, 0x54, 0x56, 0x56, 0x24, 0x24, 0x13, 0x00, 0x40, 0x00, 0x00, "Nikon", "JAA62201",
       "AF Micro-Nikkor 60mm f/2.8"},
      {0x1F, 0x54, 0x6A, 0x6A, 0x24, 0x24, 0x14, 0x00, 0x40, 0x00, 0x00, "Nikon", "JAA62301",
       "AF Micro-Nikkor 105mm f/2.8"},
      {0x20, 0x48, 0x60, 0x80, 0x24, 0x24, 0x15, 0x00, 0x00, 0x00, 0x00, "Nikon", "",
       "AF Zoom-Nikkor 80-200mm f/2.8 ED"},
      {0x21, 0x40, 0x3C, 0x5C, 0x2C, 0x34, 0x16, 0x00, 0x00, 0x00, 0x00, "Nikon", "",
       "AF Zoom-Nikkor 28-70mm f/3.5-4.5"},
      {0x22, 0x48, 0x72, 0x72, 0x18, 0x18, 0x16, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA32501", "AF DC-Nikkor 135mm f/2"},
      {0x23, 0x30, 0xBE, 0xCA, 0x3C, 0x48, 0x17, 0x00, 0x00, 0x00, 0x00, "Nikon", "",
       "Zoom-Nikkor 1200-1700mm f/5.6-8 P ED IF"},
      // - D - lenses from here
      {0x24, 0x48, 0x60, 0x80, 0x24, 0x24, 0x1A, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA742DA",
       "AF Zoom-Nikkor 80-200mm f/2.8D ED"},
      {0x25, 0x48, 0x44, 0x5c, 0x24, 0x24, 0x1B, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA743DA",
       "AF Zoom-Nikkor 35-70mm f/2.8D"},
      {0x25, 0x48, 0x44, 0x5C, 0x24, 0x24, 0x3A, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA743DA",
       "AF Zoom-Nikkor 35-70mm f/2.8D"},
      {0x25, 0x48, 0x44, 0x5c, 0x24, 0x24, 0x52, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA743DA",
       "AF Zoom-Nikkor 35-70mm f/2.8D"},
      {0x26, 0x40, 0x3C, 0x5C, 0x2C, 0x34, 0x1C, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA744DA",
       "AF Zoom-Nikkor 28-70mm f/3.5-4.5D"},
      // ^- not yet verified
      {0x27, 0x48, 0x8E, 0x8E, 0x24, 0x24, 0x1D, 0x02, 0x07, 0x00, 0x00, "Nikon", "JAA326DA",
       "AF-I Nikkor 300mm f/2.8D IF-ED"},
      {0x27, 0x48, 0x8E, 0x8E, 0x24, 0x24, 0xF1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA326DA",
       "AF-I Nikkor 300mm f/2.8D IF-ED + TC-14E"},
      {0x27, 0x48, 0x8E, 0x8E, 0x24, 0x24, 0xE1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA326DA",
       "AF-I Nikkor 300mm f/2.8D IF-ED + TC-17E"},
      {0x27, 0x48, 0x8E, 0x8E, 0x24, 0x24, 0xF2, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA326DA",
       "AF-I Nikkor 300mm f/2.8D IF-ED + TC-20E"},
      {0x28, 0x3C, 0xA6, 0xA6, 0x30, 0x30, 0x1D, 0x02, 0x07, 0x00, 0x00, "Nikon", "JAA519DA",
       "AF-I Nikkor 600mm f/4D IF-ED"},
      {0x28, 0x3C, 0xA6, 0xA6, 0x30, 0x30, 0xF1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA519DA",
       "AF-I Nikkor 600mm f/4D IF-ED + TC-14E"},
      {0x28, 0x3C, 0xA6, 0xA6, 0x30, 0x30, 0xE1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA519DA",
       "AF-I Nikkor 600mm f/4D IF-ED + TC-17E"},
      {0x28, 0x3C, 0xA6, 0xA6, 0x30, 0x30, 0xF2, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA519DA",
       "AF-I Nikkor 600mm f/4D IF-ED + TC-20E"},
      // 29
      {0x2A, 0x54, 0x3C, 0x3C, 0x0C, 0x0C, 0x26, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA124DA", "AF Nikkor 28mm f/1.4D"},
      {0x2B, 0x3C, 0x44, 0x60, 0x30, 0x3C, 0x1F, 0x02, 0x00, 0x00, 0x00, "Nikon", "",
       "AF Zoom-Nikkor 35-80mm f/4-5.6D"},
      {0x2C, 0x48, 0x6A, 0x6A, 0x18, 0x18, 0x27, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA327DA",
       "AF DC-Nikkor 105mm f/2D"},
      {0x2D, 0x48, 0x80, 0x80, 0x30, 0x30, 0x21, 0x02, 0x40, 0x00, 0x00, "Nikon", "JAA624DA",
       "AF Micro-Nikkor 200mm f/4D IF-ED"},
      {0x2E, 0x48, 0x5C, 0x82, 0x30, 0x3C, 0x22, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA747DA",
       "AF Nikkor 70-210mm f/4-5.6D"},
      {0x2E, 0x48, 0x5C, 0x82, 0x30, 0x3C, 0x28, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA747DA",
       "AF Nikkor 70-210mm f/4-5.6D"},
      {0x2F, 0x48, 0x30, 0x44, 0x24, 0x24, 0x29, 0x02, 0x00, 0x01, 0x00, "Nikon", "JAA746DA",
       "AF Zoom-Nikkor 20-35mm f/2.8D IF"},
      {0x30, 0x48, 0x98, 0x98, 0x24, 0x24, 0x24, 0x02, 0x07, 0x00, 0x00, "Nikon", "JAA520DA",
       "AF-I Nikkor 400mm f/2.8D IF-ED"},
      {0x30, 0x48, 0x98, 0x98, 0x24, 0x24, 0xF1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA520DA",
       "AF-I Nikkor 400mm f/2.8D IF-ED + TC-14E"},
      {0x30, 0x48, 0x98, 0x98, 0x24, 0x24, 0xE1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA520DA",
       "AF-I Nikkor 400mm f/2.8D IF-ED + TC-17E"},
      {0x30, 0x48, 0x98, 0x98, 0x24, 0x24, 0xF2, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA520DA",
       "AF-I Nikkor 400mm f/2.8D IF-ED + TC-20E"},
      {0x31, 0x54, 0x56, 0x56, 0x24, 0x24, 0x25, 0x02, 0x40, 0x00, 0x00, "Nikon", "JAA625DA",
       "AF Micro-Nikkor 60mm f/2.8D"},
      {0x32, 0x54, 0x6A, 0x6A, 0x24, 0x24, 0x35, 0x02, 0x40, 0x01, 0x00, "Nikon", "JAA627DA",
       "AF Micro-Nikkor 105mm f/2.8D"},
      {0x33, 0x48, 0x2D, 0x2D, 0x24, 0x24, 0x31, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA126DA", "AF Nikkor 18mm f/2.8D"},
      {0x34, 0x48, 0x29, 0x29, 0x24, 0x24, 0x32, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA626DA",
       "AF Fisheye Nikkor 16mm f/2.8D"},
      {0x35, 0x3C, 0xA0, 0xA0, 0x30, 0x30, 0x33, 0x02, 0x07, 0x00, 0x00, "Nikon", "JAA521DA",
       "AF-I Nikkor 500mm f/4D IF-ED"},
      {0x35, 0x3C, 0xA0, 0xA0, 0x30, 0x30, 0xF1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA521DA",
       "AF-I Nikkor 500mm f/4D IF-ED + TC-14E"},
      {0x35, 0x3C, 0xA0, 0xA0, 0x30, 0x30, 0xE1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA521DA",
       "AF-I Nikkor 500mm f/4D IF-ED + TC-17E"},
      {0x35, 0x3C, 0xA0, 0xA0, 0x30, 0x30, 0xF2, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA521DA",
       "AF-I Nikkor 500mm f/4D IF-ED + TC-20E"},
      {0x36, 0x48, 0x37, 0x37, 0x24, 0x24, 0x34, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA125DA", "AF Nikkor 24mm f/2.8D"},
      {0x37, 0x48, 0x30, 0x30, 0x24, 0x24, 0x36, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA127DA", "AF Nikkor 20mm f/2.8D"},
      {0x38, 0x4C, 0x62, 0x62, 0x14, 0x14, 0x37, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA328DA", "AF Nikkor 85mm f/1.8D"},
      // 39                38
      {0x3A, 0x40, 0x3C, 0x5C, 0x2C, 0x34, 0x39, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA744DA",
       "AF Zoom-Nikkor 28-70mm f/3.5-4.5D"},
      {0x3B, 0x48, 0x44, 0x5C, 0x24, 0x24, 0x3A, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA743DA",
       "AF Zoom-Nikkor 35-70mm f/2.8D N"},
      {0x3C, 0x48, 0x60, 0x80, 0x24, 0x24, 0x3B, 0x02, 0x00, 0x00, 0x00, "Nikon", "",
       "AF Zoom-Nikkor 80-200mm f/2.8D ED"},
      {0x3D, 0x3C, 0x44, 0x60, 0x30, 0x3C, 0x3E, 0x02, 0x00, 0x00, 0x00, "Nikon", "",
       "AF Zoom-Nikkor 35-80mm f/4-5.6D"},
      {0x3E, 0x48, 0x3C, 0x3C, 0x24, 0x24, 0x3D, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA128DA", "AF Nikkor 28mm f/2.8D"},
      {0x3F, 0x40, 0x44, 0x6A, 0x2C, 0x34, 0x45, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA748DA",
       "AF Zoom-Nikkor 35-105mm f/3.5-4.5D"},
      // 40
      {0x41, 0x48, 0x7c, 0x7c, 0x24, 0x24, 0x43, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA330DA",
       "AF Nikkor 180mm f/2.8D IF-ED"},
      {0x42, 0x54, 0x44, 0x44, 0x18, 0x18, 0x44, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA129DA", "AF Nikkor 35mm f/2D"},
      {0x43, 0x54, 0x50, 0x50, 0x0C, 0x0C, 0x46, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA011DB", "AF Nikkor 50mm f/1.4D"},
      {0x44, 0x44, 0x60, 0x80, 0x34, 0x3C, 0x47, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA753DB",
       "AF Zoom-Nikkor 80-200mm f/4.5-5.6D"},
      {0x45, 0x40, 0x3C, 0x60, 0x2C, 0x3C, 0x48, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA752DA",
       "AF Zoom-Nikkor 28-80mm f/3.5-5.6D"},
      {0x46, 0x3C, 0x44, 0x60, 0x30, 0x3C, 0x49, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA754DA",
       "AF Zoom-Nikkor 35-80mm f/4-5.6D N"},
      {0x47, 0x42, 0x37, 0x50, 0x2A, 0x34, 0x4A, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA756DA",
       "AF Zoom-Nikkor 24-50mm f/3.3-4.5D"},
      {0x48, 0x48, 0x8E, 0x8E, 0x24, 0x24, 0x4B, 0x02, 0x07, 0x00, 0x00, "Nikon", "JAA333DA",
       "AF-S Nikkor 300mm f/2.8D IF-ED"},
      {0x48, 0x48, 0x8E, 0x8E, 0x24, 0x24, 0xF1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA333DA",
       "AF-S Nikkor 300mm f/2.8D IF-ED + TC-14E"},
      {0x48, 0x48, 0x8E, 0x8E, 0x24, 0x24, 0xE1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA333DA",
       "AF-S Nikkor 300mm f/2.8D IF-ED + TC-17E"},
      {0x48, 0x48, 0x8E, 0x8E, 0x24, 0x24, 0xF2, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA333DA",
       "AF-S Nikkor 300mm f/2.8D IF-ED + TC-20E"},
      {0x49, 0x3C, 0xA6, 0xA6, 0x30, 0x30, 0x4C, 0x02, 0x07, 0x00, 0x00, "Nikon", "JAA522DA",
       "AF-S Nikkor 600mm f/4D IF-ED"},
      {0x49, 0x3C, 0xA6, 0xA6, 0x30, 0x30, 0xF1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA522DA",
       "AF-S Nikkor 600mm f/4D IF-ED + TC-14E"},
      {0x49, 0x3C, 0xA6, 0xA6, 0x30, 0x30, 0xE1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA522DA",
       "AF-S Nikkor 600mm f/4D IF-ED + TC-17E"},
      {0x49, 0x3C, 0xA6, 0xA6, 0x30, 0x30, 0xF2, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA522DA",
       "AF-S Nikkor 600mm f/4D IF-ED + TC-20E"},
      {0x4A, 0x54, 0x62, 0x62, 0x0C, 0x0C, 0x4D, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA332DA",
       "AF Nikkor 85mm f/1.4D IF"},
      {0x4B, 0x3C, 0xA0, 0xA0, 0x30, 0x30, 0x4E, 0x02, 0x07, 0x00, 0x00, "Nikon", "JAA523DA",
       "AF-S Nikkor 500mm f/4D IF-ED"},
      {0x4B, 0x3C, 0xA0, 0xA0, 0x30, 0x30, 0xF1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA523DA",
       "AF-S Nikkor 500mm f/4D IF-ED + TC-14E"},
      {0x4B, 0x3C, 0xA0, 0xA0, 0x30, 0x30, 0xE1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA523DA",
       "AF-S Nikkor 500mm f/4D IF-ED + TC-17E"},
      {0x4B, 0x3C, 0xA0, 0xA0, 0x30, 0x30, 0xF2, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA523DA",
       "AF-S Nikkor 500mm f/4D IF-ED + TC-20E"},
      {0x4C, 0x40, 0x37, 0x6E, 0x2C, 0x3C, 0x4F, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA757DA",
       "AF Zoom-Nikkor 24-120mm f/3.5-5.6D IF"},
      {0x4D, 0x40, 0x3C, 0x80, 0x2C, 0x3C, 0x62, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA758DA",
       "AF Zoom-Nikkor 28-200mm f/3.5-5.6D IF"},
      {0x4E, 0x48, 0x72, 0x72, 0x18, 0x18, 0x51, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA329DA",
       "AF DC-Nikkor 135mm f/2D"},
      {0x4F, 0x40, 0x37, 0x5C, 0x2C, 0x3C, 0x53, 0x06, 0x00, 0x00, 0x00, "Nikon", "JBA701AA",
       "IX-Nikkor 24-70mm f/3.5-5.6"},
      {0x50, 0x48, 0x56, 0x7C, 0x30, 0x3C, 0x54, 0x06, 0x00, 0x00, 0x00, "Nikon", "JBA702AA",
       "IX-Nikkor 60-180mm f/4-5.6"},
      //                                                  "JBA703AC" "IX-Nikkor 20-60 mm f/3.5-5.6";
      // 51
      // 52
      {0x53, 0x48, 0x60, 0x80, 0x24, 0x24, 0x57, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA762DA",
       "AF Zoom-Nikkor 80-200mm f/2.8D ED"},
      {0x53, 0x48, 0x60, 0x80, 0x24, 0x24, 0x60, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA762DA",
       "AF Zoom-Nikkor 80-200mm f/2.8D ED"},
      {0x54, 0x44, 0x5C, 0x7C, 0x34, 0x3C, 0x58, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA763DA",
       "AF Zoom-Micro Nikkor 70-180mm f/4.5-5.6D ED"},
      {0x54, 0x44, 0x5C, 0x7C, 0x34, 0x3C, 0x61, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA763DA",
       "AF Zoom-Micro Nikkor 70-180mm f/4.5-5.6D ED"},
      // 55
      {0x56, 0x48, 0x5C, 0x8E, 0x30, 0x3C, 0x5A, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA764DA",
       "AF Zoom-Nikkor 70-300mm f/4-5.6D ED"},
      // 57
      // 58
      {0x59, 0x48, 0x98, 0x98, 0x24, 0x24, 0x5D, 0x02, 0x07, 0x00, 0x00, "Nikon", "JAA524DA",
       "AF-S Nikkor 400mm f/2.8D IF-ED"},
      {0x59, 0x48, 0x98, 0x98, 0x24, 0x24, 0xF1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA524DA",
       "AF-S Nikkor 400mm f/2.8D IF-ED + TC-14E"},
      {0x59, 0x48, 0x98, 0x98, 0x24, 0x24, 0xE1, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA524DA",
       "AF-S Nikkor 400mm f/2.8D IF-ED + TC-17E"},
      {0x59, 0x48, 0x98, 0x98, 0x24, 0x24, 0xF2, 0x02, 0x0F, 0x00, 0x00, "Nikon", "JAA524DA",
       "AF-S Nikkor 400mm f/2.8D IF-ED + TC-20E"},
      {0x5A, 0x3C, 0x3E, 0x56, 0x30, 0x3C, 0x5E, 0x06, 0x00, 0x00, 0x00, "Nikon", "JBA704AA",
       "IX-Nikkor 30-60mm f/4-5.6"},
      {0x5B, 0x44, 0x56, 0x7C, 0x34, 0x3C, 0x5F, 0x06, 0x00, 0x00, 0x00, "Nikon", "JBA705AA",
       "IX-Nikkor 60-180mm f/4.5-5.6"},
      //                                                  "JBA706AC" "IX-Nikkor 20-60 mm f/3.5-5.6N";
      // 5C
      {0x5D, 0x48, 0x3C, 0x5C, 0x24, 0x24, 0x63, 0x02, 0x01, 0x00, 0x00, "Nikon", "JAA767DA",
       "AF-S Zoom-Nikkor 28-70mm f/2.8D IF-ED"},
      {0x5E, 0x48, 0x60, 0x80, 0x24, 0x24, 0x64, 0x02, 0x03, 0x00, 0x00, "Nikon", "JAA765DA",
       "AF-S Zoom-Nikkor 80-200mm f/2.8D IF-ED"},
      {0x5F, 0x40, 0x3C, 0x6A, 0x2C, 0x34, 0x65, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA766DA",
       "AF Zoom-Nikkor 28-105mm f/3.5-4.5D IF"},
      {0x60, 0x40, 0x3C, 0x60, 0x2C, 0x3C, 0x66, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA769DA",
       "AF Zoom-Nikkor 28-80mm f/3.5-5.6D"},
      {0x61, 0x44, 0x5E, 0x86, 0x34, 0x3C, 0x67, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA768DA",
       "AF Zoom-Nikkor 75-240mm f/4.5-5.6D"},
      // 62                69
      {0x63, 0x48, 0x2B, 0x44, 0x24, 0x24, 0x68, 0x02, 0x01, 0x00, 0x00, "Nikon", "JAA770DA",
       "AF-S Nikkor 17-35mm f/2.8D IF-ED"},
      {0x64, 0x00, 0x62, 0x62, 0x24, 0x24, 0x6A, 0x02, 0x40, 0x00, 0x00, "Nikon", "JAA628DA",
       "PC Micro-Nikkor 85mm f/2.8D"},
      {0x65, 0x44, 0x60, 0x98, 0x34, 0x3C, 0x6B, 0x0A, 0x00, 0x00, 0x00, "Nikon", "JAA771DA",
       "AF VR Zoom-Nikkor 80-400mm f/4.5-5.6D ED"},
      {0x66, 0x40, 0x2D, 0x44, 0x2C, 0x34, 0x6C, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA772DA",
       "AF Zoom-Nikkor 18-35mm f/3.5-4.5D IF-ED"},
      {0x67, 0x48, 0x37, 0x62, 0x24, 0x30, 0x6D, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA774DA",
       "AF Zoom-Nikkor 24-85mm f/2.8-4D IF"},
      {0x68, 0x42, 0x3C, 0x60, 0x2A, 0x3C, 0x6E, 0x06, 0x00, 0x00, 0x00, "Nikon", "JAA777DA",
       "AF Zoom-Nikkor 28-80mm f/3.3-5.6G"},
      {0x69, 0x48, 0x5C, 0x8E, 0x30, 0x3C, 0x6F, 0x06, 0x00, 0x00, 0x00, "Nikon", "JAA776DA",
       "AF Zoom-Nikkor 70-300mm f/4-5.6G"},
      {0x6A, 0x48, 0x8E, 0x8E, 0x30, 0x30, 0x70, 0x02, 0x03, 0x00, 0x00, "Nikon", "JAA334DA",
       "AF-S Nikkor 300mm f/4D IF-ED"},
      {0x6B, 0x48, 0x24, 0x24, 0x24, 0x24, 0x71, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA130DA",
       "AF Nikkor ED 14mm f/2.8D"},
      // 6C                72
      {0x6D, 0x48, 0x8E, 0x8E, 0x24, 0x24, 0x73, 0x02, 0x03, 0x00, 0x00, "Nikon", "JAA335DA",
       "AF-S Nikkor 300mm f/2.8D IF-ED II"},
      {0x6E, 0x48, 0x98, 0x98, 0x24, 0x24, 0x74, 0x02, 0x03, 0x00, 0x00, "Nikon", "JAA525DA",
       "AF-S Nikkor 400mm f/2.8D IF-ED II"},
      {0x6F, 0x3C, 0xA0, 0xA0, 0x30, 0x30, 0x75, 0x02, 0x03, 0x00, 0x00, "Nikon", "JAA526DA",
       "AF-S Nikkor 500mm f/4D IF-ED II"},
      {0x70, 0x3C, 0xA6, 0xA6, 0x30, 0x30, 0x76, 0x02, 0x03, 0x00, 0x00, "Nikon", "JAA527DA",
       "AF-S Nikkor 600mm f/4D IF-ED II"},
      // 71
      {0x72, 0x48, 0x4C, 0x4C, 0x24, 0x24, 0x77, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA012AA", "Nikkor 45mm f/2.8 P"},
      // 73
      {0x74, 0x40, 0x37, 0x62, 0x2C, 0x34, 0x78, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA780DA",
       "AF-S Zoom-Nikkor 24-85mm f/3.5-4.5G IF-ED"},
      {0x75, 0x40, 0x3C, 0x68, 0x2C, 0x3C, 0x79, 0x06, 0x00, 0x00, 0x00, "Nikon", "JAA778DA",
       "AF Zoom-Nikkor 28-100mm f/3.5-5.6G"},
      {0x76, 0x58, 0x50, 0x50, 0x14, 0x14, 0x7A, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA013DA", "AF Nikkor 50mm f/1.8D"},
      {0x77, 0x48, 0x5C, 0x80, 0x24, 0x24, 0x7B, 0x0E, 0x03, 0x00, 0x00, "Nikon", "JAA781DA",
       "AF-S VR Zoom-Nikkor 70-200mm f/2.8G IF-ED"},
      {0x78, 0x40, 0x37, 0x6E, 0x2C, 0x3C, 0x7C, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA782DA",
       "AF-S VR Zoom-Nikkor 24-120mm f/3.5-5.6G IF-ED"},
      {0x79, 0x40, 0x3C, 0x80, 0x2C, 0x3C, 0x7F, 0x06, 0x00, 0x00, 0x00, "Nikon", "JAA783DA",
       "AF Zoom-Nikkor 28-200mm f/3.5-5.6G IF-ED"},
      {0x7A, 0x3C, 0x1F, 0x37, 0x30, 0x30, 0x7E, 0x06, 0x01, 0x01, 0x00, "Nikon", "JAA784DA",
       "AF-S DX Zoom-Nikkor 12-24mm f/4G IF-ED"},
      {0x7B, 0x48, 0x80, 0x98, 0x30, 0x30, 0x80, 0x0E, 0x03, 0x00, 0x00, "Nikon", "JAA787DA",
       "AF-S VR Zoom-Nikkor 200-400mm f/4G IF-ED"},
      // 7C                81
      {0x7D, 0x48, 0x2B, 0x53, 0x24, 0x24, 0x82, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA788DA",
       "AF-S DX Zoom-Nikkor 17-55mm f/2.8G IF-ED"},
      // 7E                83
      {0x7F, 0x40, 0x2D, 0x5C, 0x2C, 0x34, 0x84, 0x06, 0x11, 0x00, 0x00, "Nikon", "JAA790DA",
       "AF-S DX Zoom-Nikkor 18-70mm f/3.5-4.5G IF-ED"},
      {0x80, 0x48, 0x1A, 0x1A, 0x24, 0x24, 0x85, 0x06, 0x00, 0x00, 0x00, "Nikon", "JAA629DA",
       "AF DX Fisheye-Nikkor 10.5mm f/2.8G ED"},
      {0x81, 0x54, 0x80, 0x80, 0x18, 0x18, 0x86, 0x0E, 0x03, 0x00, 0x00, "Nikon", "JAA336DA",
       "AF-S VR Nikkor 200mm f/2G IF-ED"},
      {0x82, 0x48, 0x8E, 0x8E, 0x24, 0x24, 0x87, 0x0E, 0x13, 0x00, 0x00, "Nikon", "JAA337DA",
       "AF-S VR Nikkor 300mm f/2.8G IF-ED"},
      {0x83, 0x00, 0xB0, 0xB0, 0x5A, 0x5A, 0x88, 0x04, 0x00, 0x00, 0x00, "Nikon", "", "FSA-L2, EDG 65, 800mm F13 G"},
      // 84
      // 85
      // 86
      // 87
      // 88
      {0x89, 0x3C, 0x53, 0x80, 0x30, 0x3C, 0x8B, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA793DA",
       "AF-S DX Zoom-Nikkor 55-200mm f/4-5.6G ED"},
      {0x8A, 0x54, 0x6A, 0x6A, 0x24, 0x24, 0x8C, 0x0E, 0x53, 0x00, 0x00, "Nikon", "JAA630DA",
       "AF-S VR Micro-Nikkor 105mm f/2.8G IF-ED"},
      {0x8B, 0x40, 0x2D, 0x80, 0x2C, 0x3C, 0x8D, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA794DA",
       "AF-S DX VR Zoom-Nikkor 18-200mm f/3.5-5.6G IF-ED"},
      //{0x8B,0x40,0x2D,0x80,0x2C,0x3C,0xFD,0x0E,0x01,0x01,0x00, "Nikon", "JAA794DA", "AF-S DX VR Zoom-Nikkor
      // 18-200mm f/3.5-5.6G IF-ED"}, // Fix bug #1026.
      {0x8B, 0x40, 0x2D, 0x80, 0x2C, 0x3C, 0xFD, 0x0E, 0x01, 0x02, 0x00, "Nikon", "JAA813DA",
       "AF-S DX VR Zoom-Nikkor 18-200mm f/3.5-5.6G IF-ED II"},
      {0x8C, 0x40, 0x2D, 0x53, 0x2C, 0x3C, 0x8E, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA792DA",
       "AF-S DX Zoom-Nikkor 18-55mm f/3.5-5.6G ED"},
      {0x8D, 0x44, 0x5C, 0x8E, 0x34, 0x3C, 0x8F, 0x0E, 0x31, 0x00, 0x00, "Nikon", "JAA795DA",
       "AF-S VR Zoom-Nikkor 70-300mm f/4.5-5.6G IF-ED"},
      // 8E                90
      {0x8F, 0x40, 0x2D, 0x72, 0x2C, 0x3C, 0x91, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA796DA",
       "AF-S DX Zoom-Nikkor 18-135mm f/3.5-5.6G IF-ED"},
      {0x90, 0x3B, 0x53, 0x80, 0x30, 0x3C, 0x92, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA798DA",
       "AF-S DX VR Zoom-Nikkor 55-200mm f/4-5.6G IF-ED"},
      // 91                93
      {0x92, 0x48, 0x24, 0x37, 0x24, 0x24, 0x94, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA801DA",
       "AF-S Zoom-Nikkor 14-24mm f/2.8G ED"},
      {0x93, 0x48, 0x37, 0x5C, 0x24, 0x24, 0x95, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA802DA",
       "AF-S Zoom-Nikkor 24-70mm f/2.8G ED"},
      {0x94, 0x40, 0x2D, 0x53, 0x2C, 0x3C, 0x96, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA797DA",
       "AF-S DX Zoom-Nikkor 18-55mm f/3.5-5.6G ED II"},
      {0x95, 0x4C, 0x37, 0x37, 0x2C, 0x2C, 0x97, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA631DA",
       "PC-E Nikkor 24mm f/3.5D ED"},
      {0x95, 0x00, 0x37, 0x37, 0x2C, 0x2C, 0x97, 0x06, 0x00, 0x00, 0x00, "Nikon", "JAA631DA",
       "PC-E Nikkor 24mm f/3.5D ED"},
      {0x96, 0x48, 0x98, 0x98, 0x24, 0x24, 0x98, 0x0E, 0x13, 0x00, 0x00, "Nikon", "JAA528DA",
       "AF-S VR Nikkor 400mm f/2.8G ED"},
      {0x97, 0x3C, 0xA0, 0xA0, 0x30, 0x30, 0x99, 0x0E, 0x13, 0x00, 0x00, "Nikon", "JAA529DA",
       "AF-S VR Nikkor 500mm f/4G ED"},
      {0x98, 0x3C, 0xA6, 0xA6, 0x30, 0x30, 0x9A, 0x0E, 0x13, 0x00, 0x00, "Nikon", "JAA530DA",
       "AF-S VR Nikkor 600mm f/4G ED"},
      {0x99, 0x40, 0x29, 0x62, 0x2C, 0x3C, 0x9B, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA800DA",
       "AF-S DX VR Zoom-Nikkor 16-85mm f/3.5-5.6G ED"},
      {0x9A, 0x40, 0x2D, 0x53, 0x2C, 0x3C, 0x9C, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA803DA",
       "AF-S DX VR Zoom-Nikkor 18-55mm f/3.5-5.6G"},
      {0x9B, 0x54, 0x4C, 0x4C, 0x24, 0x24, 0x9D, 0x02, 0x00, 0x00, 0x00, "Nikon", "JAA633DA",
       "PC-E Micro Nikkor 45mm f/2.8D ED"},
      {0x9B, 0x00, 0x4C, 0x4C, 0x24, 0x24, 0x9D, 0x06, 0x00, 0x00, 0x00, "Nikon", "JAA633DA",
       "PC-E Micro Nikkor 45mm f/2.8D ED"},
      {0x9C, 0x54, 0x56, 0x56, 0x24, 0x24, 0x9E, 0x06, 0x41, 0x00, 0x00, "Nikon", "JAA632DA",
       "AF-S Micro Nikkor 60mm f/2.8G ED"},
      {0x9D, 0x54, 0x62, 0x62, 0x24, 0x24, 0x9F, 0x02, 0x40, 0x00, 0x00, "Nikon", "JAA634DA",
       "PC-E Micro Nikkor 85mm f/2.8D"},
      {0x9D, 0x00, 0x62, 0x62, 0x24, 0x24, 0x9F, 0x06, 0x40, 0x00, 0x00, "Nikon", "JAA634DA",
       "PC-E Micro Nikkor 85mm f/2.8D"},
      {0x9E, 0x40, 0x2D, 0x6A, 0x2C, 0x3C, 0xA0, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA805DA",
       "AF-S DX VR Zoom-Nikkor 18-105mm f/3.5-5.6G ED"},
      {0x9F, 0x58, 0x44, 0x44, 0x14, 0x14, 0xA1, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA132DA",
       "AF-S DX Nikkor 35mm f/1.8G"},
      {0xA0, 0x54, 0x50, 0x50, 0x0C, 0x0C, 0xA2, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA014DA",
       "AF-S Nikkor 50mm f/1.4G"},
      {0xA1, 0x40, 0x18, 0x37, 0x2C, 0x34, 0xA3, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA804DA",
       "AF-S DX Nikkor 10-24mm f/3.5-4.5G ED"},
      {0xA2, 0x38, 0x5C, 0x8E, 0x34, 0x40, 0xCD, 0x86, 0x00, 0x00, 0x00, "Nikon", "JAA829DA",
       "AF-P DX Nikkor 70-300mm f/4.5-6.3G ED VR"},
      {0xA2, 0x48, 0x5C, 0x80, 0x24, 0x24, 0xA4, 0x0E, 0x13, 0x00, 0x00, "Nikon", "JAA807DA",
       "AF-S Nikkor 70-200mm f/2.8G ED VR II"},
      {0xA3, 0x3C, 0x29, 0x44, 0x30, 0x30, 0xA5, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA806DA",
       "AF-S Nikkor 16-35mm f/4G ED VR"},
      {0xA4, 0x54, 0x37, 0x37, 0x0C, 0x0C, 0xA6, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA131DA",
       "AF-S Nikkor 24mm f/1.4G ED"},
      {0xA5, 0x40, 0x3C, 0x8E, 0x2C, 0x3C, 0xA7, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA808DA",
       "AF-S Nikkor 28-300mm f/3.5-5.6G ED VR"},
      {0xA6, 0x48, 0x8E, 0x8E, 0x24, 0x24, 0xA8, 0x0E, 0x13, 0x00, 0x00, "Nikon", "JAA339DA",
       "AF-S Nikkor 300mm f/2.8G IF-ED VR II"},
      {0xA7, 0x4B, 0x62, 0x62, 0x2C, 0x2C, 0xA9, 0x0E, 0x41, 0x00, 0x00, "Nikon", "JAA637DA",
       "AF-S DX Micro Nikkor 85mm f/3.5G ED VR"},
      {0xA8, 0x48, 0x80, 0x98, 0x30, 0x30, 0xAA, 0x0E, 0x03, 0x00, 0x00, "Nikon", "JAA809DA",
       "AF-S Zoom-Nikkor 200-400mm f/4G IF-ED VR II"},
      {0xA9, 0x54, 0x80, 0x80, 0x18, 0x18, 0xAB, 0x0E, 0x13, 0x00, 0x00, "Nikon", "JAA340DA",
       "AF-S Nikkor 200mm f/2G ED VR II"},
      {0xAA, 0x3C, 0x37, 0x6E, 0x30, 0x30, 0xAC, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA811DA",
       "AF-S Nikkor 24-120mm f/4G ED VR"},
      // AB                AD                                          -- no lens --
      {0xAC, 0x38, 0x53, 0x8E, 0x34, 0x3C, 0xAE, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA814DA",
       "AF-S DX Nikkor 55-300mm f/4.5-5.6G ED VR"},
      {0xAD, 0x3C, 0x2D, 0x8E, 0x2C, 0x3C, 0xAF, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA812DA",
       "AF-S DX Nikkor 18-300mm f/3.5-5.6G ED VR"},
      {0xAD, 0x3C, 0xA0, 0xA0, 0x3C, 0x3C, 0xD8, 0x4E, 0x00, 0x00, 0x00, "Nikon", "JAA535DA",
       "AF-S Nikkor 500mm f/5.6E PF ED VR"},
      {0xAE, 0x54, 0x62, 0x62, 0x0C, 0x0C, 0xB0, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA338DA",
       "AF-S Nikkor 85mm f/1.4G"},
      {0xAF, 0x54, 0x44, 0x44, 0x0C, 0x0C, 0xB1, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA134DA",
       "AF-S Nikkor 35mm f/1.4G"},
      {0xB0, 0x4C, 0x50, 0x50, 0x14, 0x14, 0xB2, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA015DA",
       "AF-S Nikkor 50mm f/1.8G"},
      {0xB1, 0x48, 0x48, 0x48, 0x24, 0x24, 0xB3, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA638DA",
       "AF-S DX Micro Nikkor 40mm f/2.8G"},
      {0xB2, 0x48, 0x5C, 0x80, 0x30, 0x30, 0xB4, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA815DA",
       "AF-S Nikkor 70-200mm f/4G ED VR"},
      {0xB3, 0x4C, 0x62, 0x62, 0x14, 0x14, 0xB5, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA341DA",
       "AF-S Nikkor 85mm f/1.8G"},
      {0xB4, 0x40, 0x37, 0x62, 0x2C, 0x34, 0xB6, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA816DA",
       "AF-S Nikkor 24-85mm f/3.5-4.5G ED VR"},
      {0xB5, 0x4C, 0x3C, 0x3C, 0x14, 0x14, 0xB7, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA135DA",
       "AF-S Nikkor 28mm f/1.8G"},
      {0xB6, 0x3C, 0xB0, 0xB0, 0x3C, 0x3C, 0xB8, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA531DA",
       "AF-S VR Nikkor 800mm f/5.6E FL ED"},
      {0xB6, 0x3C, 0xB0, 0xB0, 0x3C, 0x3C, 0xB8, 0x4E, 0x01, 0x00, 0x00, "Nikon", "JAA531DA",
       "AF-S VR Nikkor 800mm f/5.6E FL ED"},
      {0xB7, 0x44, 0x60, 0x98, 0x34, 0x3C, 0xB9, 0x0E, 0x01, 0x00, 0x00, "Nikon", "JAA817DA",
       "AF-S Nikkor 80-400mm f/4.5-5.6G ED VR"},
      {0xB8, 0x40, 0x2D, 0x44, 0x2C, 0x34, 0xBA, 0x06, 0x01, 0x00, 0x00, "Nikon", "JAA818DA",
       "AF-S Nikkor 18-35mm f/3.5-4.5G ED"},
      {0xA0, 0x40, 0x2D, 0x74, 0x2C, 0x3C, 0xBB, 0x0E, 0x01, 0x00, 0x01, "Nikon", "JAA819DA",
       "AF-S DX Nikkor 18-140mm f/3.5-5.6G ED VR"},
      {0xA1, 0x54, 0x55, 0x55, 0x0C, 0x0C, 0xBC, 0x06, 0x01, 0x00, 0x01, "Nikon", "JAA136DA",
       "AF-S Nikkor 58mm f/1.4G"},
      {0xA2, 0x40, 0x2D, 0x53, 0x2C, 0x3C, 0xBD, 0x0E, 0x01, 0x00, 0x01, "Nikon", "JAA820DA",
       "AF-S DX Nikkor 18-55mm f/3.5-5.6G VR II"},
      {0xA4, 0x40, 0x2D, 0x8E, 0x2C, 0x40, 0xBF, 0x0E, 0x01, 0x00, 0x01, "Nikon", "JAA821DA",
       "AF-S DX Nikkor 18-300mm f/3.5-6.3G ED VR"},
      {0xA5, 0x4C, 0x44, 0x44, 0x14, 0x14, 0xC0, 0x06, 0x01, 0x00, 0x01, "Nikon", "JAA137DA",
       "AF-S Nikkor 35mm f/1.8G ED"},
      {0xA6, 0x48, 0x98, 0x98, 0x24, 0x24, 0xC1, 0x0E, 0x01, 0x00, 0x01, "Nikon", "JAA532DA",
       "AF-S Nikkor 400mm f/2.8E FL ED VR"},
      {0xA7, 0x3C, 0x53, 0x80, 0x30, 0x3C, 0xC2, 0x0E, 0x01, 0x00, 0x01, "Nikon", "JAA823DA",
       "AF-S DX Nikkor 55-200mm f/4-5.6G ED VR II"},
      {0xA8, 0x48, 0x8E, 0x8E, 0x30, 0x30, 0xC3, 0x4E, 0x01, 0x00, 0x01, "Nikon", "JAA342DA",
       "AF-S Nikkor 300mm f/4E PF ED VR"},
      {0xA8, 0x48, 0x8E, 0x8E, 0x30, 0x30, 0xC3, 0x0E, 0x01, 0x00, 0x01, "Nikon", "JAA342DA",
       "AF-S Nikkor 300mm f/4E PF ED VR"},
      {0xA9, 0x4C, 0x31, 0x31, 0x14, 0x14, 0xC4, 0x06, 0x01, 0x00, 0x01, "Nikon", "JAA138DA",
       "AF-S Nikkor 20mm f/1.8G ED"},
      {0xAA, 0x48, 0x37, 0x5C, 0x24, 0x24, 0xC5, 0x4E, 0x01, 0x00, 0x02, "Nikon", "JAA824DA",
       "AF-S Nikkor 24-70mm f/2.8E ED VR"},
      {0xAA, 0x48, 0x37, 0x5C, 0x24, 0x24, 0xC5, 0x0E, 0x01, 0x00, 0x02, "Nikon", "JAA824DA",
       "AF-S Nikkor 24-70mm f/2.8E ED VR"},
      {0xAB, 0x3C, 0xA0, 0xA0, 0x30, 0x30, 0xC6, 0x4E, 0x01, 0x00, 0x02, "Nikon", "JAA533DA",
       "AF-S Nikkor 500mm f/4E FL ED VR"},
      {0xAC, 0x3C, 0xA6, 0xA6, 0x30, 0x30, 0xC7, 0x4E, 0x01, 0x00, 0x02, "Nikon", "JAA534DA",
       "AF-S Nikkor 600mm f/4E FL ED VR"},
      {0xAD, 0x48, 0x28, 0x60, 0x24, 0x30, 0xC8, 0x4E, 0x01, 0x00, 0x02, "Nikon", "JAA825DA",
       "AF-S DX Nikkor 16-80mm f/2.8-4E ED VR"},
      {0xAD, 0x48, 0x28, 0x60, 0x24, 0x30, 0xC8, 0x0E, 0x01, 0x00, 0x02, "Nikon", "JAA825DA",
       "AF-S DX Nikkor 16-80mm f/2.8-4E ED VR"},
      {0xAE, 0x3C, 0x80, 0xA0, 0x3C, 0x3C, 0xC9, 0x4E, 0x01, 0x00, 0x02, "Nikon", "JAA822DA",
       "AF-S Nikkor 200-500mm f/5.6E ED VR"},
      {0xAE, 0x3C, 0x80, 0xA0, 0x3C, 0x3C, 0xC9, 0x0E, 0x01, 0x00, 0x02, "Nikon", "JAA822DA",
       "AF-S Nikkor 200-500mm f/5.6E ED VR"},
      {0xA0, 0x40, 0x2D, 0x53, 0x2C, 0x3C, 0xCA, 0x8E, 0x01, 0x00, 0x03, "Nikon", "JAA826DA",
       "AF-P DX Nikkor 18-55mm f/3.5-5.6G VR"},
      {0xA0, 0x40, 0x2D, 0x53, 0x2C, 0x3C, 0xCA, 0x0E, 0x01, 0x00, 0x03, "Nikon", "JAA827DA",
       "AF-P DX Nikkor 18-55mm f/3.5-5.6G"},
      //                  CB
      {0xAF, 0x4C, 0x37, 0x37, 0x14, 0x14, 0xCC, 0x06, 0x01, 0x00, 0x03, "Nikon", "JAA139DA",
       "AF-S Nikkor 24mm f/1.8G ED"},
      {0xA3, 0x38, 0x5C, 0x8E, 0x34, 0x40, 0xCE, 0x8E, 0x01, 0x00, 0x03, "Nikon", "JAA829DA",
       "AF-P DX Nikkor 70-300mm f/4.5-6.3G ED VR"},
      {0xA3, 0x38, 0x5C, 0x8E, 0x34, 0x40, 0xCE, 0x0E, 0x01, 0x00, 0x03, "Nikon", "JAA828DA",
       "AF-P DX Nikkor 70-300mm f/4.5-6.3G ED"},
      {0xA4, 0x48, 0x5C, 0x80, 0x24, 0x24, 0xCF, 0x4E, 0x01, 0x00, 0x03, "Nikon", "JAA830DA",
       "AF-S Nikkor 70-200mm f/2.8E FL ED VR"},
      {0xA4, 0x48, 0x5C, 0x80, 0x24, 0x24, 0xCF, 0x0E, 0x01, 0x00, 0x03, "Nikon", "JAA830DA",
       "AF-S Nikkor 70-200mm f/2.8E FL ED VR"},
      {0xA5, 0x54, 0x6A, 0x6A, 0x0C, 0x0C, 0xD0, 0x46, 0x01, 0x00, 0x03, "Nikon", "JAA343DA",
       "AF-S Nikkor 105mm f/1.4E ED"},
      {0xA5, 0x54, 0x6A, 0x6A, 0x0C, 0x0C, 0xD0, 0x06, 0x01, 0x00, 0x03, "Nikon", "JAA343DA",
       "AF-S Nikkor 105mm f/1.4E ED"},
      {0xA6, 0x48, 0x2F, 0x2F, 0x30, 0x30, 0xD1, 0x46, 0x01, 0x00, 0x03, "Nikon", "JAA639DA", "PC Nikkor 19mm f/4E ED"},
      {0xA6, 0x48, 0x2F, 0x2F, 0x30, 0x30, 0xD1, 0x06, 0x01, 0x00, 0x03, "Nikon", "JAA639DA", "PC Nikkor 19mm f/4E ED"},
      {0xA7, 0x40, 0x11, 0x26, 0x2C, 0x34, 0xD2, 0x46, 0x01, 0x00, 0x03, "Nikon", "JAA831DA",
       "AF-S Fisheye Nikkor 8-15mm f/3.5-4.5E ED"},
      {0xA7, 0x40, 0x11, 0x26, 0x2C, 0x34, 0xD2, 0x06, 0x01, 0x00, 0x03, "Nikon", "JAA831DA",
       "AF-S Fisheye Nikkor 8-15mm f/3.5-4.5E ED"},
      {0xA8, 0x38, 0x18, 0x30, 0x34, 0x3C, 0xD3, 0x8E, 0x01, 0x00, 0x03, "Nikon", "JAA832DA",
       "AF-P DX Nikkor 10-20mm f/4.5-5.6G VR"},
      {0xA8, 0x38, 0x18, 0x30, 0x34, 0x3C, 0xD3, 0x0E, 0x01, 0x00, 0x03, "Nikon", "JAA832DA",
       "AF-P DX Nikkor 10-20mm f/4.5-5.6G VR"},
      // A9                D4
      // AA                D5
      {0xAB, 0x44, 0x5C, 0x8E, 0x34, 0x3C, 0xD6, 0xCE, 0x01, 0x00, 0x03, "Nikon", "JAA833DA",
       "AF-P Nikkor 70-300mm f/4.5-5.6E ED VR"},
      {0xAB, 0x44, 0x5C, 0x8E, 0x34, 0x3C, 0xD6, 0x0E, 0x01, 0x00, 0x03, "Nikon", "JAA833DA",
       "AF-P Nikkor 70-300mm f/4.5-5.6E ED VR"},
      {0xAC, 0x54, 0x3C, 0x3C, 0x0C, 0x0C, 0xD7, 0x46, 0x01, 0x00, 0x03, "Nikon", "JAA140DA",
       "AF-S Nikkor 28mm f/1.4E ED"},
      {0xAC, 0x54, 0x3C, 0x3C, 0x0C, 0x0C, 0xD7, 0x06, 0x01, 0x00, 0x03, "Nikon", "JAA140DA",
       "AF-S Nikkor 28mm f/1.4E ED"},
      //
      //
      //
      //
      //   "AF-S DX Nikkor 18-200mm f/3.5-5.6G ED VR II"
      //
      {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA90701", "TC-16A"},
      {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA90701", "TC-16A"},
      //
      //------------------------------------------------------------------------------
      // Sigma lenses by focal length, first fixed then zoom lenses
      //------------------------------------------------------------------------------
      //
      {0xFE, 0x47, 0x00, 0x00, 0x24, 0x24, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "486556",
       "4.5mm F2.8 EX DC HSM Circular Fisheye"},
      {0x26, 0x48, 0x11, 0x11, 0x30, 0x30, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "483", "8mm F4 EX Circular Fisheye"},
      {0x79, 0x40, 0x11, 0x11, 0x2C, 0x2C, 0x1C, 0x06, 0x00, 0x00, 0x00, "Sigma", "485597",
       "8mm F3.5 EX Circular Fisheye"},
      {0xDC, 0x48, 0x19, 0x19, 0x24, 0x24, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "477554",
       "10mm F2.8 EX DC HSM Fisheye"},
      {0x02, 0x3F, 0x24, 0x24, 0x2C, 0x2C, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "468", "14mm F3.5"},
      {0x48, 0x48, 0x24, 0x24, 0x24, 0x24, 0x4B, 0x02, 0x01, 0x00, 0x00, "Sigma", "", "14mm F2.8 EX Aspherical HSM"},
      {0x26, 0x48, 0x27, 0x27, 0x24, 0x24, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "476441",
       "15mm F2.8 EX Diagonal Fisheye"},
      {0xEA, 0x48, 0x27, 0x27, 0x24, 0x24, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "15mm F2.8 EX Diagonal Fisheye"},
      // M                                         "Sigma" "410"    "18mm F3.5";
      {0x26, 0x58, 0x31, 0x31, 0x14, 0x14, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "411442",
       "20mm F1.8 EX DG Aspherical RF"},
      {0x79, 0x54, 0x31, 0x31, 0x0C, 0x0C, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "412555", "20mm F1.4 DG HSM | A"},
      {0x7E, 0x54, 0x37, 0x37, 0x0C, 0x0C, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "401559", "24mm F1.4 DG HSM | A"},
      {0x26, 0x58, 0x37, 0x37, 0x14, 0x14, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "432447",
       "24mm F1.8 EX DG Aspherical Macro"},
      {0xE1, 0x58, 0x37, 0x37, 0x14, 0x14, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "432447",
       "24mm F1.8 EX DG Aspherical Macro"},
      {0x02, 0x46, 0x37, 0x37, 0x25, 0x25, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "438",
       "24mm F2.8 Super Wide II Macro"},
      {0xBC, 0x54, 0x3C, 0x3C, 0x0C, 0x0C, 0x4B, 0x46, 0x00, 0x00, 0x00, "Sigma", "441555", "28mm F1.4 DG HSM | A"},
      {0x26, 0x58, 0x3C, 0x3C, 0x14, 0x14, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "440442",
       "28mm F1.8 EX DG Aspherical Macro"},
      {0x48, 0x54, 0x3E, 0x3E, 0x0C, 0x0C, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "477554", "30mm F1.4 EX DC HSM"},
      {0xF8, 0x54, 0x3E, 0x3E, 0x0C, 0x0C, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "477554", "30mm F1.4 EX DC HSM"},
      {0x91, 0x54, 0x44, 0x44, 0x0C, 0x0C, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "", "35mm F1.4 DG HSM"},
      {0xDE, 0x54, 0x50, 0x50, 0x0C, 0x0C, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "310554", "50mm F1.4 EX DG HSM"},
      {0x88, 0x54, 0x50, 0x50, 0x0C, 0x0C, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "311551", "50mm F1.4 DG HSM | A"},
      {0x02, 0x48, 0x50, 0x50, 0x24, 0x24, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "Macro 50mm F2.8"},
      {0x32, 0x54, 0x50, 0x50, 0x24, 0x24, 0x35, 0x02, 0x00, 0x00, 0x00, "Sigma", "346447", "Macro 50mm F2.8 EX DG"},
      {0xE3, 0x54, 0x50, 0x50, 0x24, 0x24, 0x35, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "Macro 50mm F2.8 EX DG"},
      {0x79, 0x48, 0x5C, 0x5C, 0x24, 0x24, 0x1C, 0x06, 0x00, 0x00, 0x00, "Sigma", "270599", "Macro 70mm F2.8 EX DG"},
      {0x9B, 0x54, 0x62, 0x62, 0x0C, 0x0C, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "", "85mm F1.4 EX DG HSM"},
      {0x02, 0x48, 0x65, 0x65, 0x24, 0x24, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "Macro 90mm F2.8"},
      {0x32, 0x54, 0x6A, 0x6A, 0x24, 0x24, 0x35, 0x02, 0x00, 0x02, 0x00, "Sigma", "256", "Macro 105mm F2.8 EX DG"},
      {0xE5, 0x54, 0x6A, 0x6A, 0x24, 0x24, 0x35, 0x02, 0x40, 0x00, 0x00, "Sigma", "257446", "Macro 105mm F2.8 EX DG"},
      {0x97, 0x48, 0x6A, 0x6A, 0x24, 0x24, 0x4B, 0x0E, 0x00, 0x00, 0x00, "Sigma", "", "Macro 105mm F2.8 EX DG OS HSM"},
      {0x48, 0x48, 0x76, 0x76, 0x24, 0x24, 0x4B, 0x06, 0x43, 0x00, 0x00, "Sigma", "104559",
       "APO Macro 150mm F2.8 EX DG HSM"},
      {0xF5, 0x48, 0x76, 0x76, 0x24, 0x24, 0x4B, 0x06, 0x43, 0x00, 0x00, "Sigma", "104559",
       "APO Macro 150mm F2.8 EX DG HSM"},
      {0x99, 0x48, 0x76, 0x76, 0x24, 0x24, 0x4B, 0x0E, 0x43, 0x00, 0x00, "Sigma", "",
       "APO Macro 150mm F2.8 EX DG OS HSM"},
      {0x48, 0x4C, 0x7C, 0x7C, 0x2C, 0x2C, 0x4B, 0x02, 0x43, 0x00, 0x00, "Sigma", "", "APO Macro 180mm F3.5 EX DG HSM"},
      {0x48, 0x4C, 0x7D, 0x7D, 0x2C, 0x2C, 0x4B, 0x02, 0x43, 0x00, 0x00, "Sigma", "105556",
       "APO Macro 180mm F3.5 EX DG HSM"},
      {0x94, 0x48, 0x7C, 0x7C, 0x24, 0x24, 0x4B, 0x0E, 0x43, 0x00, 0x00, "Sigma", "",
       "APO Macro 180mm F2.8 EX DG OS HSM"},
      // M                                         "Sigma" ""       "APO 300mm F2.8";
      {0x48, 0x54, 0x8E, 0x8E, 0x24, 0x24, 0x4B, 0x02, 0x03, 0x00, 0x00, "Sigma", "", "APO 300mm F2.8 EX DG HSM"},
      {0xFB, 0x54, 0x8E, 0x8E, 0x24, 0x24, 0x4B, 0x02, 0x13, 0x00, 0x00, "Sigma", "195557", "APO 300mm F2.8 EX DG HSM"},
      {0x26, 0x48, 0x8E, 0x8E, 0x30, 0x30, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "APO Tele Macro 300mm F4"},
      {0x02, 0x2F, 0x98, 0x98, 0x3D, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "APO 400mm F5.6"},
      {0x26, 0x3C, 0x98, 0x98, 0x3C, 0x3C, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "APO Tele Macro 400mm F5.6"},
      {0x02, 0x37, 0xA0, 0xA0, 0x34, 0x34, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "APO 500mm F4.5"},
      {0x48, 0x44, 0xA0, 0xA0, 0x34, 0x34, 0x4B, 0x02, 0x03, 0x00, 0x00, "Sigma", "", "APO 500mm F4.5 EX HSM"},
      {0xF1, 0x44, 0xA0, 0xA0, 0x34, 0x34, 0x4B, 0x02, 0x03, 0x00, 0x00, "Sigma", "184551", "APO 500mm F4.5 EX DG HSM"},
      {0x02, 0x34, 0xA0, 0xA0, 0x44, 0x44, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "APO 500mm F7.2"},
      {0x02, 0x3C, 0xB0, 0xB0, 0x3C, 0x3C, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "APO 800mm F5.6"},
      {0x48, 0x3C, 0xB0, 0xB0, 0x3C, 0x3C, 0x4B, 0x02, 0x03, 0x00, 0x00, "Sigma", "", "APO 800mm F5.6 EX HSM"},
      // M                                         "Sigma" "152550" "APO 800mm F5.6 EX DG HSM";
      // M2    B8 B8 49 49 02" "00" "00" "00" "00" "Sigma" ""       "APO 1000mm F8.0";
      //
      {0x9E, 0x38, 0x11, 0x29, 0x34, 0x3C, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "", "8-16mm F4.5-5.6 DC HSM"},
      {0xA1, 0x41, 0x19, 0x31, 0x2C, 0x2C, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "", "10-20mm F3.5 EX DC HSM"},
      {0x48, 0x3C, 0x19, 0x31, 0x30, 0x3C, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "201555", "10-20mm F4-5.6 EX DC HSM"},
      {0xF9, 0x3C, 0x19, 0x31, 0x30, 0x3C, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "201555", "10-20mm F4-5.6 EX DC HSM"},
      {0xCA, 0x3C, 0x1F, 0x37, 0x30, 0x30, 0x4B, 0x46, 0x01, 0x00, 0x00, "Sigma", "205955", "12-24mm F4 DG HSM | A"},
      {0x48, 0x38, 0x1F, 0x37, 0x34, 0x3C, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "200558",
       "12-24mm F4.5-5.6 EX DG Aspherical HSM"},
      {0xF0, 0x38, 0x1F, 0x37, 0x34, 0x3C, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "200558",
       "12-24mm F4.5-5.6 EX DG Aspherical HSM"},
      {0x96, 0x38, 0x1F, 0x37, 0x34, 0x3C, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "", "12-24mm F4.5-5.6 II DG HSM"},
      {0xC1, 0x48, 0x24, 0x37, 0x24, 0x24, 0x4B, 0x46, 0x00, 0x00, 0x00, "Sigma", "", "14-24mm F2.8 DG HSM | A"},
      {0x26, 0x40, 0x27, 0x3F, 0x2C, 0x34, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "",
       "15-30mm F3.5-4.5 EX DG Aspherical DF"},
      {0x48, 0x48, 0x2B, 0x44, 0x24, 0x30, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "",
       "17-35mm F2.8-4 EX DG Aspherical HSM"},
      {0x26, 0x54, 0x2B, 0x44, 0x24, 0x30, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "17-35mm F2.8-4 EX Aspherical"},
      {0x9D, 0x48, 0x2B, 0x50, 0x24, 0x24, 0x4B, 0x0E, 0x00, 0x00, 0x00, "Sigma", "", "17-50mm F2.8 EX DC OS HSM"},
      {0x8F, 0x48, 0x2B, 0x50, 0x24, 0x24, 0x4B, 0x0E, 0x00, 0x00, 0x00, "Sigma", "", "17-50mm F2.8 EX DC OS HSM"},
      {0x7A, 0x47, 0x2B, 0x5C, 0x24, 0x34, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "689599",
       "17-70mm F2.8-4.5 DC Macro Asp. IF HSM"},
      {0x7A, 0x48, 0x2B, 0x5C, 0x24, 0x34, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "689599",
       "17-70mm F2.8-4.5 DC Macro Asp. IF HSM"},
      {0x7F, 0x48, 0x2B, 0x5C, 0x24, 0x34, 0x1C, 0x06, 0x00, 0x00, 0x00, "Sigma", "",
       "17-70mm F2.8-4.5 DC Macro Asp. IF"},
      {0x8E, 0x3C, 0x2B, 0x5C, 0x24, 0x30, 0x4B, 0x0E, 0x00, 0x00, 0x00, "Sigma", "",
       "17-70mm F2.8-4 DC Macro OS HSM | C"},
      {0xA0, 0x48, 0x2A, 0x5C, 0x24, 0x30, 0x4B, 0x0E, 0x00, 0x00, 0xFE, "Sigma", "", "17-70mm F2.8-4 DC Macro OS HSM"},
      {0x8B, 0x4C, 0x2D, 0x44, 0x14, 0x14, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "", "18-35mm F1.8 DC HSM"},
      {0x26, 0x40, 0x2D, 0x44, 0x2B, 0x34, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "18-35mm F3.5-4.5 Aspherical"},
      {0x26, 0x48, 0x2D, 0x50, 0x24, 0x24, 0x1C, 0x06, 0x00, 0x00, 0x00, "Sigma", "", "18-50mm F2.8 EX DC"},
      {0x7F, 0x48, 0x2D, 0x50, 0x24, 0x24, 0x1C, 0x06, 0x00, 0x00, 0x00, "Sigma", "", "18-50mm F2.8 EX DC Macro"},
      {0x7A, 0x48, 0x2D, 0x50, 0x24, 0x24, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "582593", "18-50mm F2.8 EX DC Macro"},
      {0xF6, 0x48, 0x2D, 0x50, 0x24, 0x24, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "582593", "18-50mm F2.8 EX DC Macro"},
      {0xA4, 0x47, 0x2D, 0x50, 0x24, 0x34, 0x4B, 0x0E, 0x01, 0x00, 0x00, "Sigma", "", "18-50mm F2.8-4.5 DC OS HSM"},
      {0x26, 0x40, 0x2D, 0x50, 0x2C, 0x3C, 0x1C, 0x06, 0x00, 0x00, 0x00, "Sigma", "", "18-50mm F3.5-5.6 DC"},
      {0x7A, 0x40, 0x2D, 0x50, 0x2C, 0x3C, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "551551", "18-50mm F3.5-5.6 DC HSM"},
      {0x26, 0x40, 0x2D, 0x70, 0x2B, 0x3C, 0x1C, 0x06, 0x00, 0x00, 0x00, "Sigma", "", "18-125mm F3.5-5.6 DC"},
      {0xCD, 0x3D, 0x2D, 0x70, 0x2E, 0x3C, 0x4B, 0x0E, 0x01, 0x00, 0x00, "Sigma", "853556",
       "18-125mm F3.8-5.6 DC OS HSM"},
      {0x26, 0x40, 0x2D, 0x80, 0x2C, 0x40, 0x1C, 0x06, 0x00, 0x00, 0x00, "Sigma", "777555", "18-200mm F3.5-6.3 DC"},
      {0xFF, 0x40, 0x2D, 0x80, 0x2C, 0x40, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "", "18-200mm F3.5-6.3 DC"},
      {0x7A, 0x40, 0x2D, 0x80, 0x2C, 0x40, 0x4B, 0x0E, 0x01, 0x00, 0x00, "Sigma", "888558",
       "18-200mm F3.5-6.3 DC OS HSM"},
      {0xED, 0x40, 0x2D, 0x80, 0x2C, 0x40, 0x4B, 0x0E, 0x01, 0x00, 0x00, "Sigma", "888558",
       "18-200mm F3.5-6.3 DC OS HSM"},
      {0x90, 0x40, 0x2D, 0x80, 0x2C, 0x40, 0x4B, 0x0E, 0x01, 0x00, 0x00, "Sigma", "", "18-200mm F3.5-6.3 II DC OS HSM"},
      {0xA5, 0x40, 0x2D, 0x88, 0x2C, 0x40, 0x4B, 0x0E, 0x01, 0x00, 0x00, "Sigma", "", "18-250mm F3.5-6.3 DC OS HSM"},
      {0x92, 0x39, 0x2D, 0x88, 0x2C, 0x40, 0x4B, 0x0E, 0x01, 0x00, 0x00, "Sigma", "",
       "18-250mm F3.5-6.3 DC OS Macro HSM"},
      {0x26, 0x48, 0x31, 0x49, 0x24, 0x24, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "20-40mm F2.8"},
      {0x02, 0x3A, 0x37, 0x50, 0x31, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "24-50mm F4-5.6 UC"},
      {0x26, 0x48, 0x37, 0x56, 0x24, 0x24, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "547448", "24-60mm F2.8 EX DG"},
      {0xB6, 0x48, 0x37, 0x56, 0x24, 0x24, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "547448", "24-60mm F2.8 EX DG"},
      {0xA6, 0x48, 0x37, 0x5C, 0x24, 0x24, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "571559",
       "24-70mm F2.8 IF EX DG HSM"},
      {0x26, 0x54, 0x37, 0x5C, 0x24, 0x24, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "24-70mm F2.8 EX DG Macro"},
      {0x67, 0x54, 0x37, 0x5C, 0x24, 0x24, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "548445", "24-70mm F2.8 EX DG Macro"},
      {0xE9, 0x54, 0x37, 0x5C, 0x24, 0x24, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "548445", "24-70mm F2.8 EX DG Macro"},
      {0x26, 0x40, 0x37, 0x5C, 0x2C, 0x3C, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "24-70mm F3.5-5.6 Aspherical HF"},
      {0x8A, 0x3C, 0x37, 0x6A, 0x30, 0x30, 0x4B, 0x0E, 0x00, 0x00, 0x00, "Sigma", "", "24-105mm F4 DG OS HSM"},
      {0x26, 0x54, 0x37, 0x73, 0x24, 0x34, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "24-135mm F2.8-4.5"},
      {0x02, 0x46, 0x3C, 0x5C, 0x25, 0x25, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "28-70mm F2.8"},
      {0x26, 0x54, 0x3C, 0x5C, 0x24, 0x24, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "28-70mm F2.8 EX"},
      {0x26, 0x48, 0x3C, 0x5C, 0x24, 0x24, 0x1C, 0x06, 0x00, 0x00, 0x00, "Sigma", "549442", "28-70mm F2.8 EX DG"},
      {0x79, 0x48, 0x3C, 0x5C, 0x24, 0x24, 0x1C, 0x06, 0x00, 0x00, 0x00, "Sigma", "", "28-70mm F2.8 EX DG"},
      {0x26, 0x48, 0x3C, 0x5C, 0x24, 0x30, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "634445", "28-70mm F2.8-4 DG"},
      {0x02, 0x3F, 0x3C, 0x5C, 0x2D, 0x35, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "28-70mm F3.5-4.5 UC"},
      {0x26, 0x40, 0x3C, 0x60, 0x2C, 0x3C, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "",
       "28-80mm F3.5-5.6 Mini Zoom Macro II Aspherical"},
      {0x26, 0x40, 0x3C, 0x65, 0x2C, 0x3C, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "28-90mm F3.5-5.6 Macro"},
      {0x26, 0x48, 0x3C, 0x6A, 0x24, 0x30, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "28-105mm F2.8-4 Aspherical"},
      {0x26, 0x3E, 0x3C, 0x6A, 0x2E, 0x3C, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "",
       "28-105mm F3.8-5.6 UC-III Aspherical IF"},
      {0x26, 0x40, 0x3C, 0x80, 0x2C, 0x3C, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "",
       "28-200mm F3.5-5.6 Compact Aspherical Hyperzoom Macro"},
      {0x26, 0x40, 0x3C, 0x80, 0x2B, 0x3C, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "",
       "28-200mm F3.5-5.6 Compact Aspherical Hyperzoom Macro"},
      {0x26, 0x3D, 0x3C, 0x80, 0x2F, 0x3D, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "28-300mm F3.8-5.6 Aspherical"},
      {0x26, 0x41, 0x3C, 0x8E, 0x2C, 0x40, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "795443",
       "28-300mm F3.5-6.3 DG Macro"},
      {0xE6, 0x41, 0x3C, 0x8E, 0x2C, 0x40, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "28-300mm F3.5-6.3 DG Macro"},
      {0x26, 0x40, 0x3C, 0x8E, 0x2C, 0x40, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "28-300mm F3.5-6.3 Macro"},
      {0x02, 0x3B, 0x44, 0x61, 0x30, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "35-80mm F4-5.6"},
      {0x02, 0x40, 0x44, 0x73, 0x2B, 0x36, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "35-135mm F3.5-4.5 a"},
      {0xCC, 0x4C, 0x50, 0x68, 0x14, 0x14, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "", "50-100mm F1.8 DC HSM | A"},
      {0x7A, 0x47, 0x50, 0x76, 0x24, 0x24, 0x4B, 0x06, 0x03, 0x00, 0x00, "Sigma", "", "50-150mm F2.8 EX APO DC HSM"},
      {0xFD, 0x47, 0x50, 0x76, 0x24, 0x24, 0x4B, 0x06, 0x03, 0x00, 0x00, "Sigma", "691554",
       "50-150mm F2.8 EX APO DC HSM II"},
      {0x98, 0x48, 0x50, 0x76, 0x24, 0x24, 0x4B, 0x0E, 0x00, 0x00, 0x00, "Sigma", "", "50-150mm F2.8 EX DC APO OS HSM"},
      {0x48, 0x3C, 0x50, 0xA0, 0x30, 0x40, 0x4B, 0x02, 0x03, 0x00, 0x00, "Sigma", "736552",
       "APO 50-500mm F4-6.3 EX HSM"},
      {0x9F, 0x37, 0x50, 0xA0, 0x34, 0x40, 0x4B, 0x0E, 0x03, 0x00, 0x00, "Sigma", "",
       "50-500mm F4.5-6.3 APO DG OS HSM"},
      // M                                         "Sigma" "686550" "50-200mm F4-5.6 DC OS HSM";
      {0x26, 0x3C, 0x54, 0x80, 0x30, 0x3C, 0x1C, 0x06, 0x00, 0x00, 0x00, "Sigma", "", "55-200mm F4-5.6 DC"},
      {0x7A, 0x3B, 0x53, 0x80, 0x30, 0x3C, 0x4B, 0x06, 0x01, 0x00, 0x00, "Sigma", "", "55-200mm F4-5.6 DC HSM"},
      {0xBF, 0x38, 0x56, 0xA6, 0x34, 0x40, 0x4B, 0x4E, 0x00, 0x00, 0x00, "Sigma", "",
       "60-600mm F4.5-6.3 DG OS HSM | S"},
      {0x48, 0x54, 0x5C, 0x80, 0x24, 0x24, 0x4B, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "70-200mm F2.8 EX APO IF HSM"},
      {0x7A, 0x48, 0x5C, 0x80, 0x24, 0x24, 0x4B, 0x06, 0x03, 0x00, 0x00, "Sigma", "",
       "70-200mm F2.8 EX APO DG Macro HSM II"},
      {0xEE, 0x48, 0x5C, 0x80, 0x24, 0x24, 0x4B, 0x06, 0x03, 0x00, 0x00, "Sigma", "579555",
       "70-200mm F2.8 EX APO DG Macro HSM II"},
      {0x9C, 0x48, 0x5C, 0x80, 0x24, 0x24, 0x4B, 0x0E, 0x03, 0x00, 0x00, "Sigma", "", "70-200mm F2.8 EX DG OS HSM"},
      {0x02, 0x46, 0x5C, 0x82, 0x25, 0x25, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "70-210mm F2.8 APO"},
      {0x02, 0x40, 0x5C, 0x82, 0x2C, 0x35, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "APO 70-210mm F3.5-4.5"},
      {0x26, 0x3C, 0x5C, 0x82, 0x30, 0x3C, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "70-210mm F4-5.6 UC-II"},
      {0x26, 0x3C, 0x5C, 0x8E, 0x30, 0x3C, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "70-300mm F4-5.6 DG Macro"},
      {0x56, 0x3C, 0x5C, 0x8E, 0x30, 0x3C, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "",
       "70-300mm F4-5.6 APO Macro Super II"},
      {0xE0, 0x3C, 0x5C, 0x8E, 0x30, 0x3C, 0x4B, 0x06, 0x00, 0x00, 0x00, "Sigma", "508555",
       "APO 70-300mm F4-5.6 DG Macro"},
      {0xA3, 0x3C, 0x5C, 0x8E, 0x30, 0x3C, 0x4B, 0x0E, 0x00, 0x00, 0x00, "Sigma", "572556", "70-300mm F4-5.6 DG OS"},
      {0x02, 0x37, 0x5E, 0x8E, 0x35, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "75-300mm F4.5-5.6 APO"},
      {0x02, 0x3A, 0x5E, 0x8E, 0x32, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, "Sigma", "", "75-300mm F4.0-5.6"},
      {0x77, 0x44, 0x61, 0x98, 0x34, 0x3C, 0x7B, 0x0E, 0x03, 0x00, 0x00, "Sigma", "", "80-400mm f4.5-5.6 EX OS"},
      {0x77, 0x44, 0x60, 0x98, 0x34, 0x3C, 0x7B, 0x0E, 0x03, 0x00, 0x00, "Sigma", "", "80-400mm f4.5-5.6 APO DG D OS"},
      {0x48, 0x48, 0x68, 0x8E, 0x30, 0x30, 0x4B, 0x02, 0x03, 0x00, 0x00, "Sigma", "134556",
       "APO 100-300mm F4 EX IF HSM"},
      {0xF3, 0x48, 0x68, 0x8E, 0x30, 0x30, 0x4B, 0x02, 0x13, 0x00, 0x00, "Sigma", "134556",
       "APO 100-300mm F4 EX IF HSM"},
      {0x48, 0x54, 0x6F, 0x8E, 0x24, 0x24, 0x4B, 0x02, 0x03, 0x00, 0x00, "Sigma", "", "APO 120-300mm F2.8 EX DG HSM"},
      {0x7A, 0x54, 0x6E, 0x8E, 0x24, 0x24, 0x4B, 0x02, 0x03, 0x00, 0x00, "Sigma", "135553",
       "APO 120-300mm F2.8 EX DG HSM"},
      {0xFA, 0x54, 0x6E, 0x8E, 0x24, 0x24, 0x4B, 0x02, 0x03, 0x00, 0x00, "Sigma", "135553",
       "APO 120-300mm F2.8 EX DG HSM"},
      {0xCF, 0x38, 0x6E, 0x98, 0x34, 0x3C, 0x4B, 0x0E, 0x03, 0x00, 0x00, "Sigma", "728557",
       "APO 120-400mm F4.5-5.6 DG OS HSM"},
      {0xC3, 0x34, 0x68, 0x98, 0x38, 0x40, 0x4B, 0x4E, 0x03, 0x00, 0x00, "Sigma", "", "100-400mm F5-6.3 DG OS HSM | C"},
      {0x8D, 0x48, 0x6E, 0x8E, 0x24, 0x24, 0x4B, 0x0E, 0x03, 0x00, 0x00, "Sigma", "", "120-300mm F2.8 DG OS HSM | S"},
      {0x26, 0x44, 0x73, 0x98, 0x34, 0x3C, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "",
       "135-400mm F4.5-5.6 APO Aspherical"},
      {0xCE, 0x34, 0x76, 0xA0, 0x38, 0x40, 0x4B, 0x0E, 0x03, 0x00, 0x00, "Sigma", "737559",
       "APO 150-500mm F5-6.3 DG OS HSM"},
      {0x81, 0x34, 0x76, 0xA6, 0x38, 0x40, 0x4B, 0x0E, 0x03, 0x00, 0x00, "Sigma", "", "150-600mm F5-6.3 DG OS HSM | S"},
      {0x82, 0x34, 0x76, 0xA6, 0x38, 0x40, 0x4B, 0x0E, 0x03, 0x00, 0x00, "Sigma", "", "150-600mm F5-6.3 DG OS HSM | C"},
      {0x26, 0x40, 0x7B, 0xA0, 0x34, 0x40, 0x1C, 0x02, 0x00, 0x00, 0x00, "Sigma", "",
       "APO 170-500mm F5-6.3 Aspherical RF"},
      {0xA7, 0x49, 0x80, 0xA0, 0x24, 0x24, 0x4B, 0x06, 0x03, 0x00, 0x00, "Sigma", "", "APO 200-500mm F2.8 EX DG"},
      {0x48, 0x3C, 0x8E, 0xB0, 0x3C, 0x3C, 0x4B, 0x02, 0x03, 0x00, 0x00, "Sigma", "595555",
       "APO 300-800mm F5.6 EX DG HSM"},
      //
      //------------------------------------------------------------------------------
      // Tamron lenses by focal length, first fixed then zoom lenses
      //------------------------------------------------------------------------------
      //
      {0x00, 0x47, 0x25, 0x25, 0x24, 0x24, 0x00, 0x02, 0x00, 0x00, 0x00, "Tamron", "69E",
       "SP AF 14mm F/2.8 Aspherical (IF)"},
      {0xE8, 0x4C, 0x44, 0x44, 0x14, 0x14, 0xDF, 0x0E, 0x00, 0x00, 0x00, "Tamron", "F012", "SP 35mm F/1.8 Di VC USD"},
      {0xE7, 0x4C, 0x4C, 0x4C, 0x14, 0x14, 0xDF, 0x0E, 0x00, 0x00, 0x00, "Tamron", "F013", "SP 45mm F/1.8 Di VC USD"},
      {0xF4, 0x54, 0x56, 0x56, 0x18, 0x18, 0x84, 0x06, 0x01, 0x00, 0x00, "Tamron", "G005",
       "SP AF 60mm F/2 Di II LD (IF) Macro 1:1"},
      {0x1E, 0x5D, 0x64, 0x64, 0x20, 0x20, 0x13, 0x00, 0x40, 0x00, 0x00, "Tamron", "52E", "SP AF 90mm F/2.5"},
      {0x20, 0x5A, 0x64, 0x64, 0x20, 0x20, 0x14, 0x00, 0x40, 0x00, 0x00, "Tamron", "152E", "SP AF 90mm F/2.5 Macro"},
      {0x22, 0x53, 0x64, 0x64, 0x24, 0x24, 0xE0, 0x02, 0x40, 0x00, 0x00, "Tamron", "72E", "SP AF 90mm F/2.8 Macro 1:1"},
      {0x32, 0x53, 0x64, 0x64, 0x24, 0x24, 0x35, 0x02, 0x40, 0x01, 0x00, "Tamron", "172E",
       "SP AF 90mm F/2.8 Macro 1:1"},
      {0x32, 0x53, 0x64, 0x64, 0x24, 0x24, 0x35, 0x02, 0x40, 0x02, 0x00, "Tamron", "272E",
       "SP AF 90mm F/2.8 Di Macro 1:1"},
      {0xF8, 0x55, 0x64, 0x64, 0x24, 0x24, 0x84, 0x06, 0x41, 0x00, 0x00, "Tamron", "272NII",
       "SP AF 90mm F/2.8 Di Macro 1:1"},
      {0xF8, 0x54, 0x64, 0x64, 0x24, 0x24, 0xDF, 0x06, 0x41, 0x00, 0x00, "Tamron", "272NII",
       "SP AF 90mm F/2.8 Di Macro 1:1"},
      {0xFE, 0x54, 0x64, 0x64, 0x24, 0x24, 0xDF, 0x0E, 0x00, 0x00, 0x00, "Tamron", "F004",
       "SP 90mm F/2.8 Di VC USD Macro 1:1"},
      {0xE4, 0x54, 0x64, 0x64, 0x24, 0x24, 0xDF, 0x0E, 0x00, 0x00, 0x00, "Tamron", "F017",
       "SP 90mm F/2.8 Di VC USD Macro 1:1"},
      {0x00, 0x4C, 0x7C, 0x7C, 0x2C, 0x2C, 0x00, 0x02, 0x00, 0x00, 0x00, "Tamron", "B01", "SP AF 180mm F/3.5 Di Model"},
      {0x21, 0x56, 0x8E, 0x8E, 0x24, 0x24, 0x14, 0x00, 0x00, 0x00, 0x00, "Tamron", "60E", "SP AF 300mm F/2.8 LD-IF"},
      {0x27, 0x54, 0x8E, 0x8E, 0x24, 0x24, 0x1D, 0x02, 0x00, 0x00, 0x00, "Tamron", "360E", "SP AF 300mm F/2.8 LD-IF"},
      //
      {0xF6, 0x3F, 0x18, 0x37, 0x2C, 0x34, 0x84, 0x06, 0x01, 0x00, 0x00, "Tamron", "B001",
       "SP AF 10-24mm F/3.5-4.5 Di II LD Aspherical (IF)"},
      {0xF6, 0x3F, 0x18, 0x37, 0x2C, 0x34, 0xDF, 0x06, 0x01, 0x00, 0x00, "Tamron", "B001",
       "SP AF 10-24mm F/3.5-4.5 Di II LD Aspherical (IF)"},
      {0x00, 0x36, 0x1C, 0x2D, 0x34, 0x3C, 0x00, 0x06, 0x00, 0x00, 0x00, "Tamron", "A13",
       "SP AF 11-18mm F/4.5-5.6 Di II LD Aspherical (IF)"},
      {0xCA, 0x48, 0x27, 0x3E, 0x24, 0x24, 0xDF, 0x4E, 0x00, 0x00, 0x00, "Tamron", "A041",
       "SP 15-30mm F/2.8 Di VC USD G2"},
      {0xE9, 0x48, 0x27, 0x3E, 0x24, 0x24, 0xDF, 0x0E, 0x00, 0x00, 0x00, "Tamron", "A012",
       "SP 15-30mm F/2.8 Di VC USD"},
      {0xEA, 0x40, 0x29, 0x8E, 0x2C, 0x40, 0xDF, 0x0E, 0x00, 0x00, 0x00, "Tamron", "B016",
       "16-300mm F/3.5-6.3 Di II VC PZD Macro"},
      {0x07, 0x46, 0x2B, 0x44, 0x24, 0x30, 0x03, 0x02, 0x00, 0x00, 0x00, "Tamron", "A05",
       "SP AF 17-35mm F/2.8-4 Di LD Aspherical (IF)"},
      {0x00, 0x53, 0x2B, 0x50, 0x24, 0x24, 0x00, 0x06, 0x00, 0x00, 0x00, "Tamron", "A16",
       "SP AF 17-50mm F/2.8 XR Di II LD Aspherical (IF)"},
      {0x00, 0x54, 0x2B, 0x50, 0x24, 0x24, 0x00, 0x06, 0x01, 0x00, 0x00, "Tamron", "A16NII",
       "SP AF 17-50mm F/2.8 XR Di II LD Aspherical (IF)"},
      {0xFB, 0x54, 0x2B, 0x50, 0x24, 0x24, 0x84, 0x06, 0x01, 0x00, 0x00, "Tamron", "A16NII",
       "SP AF 17-50mm F/2.8 XR Di II LD Aspherical (IF)"},
      {0xF3, 0x54, 0x2B, 0x50, 0x24, 0x24, 0x84, 0x0E, 0x01, 0x00, 0x00, "Tamron", "B005",
       "SP AF 17-50mm F/2.8 XR Di II VC LD Aspherical (IF)"},
      {0x00, 0x3F, 0x2D, 0x80, 0x2B, 0x40, 0x00, 0x06, 0x00, 0x00, 0x00, "Tamron", "A14",
       "AF 18-200mm F/3.5-6.3 XR Di II LD Aspherical (IF)"},
      {0x00, 0x3F, 0x2D, 0x80, 0x2C, 0x40, 0x00, 0x06, 0x00, 0x00, 0x00, "Tamron", "A14",
       "AF 18-200mm F/3.5-6.3 XR Di II LD Aspherical (IF) Macro"},
      {0x00, 0x40, 0x2D, 0x80, 0x2C, 0x40, 0x00, 0x06, 0x01, 0x00, 0x00, "Tamron", "A14NII",
       "AF 18-200mm F/3.5-6.3 XR Di II LD Aspherical (IF) Macro"},
      {0xFC, 0x40, 0x2D, 0x80, 0x2C, 0x40, 0xDF, 0x06, 0x01, 0x00, 0x00, "Tamron", "A14NII",
       "AF 18-200mm F/3.5-6.3 XR Di II LD Aspherical (IF) Macro"},
      {0xE6, 0x40, 0x2D, 0x80, 0x2C, 0x40, 0xDF, 0x0E, 0x01, 0x00, 0x00, "Tamron", "B018",
       "18-200mm F/3.5-6.3 Di II VC"},
      {0x00, 0x40, 0x2D, 0x88, 0x2C, 0x40, 0x62, 0x06, 0x00, 0x00, 0x00, "Tamron", "A18",
       "AF 18-250mm F/3.5-6.3 Di II LD Aspherical (IF) Macro"},
      {0x00, 0x40, 0x2D, 0x88, 0x2C, 0x40, 0x00, 0x06, 0x01, 0x00, 0x00, "Tamron", "A18NII",
       "AF 18-250mm F/3.5-6.3 Di II LD Aspherical (IF) Macro "},
      {0xF5, 0x40, 0x2C, 0x8A, 0x2C, 0x40, 0x40, 0x0E, 0x01, 0x00, 0x00, "Tamron", "B003",
       "AF 18-270mm F/3.5-6.3 Di II VC LD Aspherical (IF) Macro"},
      {0xF0, 0x3F, 0x2D, 0x8A, 0x2C, 0x40, 0xDF, 0x0E, 0x01, 0x00, 0x00, "Tamron", "B008",
       "AF 18-270mm F/3.5-6.3 Di II VC PZD"},
      {0xE0, 0x40, 0x2D, 0x98, 0x2C, 0x41, 0xDF, 0x4E, 0x01, 0x00, 0x00, "Tamron", "B028",
       "18-400mm F/3.5-6.3 Di II VC HLD"},
      {0x07, 0x40, 0x2F, 0x44, 0x2C, 0x34, 0x03, 0x02, 0x00, 0x00, 0x00, "Tamron", "A10", "AF 19-35mm F/3.5-4.5"},
      {0x07, 0x40, 0x30, 0x45, 0x2D, 0x35, 0x03, 0x02, 0x00, 0x00, 0x00, "Tamron", "A10", "AF 19-35mm F/3.5-4.5"},
      {0x00, 0x49, 0x30, 0x48, 0x22, 0x2B, 0x00, 0x02, 0x00, 0x00, 0x00, "Tamron", "166D", "SP AF 20-40mm F/2.7-3.5"},
      {0x0E, 0x4A, 0x31, 0x48, 0x23, 0x2D, 0x0E, 0x02, 0x00, 0x00, 0x00, "Tamron", "166D", "SP AF 20-40mm F/2.7-3.5"},
      // M                                         "Tamron" "266D"   "SP AF 20-40mm F/2.7-3.5 Aspherical-IF";
      {0xFE, 0x48, 0x37, 0x5C, 0x24, 0x24, 0xDF, 0x0E, 0x01, 0x00, 0x00, "Tamron", "A007",
       "SP 24-70mm F/2.8 Di VC USD"},
      // M                                         "Tamron" "73D"    "AF 24-70mm F/3.3-5.6 Aspherical";
      {0xCE, 0x47, 0x37, 0x5C, 0x25, 0x25, 0xDF, 0x4E, 0x00, 0x00, 0x00, "Tamron", "A032",
       "SP 24-70mm F/2.8 Di VC USD G2"},
      {0xCE, 0x47, 0x37, 0x5C, 0x25, 0x25, 0xDF, 0x0E, 0x00, 0x00, 0x00, "Tamron", "A032",
       "SP 24-70mm F/2.8 Di VC USD G2"},
      {0x45, 0x41, 0x37, 0x72, 0x2C, 0x3C, 0x48, 0x02, 0x00, 0x00, 0x00, "Tamron", "190D",
       "SP AF 24-135mm F/3.5-5.6 AD Aspherical (IF) Macro"},
      // M                                         "Tamron" "159D"   "AF 28-70mm F/3.5-4.5";
      // M                                         "Tamron" "259D"   "AF 28-70mm F/3.5-4.5";
      {0x33, 0x54, 0x3C, 0x5E, 0x24, 0x24, 0x62, 0x02, 0x00, 0x00, 0x00, "Tamron", "A09",
       "SP AF 28-75mm F/2.8 XR Di LD Aspherical (IF) Macro"},
      {0xFA, 0x54, 0x3C, 0x5E, 0x24, 0x24, 0x84, 0x06, 0x01, 0x00, 0x00, "Tamron", "A09NII",
       "SP AF 28-75mm F/2.8 XR Di LD Aspherical (IF) Macro"},
      {0xFA, 0x54, 0x3C, 0x5E, 0x24, 0x24, 0xDF, 0x06, 0x01, 0x00, 0x00, "Tamron", "A09NII",
       "SP AF 28-75mm F/2.8 XR Di LD Aspherical (IF) Macro"},
      {0x10, 0x3D, 0x3C, 0x60, 0x2C, 0x3C, 0xD2, 0x02, 0x00, 0x00, 0x00, "Tamron", "177D",
       "AF 28-80mm F/3.5-5.6 Aspherical"},
      {0x45, 0x3D, 0x3C, 0x60, 0x2C, 0x3C, 0x48, 0x02, 0x00, 0x00, 0x00, "Tamron", "177D",
       "AF 28-80mm F/3.5-5.6 Aspherical"},
      {0x00, 0x48, 0x3C, 0x6A, 0x24, 0x24, 0x00, 0x02, 0x00, 0x00, 0x00, "Tamron", "176D",
       "SP AF 28-105mm F/2.8 LD Aspherical IF"},
      // M                                         "Tamron" "276D"   "SP AF 28-105mm F/2.8 LD Aspherical IF";
      // M                                         "Tamron" "179D"   "AF 28-105mm F4.0-5.6 IF";
      {0x4D, 0x3E, 0x3C, 0x80, 0x2E, 0x3C, 0x62, 0x02, 0x00, 0x00, 0x00, "Tamron", "A03N",
       "AF 28-200mm F/3.8-5.6 XR Aspherical (IF) Macro"},
      // M                                         "Tamron" "471D"   "AF 28-200mm F/3.8-5.6 Aspherical IF Super2
      // Silver";
      {0x0B, 0x3E, 0x3D, 0x7F, 0x2F, 0x3D, 0x0E, 0x00, 0x00, 0x00, 0x00, "Tamron", "71D", "AF 28-200mm F/3.8-5.6"},
      {0x0B, 0x3E, 0x3D, 0x7F, 0x2F, 0x3D, 0x0E, 0x02, 0x00, 0x00, 0x00, "Tamron", "171D", "AF 28-200mm F/3.8-5.6D"},
      {0x12, 0x3D, 0x3C, 0x80, 0x2E, 0x3C, 0xDF, 0x02, 0x00, 0x00, 0x00, "Tamron", "271D",
       "AF 28-200mm F/3.8-5.6 LD Aspherical (IF)"},
      {0x4D, 0x41, 0x3C, 0x8E, 0x2B, 0x40, 0x62, 0x02, 0x00, 0x00, 0x00, "Tamron", "A061",
       "AF 28-300mm F/3.5-6.3 XR Di LD Aspherical (IF)"},
      {0x4D, 0x41, 0x3C, 0x8E, 0x2C, 0x40, 0x62, 0x02, 0x00, 0x00, 0x00, "Tamron", "185D",
       "AF 28-300mm F/3.5-6.3 XR LD Aspherical (IF)"},
      // M                                         "Tamron" "285D"   "AF 28-300mm F/3.8-6.3 LD Aspherical IF
      // Silver";
      {0xF9, 0x40, 0x3C, 0x8E, 0x2C, 0x40, 0x40, 0x0E, 0x01, 0x00, 0x00, "Tamron", "A20",
       "AF 28-300mm F/3.5-6.3 XR Di VC LD Aspherical (IF) Macro"},
      // M                                         "Tamron" "63D"    "AF 35-90mm F/4-5.6";
      // M                                         "Tamron" "65D"    "SP AF 35-105mm F/2.8 Aspherical";
      // M                                         "Tamron" ""       "AF 35-135mm F/3.5-4.5";
      {0xC9, 0x3C, 0x44, 0x76, 0x25, 0x31, 0xDF, 0x4E, 0x00, 0x00, 0x00, "Tamron", "A043",
       "35-150mm F/2.8-4 Di VC OSD"},
      {0x00, 0x47, 0x53, 0x80, 0x30, 0x3C, 0x00, 0x06, 0x00, 0x00, 0x00, "Tamron", "A15",
       "AF 55-200mm F/4-5.6 Di II LD"},
      {0xF7, 0x53, 0x5C, 0x80, 0x24, 0x24, 0x84, 0x06, 0x01, 0x00, 0x00, "Tamron", "A001",
       "SP AF 70-200mm F/2.8 Di LD (IF) Macro"},
      {0xFE, 0x53, 0x5C, 0x80, 0x24, 0x24, 0x84, 0x06, 0x01, 0x00, 0x00, "Tamron", "A001",
       "SP AF 70-200mm F/2.8 Di LD (IF) Macro"},
      {0xF7, 0x53, 0x5C, 0x80, 0x24, 0x24, 0x40, 0x06, 0x01, 0x00, 0x00, "Tamron", "A001",
       "SP AF 70-200mm F/2.8 Di LD (IF) Macro"},
      {0xFE, 0x54, 0x5C, 0x80, 0x24, 0x24, 0xDF, 0x0E, 0x01, 0x00, 0x00, "Tamron", "A009",
       "SP 70-200mm F/2.8 Di VC USD"},
      {0xE2, 0x47, 0x5C, 0x80, 0x24, 0x24, 0xDF, 0x4E, 0x00, 0x00, 0x00, "Tamron", "A025",
       "SP 70-200mm F/2.8 Di VC USD G2"},
      {0xCD, 0x47, 0x5C, 0x82, 0x31, 0x31, 0xDF, 0x4E, 0x00, 0x00, 0x00, "Tamron", "A034", "70-210mm F/4 Di VC USD"},
      // M                                         "Tamron" "67D"    "SP AF 70-210mm f/2.8 LD";
      // M                                         "Tamron" ""       "AF 70-210mm F/3.5-4.5";
      // M                                         "Tamron" "158D"   "AF 70-210mm F/4-5.6";
      // M                                         "Tamron" "258D"   "AF 70-210mm F/4-5.6";
      // M                                         "Tamron" "172D"   "AF 70-300mm F/4-5.6";
      // M                                         "Tamron" "472D"   "AF 70-300mm F/4-5.6 LD";
      {0x69, 0x48, 0x5C, 0x8E, 0x30, 0x3C, 0x6F, 0x02, 0x00, 0x00, 0x00, "Tamron", "572D/772D",
       "AF 70-300mm F/4-5.6 LD Macro 1:2"},
      {0x69, 0x47, 0x5C, 0x8E, 0x30, 0x3C, 0x00, 0x02, 0x00, 0x00, 0x00, "Tamron", "A17N",
       "AF 70-300mm F/4-5.6 Di LD Macro 1:2"},
      {0x00, 0x48, 0x5C, 0x8E, 0x30, 0x3C, 0x00, 0x06, 0x01, 0x00, 0x00, "Tamron", "A17NII",
       "AF 70-300mm F/4-5.6 Di LD Macro 1:2"},
      {0xF1, 0x47, 0x5C, 0x8E, 0x30, 0x3C, 0xDF, 0x0E, 0x00, 0x00, 0x00, "Tamron", "A005",
       "SP 70-300mm F4-5.6 Di VC USD"},
      // M                                         "Tamron" "872D"   "AF 75-300mm F/4-5.6 LD";
      // M                                         "Tamron" "278D"   "AF 80-210mm F/4.5-5.6";
      // M                                         "Tamron" "62D"    "AF 90-300mm F/4.5-5.6";
      // M                                         "Tamron" "186D"   "AF 100-300mm F/5-6.3";
      {0xEB, 0x40, 0x76, 0xA6, 0x38, 0x40, 0xDF, 0x0E, 0x00, 0x00, 0x00, "Tamron", "A011",
       "SP AF 150-600mm F/5-6.3 VC USD"},
      {0xE3, 0x40, 0x76, 0xA6, 0x38, 0x40, 0xDF, 0x4E, 0x00, 0x00, 0x00, "Tamron", "A022",
       "SP AF 150-600mm F/5-6.3 Di VC USD G2"},
      {0x20, 0x3C, 0x80, 0x98, 0x3D, 0x3D, 0x1E, 0x02, 0x00, 0x00, 0x00, "Tamron", "75D", "AF 200-400mm F/5.6 LD IF"},
      {0x00, 0x3E, 0x80, 0xA0, 0x38, 0x3F, 0x00, 0x02, 0x00, 0x00, 0x00, "Tamron", "A08",
       "SP AF 200-500mm F/5-6.3 Di LD (IF)"},
      {0x00, 0x3F, 0x80, 0xA0, 0x38, 0x3F, 0x00, 0x02, 0x00, 0x00, 0x00, "Tamron", "A08", "SP AF 200-500mm F/5-6.3 Di"},
      //
      //------------------------------------------------------------------------------
      // Tokina Lenses by focal length, first fixed then zoom lenses
      //------------------------------------------------------------------------------
      //
      {0x00, 0x40, 0x2B, 0x2B, 0x2C, 0x2C, 0x00, 0x02, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 17 AF PRO (AF 17mm f/3.5)"},
      {0x00, 0x47, 0x44, 0x44, 0x24, 0x24, 0x00, 0x06, 0x40, 0x00, 0x00, "Tokina", "T303503",
       "AT-X M35 PRO DX (AF 35mm f/2.8 Macro)"},
      {0x00, 0x54, 0x68, 0x68, 0x24, 0x24, 0x00, 0x02, 0x40, 0x00, 0x00, "Tokina", "T310003N",
       "AT-X M100 AF PRO D (AF 100mm f/2.8 Macro)"},
      {0x27, 0x48, 0x8E, 0x8E, 0x30, 0x30, 0x1D, 0x02, 0x00, 0x00, 0x00, "Tokina", "", "AT-X 304 AF (AF 300mm f/4)"},
      {0x00, 0x54, 0x8E, 0x8E, 0x24, 0x24, 0x00, 0x02, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 300 AF PRO (AF 300mm f/2.8)"},
      {0x12, 0x3B, 0x98, 0x98, 0x3D, 0x3D, 0x09, 0x00, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 400 AF SD (AF 400mm f/5.6)"},
      //
      {0x00, 0x40, 0x18, 0x2B, 0x2C, 0x34, 0x00, 0x06, 0x00, 0x00, 0x00, "Tokina", "T4101703",
       "AT-X 107 AF DX Fisheye (AF 10-17mm f/3.5-4.5)"},
      {0x00, 0x48, 0x1C, 0x29, 0x24, 0x24, 0x00, 0x06, 0x00, 0x00, 0x00, "Tokina", "T4111603",
       "AT-X 116 PRO DX (AF 11-16mm f/2.8)"},
      {0x7A, 0x48, 0x1C, 0x29, 0x24, 0x24, 0x7E, 0x06, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 116 PRO DX II (AF 11-16mm f/2.8)"},
      {0x7A, 0x48, 0x1C, 0x30, 0x24, 0x24, 0x7E, 0x06, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 11-20 F2.8 PRO DX (AF 11-20mm f/2.8)"},
      {0x8B, 0x48, 0x1C, 0x30, 0x24, 0x24, 0x85, 0x06, 0x00, 0x00, 0x00, "Tokina", "", "ATX-i 11-20mm F2.8 CF"},
      {0x00, 0x3C, 0x1F, 0x37, 0x30, 0x30, 0x00, 0x06, 0x00, 0x00, 0x00, "Tokina", "T4122403",
       "AT-X 124 AF PRO DX (AF 12-24mm f/4)"},
      {0x7A, 0x3C, 0x1F, 0x37, 0x30, 0x30, 0x7E, 0x06, 0x01, 0x02, 0x00, "Tokina", "T4122423",
       "AT-X 124 AF PRO DX II (AF 12-24mm f/4)"},
      {0x7A, 0x3C, 0x1F, 0x3C, 0x30, 0x30, 0x7E, 0x06, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 12-28 PRO DX (AF 12-28mm f/4)"},
      {0x94, 0x54, 0x24, 0x30, 0x18, 0x18, 0x4a, 0x06, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 14-20 F2 PRO DX (AF 14-20mm f/2)"},
      {0x00, 0x48, 0x29, 0x3C, 0x24, 0x24, 0x00, 0x06, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 16-28 AF PRO FX (AF 16-28mm f/2.8)"},
      {0x00, 0x48, 0x29, 0x50, 0x24, 0x24, 0x00, 0x06, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 165 PRO DX (AF 16-50mm f/2.8)"},
      {0x00, 0x40, 0x2A, 0x72, 0x2C, 0x3C, 0x00, 0x06, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 16.5-135 DX (AF 16.5-135mm f/3.5-5.6)"},
      {0x00, 0x3C, 0x2B, 0x44, 0x30, 0x30, 0x00, 0x06, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 17-35 F4 PRO FX (AF 17-35mm f/4)"},
      {0x2F, 0x40, 0x30, 0x44, 0x2C, 0x34, 0x29, 0x02, 0x00, 0x02, 0x00, "Tokina", "", "AF 193 (AF 19-35mm f/3.5-4.5)"},
      {0x2F, 0x48, 0x30, 0x44, 0x24, 0x24, 0x29, 0x02, 0x00, 0x02, 0x00, "Tokina", "",
       "AT-X 235 AF PRO (AF 20-35mm f/2.8)"},
      // M                                         "Tokina" ""          "AF 235 (AF 20-35mm f/3.5-4.5)"
      {0x2F, 0x40, 0x30, 0x44, 0x2C, 0x34, 0x29, 0x02, 0x00, 0x01, 0x00, "Tokina", "",
       "AF 235 II (AF 20-35mm f/3.5-4.5)"},
      // M                                         "Tokina" ""          "AT-X 240 AF (AF 24-40mm f/2.8)"
      {0x00, 0x48, 0x37, 0x5C, 0x24, 0x24, 0x00, 0x06, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 24-70MM F2.8 PRO FX (AF 24-70mm f/2.8)"},
      {0x00, 0x40, 0x37, 0x80, 0x2C, 0x3C, 0x00, 0x02, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 242 AF (AF 24-200mm f/3.5-5.6)"},
      {0x25, 0x48, 0x3C, 0x5C, 0x24, 0x24, 0x1B, 0x02, 0x00, 0x02, 0x00, "Tokina", "",
       "AT-X 270 AF PRO II (AF 28-70mm f/2.6-2.8)"},
      {0x25, 0x48, 0x3C, 0x5C, 0x24, 0x24, 0x1B, 0x02, 0x00, 0x01, 0x00, "Tokina", "",
       "AT-X 287 AF PRO SV (AF 28-70mm f/2.8)"},
      {0x07, 0x48, 0x3C, 0x5C, 0x24, 0x24, 0x03, 0x00, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 287 AF (AF 28-70mm f/2.8)"},
      {0x07, 0x47, 0x3C, 0x5C, 0x25, 0x35, 0x03, 0x00, 0x00, 0x00, 0x00, "Tokina", "",
       "AF 287 SD (AF 28-70mm f/2.8-4.5)"},
      {0x07, 0x40, 0x3C, 0x5C, 0x2C, 0x35, 0x03, 0x00, 0x00, 0x00, 0x00, "Tokina", "",
       "AF 270 II (AF 28-70mm f/3.5-4.5)"},
      {0x00, 0x48, 0x3C, 0x60, 0x24, 0x24, 0x00, 0x02, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 280 AF PRO (AF 28-80mm f/2.8)"},
      // M                                         "Tokina" ""          "AF 280 II EMZ (AF 28-80mm f/3.5-5.6)"
      // M                                         "Tokina" ""          "AF 205 (AF 28-105mm f/3.5-4.5)"
      // M                                         "Tokina" ""          "AF 282 (AF 28-200mm 3.5-5.6)"
      // M                                         "Tokina" ""          "AF 282 EMZ II (AF 28-210mm f/4.2-6.5)"
      // M                                         "Tokina" ""          "AF 370 (AF 35-70mm f/3.5-4.6)"
      // M                                         "Tokina" ""          "AF 370 II (AF 35-70mm f/3.5-4.6)"
      {0x25, 0x44, 0x44, 0x8E, 0x34, 0x42, 0x1B, 0x02, 0x00, 0x00, 0x00, "Tokina", "",
       "AF 353 (AF 35-300mm f/4.5-6.7)"},
      {0x00, 0x48, 0x50, 0x72, 0x24, 0x24, 0x00, 0x06, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 535 PRO DX (AF 50-135mm f/2.8)"},
      {0x00, 0x3C, 0x5C, 0x80, 0x30, 0x30, 0x00, 0x0E, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 70-200 F4 FX VCM-S (AF 70-200mm f/4)"},
      {0x00, 0x48, 0x5C, 0x80, 0x30, 0x30, 0x00, 0x0E, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 70-200 F4 FX VCM-S (AF 70-200mm f/4)"},
      // M                                         "Tokina" ""          "AF 745 (AF 70-210mm f/4.5)"
      // M                                         "Tokina" ""          "AF 210 (AF 70-210mm f/4.0-5.6)"
      // M                                         "Tokina" ""          "AF 210 II SD (AF 70-210mm f/4.0-5.6)"
      {0x12, 0x44, 0x5E, 0x8E, 0x34, 0x3C, 0x09, 0x00, 0x00, 0x00, 0x00, "Tokina", "",
       "AF 730 (AF 75-300mm f/4.5-5.6)"},
      // M                                         "Tokina" ""          "AF 730 II (AF 75-300mm f/4.5-5.6)"
      {0x14, 0x54, 0x60, 0x80, 0x24, 0x24, 0x0B, 0x00, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 828 AF (AF 80-200mm f/2.8)"},
      {0x24, 0x54, 0x60, 0x80, 0x24, 0x24, 0x1A, 0x02, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 828 AF PRO (AF 80-200mm f/2.8)"},
      // M                                         "Tokina" ""          "AT-X 840 AF (AF 80-400mm f/4.5-5.6)"
      {0x24, 0x44, 0x60, 0x98, 0x34, 0x3C, 0x1A, 0x02, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 840 AF-II (AF 80-400mm f/4.5-5.6)"},
      {0x00, 0x44, 0x60, 0x98, 0x34, 0x3C, 0x00, 0x02, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 840 D (AF 80-400mm f/4.5-5.6)"},
      {0x14, 0x48, 0x68, 0x8E, 0x30, 0x30, 0x0B, 0x00, 0x00, 0x00, 0x00, "Tokina", "",
       "AT-X 340 AF (AF 100-300mm f/4)"},
      // M                                         "Tokina" ""          "AT-X 340 AF-II (AF 100-300mm f/4)"
      // M                                         "Tokina" ""          "AF 130 EMZ II (AF 100-300mm f/5.6-6.7)"
      // M                                         "Tokina" ""          "AF 140 EMZ (AF 100-400mm f/4.5-6.3)"
      //
      //------------------------------------------------------------------------------
      // Lenses from various other brands
      //------------------------------------------------------------------------------
      //
      {0x06, 0x3F, 0x68, 0x68, 0x2C, 0x2C, 0x06, 0x00, 0x00, 0x00, 0x00, "Cosina", "", "AF 100mm F3.5 Macro"},
      {0x07, 0x36, 0x3D, 0x5F, 0x2C, 0x3C, 0x03, 0x00, 0x00, 0x00, 0x00, "Cosina", "",
       "AF Zoom 28-80mm F3.5-5.6 MC Macro"},
      {0x07, 0x46, 0x3D, 0x6A, 0x25, 0x2F, 0x03, 0x00, 0x00, 0x00, 0x00, "Cosina", "", "AF Zoom 28-105mm F2.8-3.8 MC"},
      // M                                         "Cosina" "" "AF Zoom 28-210mm F3.5-5.6";
      // M                                         "Cosina" "" "AF Zoom 28-210mm F4.2-6.5 Aspherical IF";
      // M                                         "Cosina" "" "AF Zoom 28-300mm F4.0-6.3";
      // M                                         "Cosina" "" "AF Zoom 70-210mm F2.8-4.0";
      {0x12, 0x36, 0x5C, 0x81, 0x35, 0x3D, 0x09, 0x00, 0x00, 0x00, 0x00, "Cosina", "",
       "AF Zoom 70-210mm F4.5-5.6 MC Macro"},
      {0x12, 0x39, 0x5C, 0x8E, 0x34, 0x3D, 0x08, 0x02, 0x00, 0x00, 0x00, "Cosina", "",
       "AF Zoom 70-300mm F4.5-5.6 MC Macro"},
      {0x12, 0x3B, 0x68, 0x8D, 0x3D, 0x43, 0x09, 0x02, 0x00, 0x00, 0x00, "Cosina", "",
       "AF Zoom 100-300mm F5.6-6.7 MC Macro"},
      // M                                         "Cosina" "" "AF Zoom 100-400mm F5.6-6.7 MC";
      //
      {0x12, 0x38, 0x69, 0x97, 0x35, 0x42, 0x09, 0x02, 0x00, 0x00, 0x00, "Promaster", "",
       "Spectrum 7 100-400mm 1:4.5-6.7"},
      //
      {0x00, 0x40, 0x31, 0x31, 0x2C, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, "Voigtlander", "BA295AN",
       "Color Skopar 20mm F3.5 SLII Aspherical"},
      {0x00, 0x48, 0x3C, 0x3C, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, "Voigtlander", "",
       "Color Skopar 28mm F2.8 SL II"},
      {0x00, 0x54, 0x48, 0x48, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, "Voigtlander", "BA229DN",
       "Ultron 40mm F2 SLII Aspherical"},
      {0x00, 0x54, 0x55, 0x55, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, "Voigtlander", "BA239BN",
       "Nokton 58mm F1.4 SLII"},
      {0x00, 0x40, 0x64, 0x64, 0x2C, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, "Voigtlander", "",
       "APO-Lanthar 90mm F3.5 SLII Close Focus"},
      //
      {0x00, 0x40, 0x2D, 0x2D, 0x2C, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, "Carl Zeiss", "", "Distagon T* 3,5/18 ZF.2"},
      {0x00, 0x48, 0x32, 0x32, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, "Carl Zeiss", "", "Distagon T* 2,8/21 ZF.2"},
      {0x00, 0x54, 0x38, 0x38, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, "Carl Zeiss", "", "Distagon T* 2/25 ZF.2"},
      {0x00, 0x54, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, "Carl Zeiss", "", "Distagon T* 2/28 ZF.2"},
      {0x00, 0x54, 0x44, 0x44, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, "Carl Zeiss", "", "Distagon T* 1.4/35 ZF.2"},
      {0x00, 0x54, 0x44, 0x44, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, "Carl Zeiss", "", "Distagon T* 2/35 ZF.2"},
      {0x00, 0x54, 0x50, 0x50, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, "Carl Zeiss", "", "Planar T* 1,4/50 ZF.2"},
      {0x00, 0x54, 0x50, 0x50, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, "Carl Zeiss", "", "Makro-Planar T* 2/50 ZF.2"},
      {0x00, 0x54, 0x62, 0x62, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, "Carl Zeiss", "", "Planar T* 1,4/85 ZF.2"},
      {0x00, 0x54, 0x68, 0x68, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, "Carl Zeiss", "",
       "Makro-Planar T* 2/100 ZF.2"},
      {0x00, 0x54, 0x72, 0x72, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, "Carl Zeiss", "", "Apo Sonnar T* 2/135 ZF.2"},
      {0x02, 0x54, 0x3C, 0x3C, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, "Zeiss", "", "Otus 1.4/28"},
      {0x00, 0x54, 0x53, 0x53, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, "Zeiss", "", "Otus 1.4/55"},
      {0x01, 0x54, 0x62, 0x62, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, "Zeiss", "", "Otus 1.4/85"},
      {0x03, 0x54, 0x68, 0x68, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, "Zeiss", "", "Otus 1.4/100"},
      //                    " "00" "00" "00" "00" "Zeiss" "" "Milvus 2.8/15"
      //                    " "00" "00" "00" "00" "Zeiss" "" "Milvus 2.8/18"
      //                    " "00" "00" "00" "00" "Zeiss" "" "Milvus 2.8/21"
      //                    " "00" "00" "00" "00" "Zeiss" "" "Milvus 1.4/25"
      {0x52, 0x54, 0x44, 0x44, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, "Zeiss", "", "Milvus 2/35"},
      {0x53, 0x54, 0x50, 0x50, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, "Zeiss", "", "Milvus 1.4/50"},
      {0x54, 0x54, 0x50, 0x50, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, "Zeiss", "", "Milvus 2/50M"},
      {0x55, 0x54, 0x62, 0x62, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, "Zeiss", "", "Milvus 1.4/85"},
      {0x56, 0x54, 0x68, 0x68, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, "Zeiss", "", "Milvus 2/100M"},
      //                    " "00" "00" "00" "00" "Zeiss" "" "Milvus 2/135"
      //
      {0x00, 0x54, 0x56, 0x56, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, "Coastal Optical Systems", "",
       "60mm 1:4 UV-VIS-IR Macro Apo"},
      //
      {0xBF, 0x4E, 0x26, 0x26, 0x1E, 0x1E, 0x01, 0x04, 0x00, 0x00, 0x00, "Irix", "", "15mm f/2.4 Firefly"},
      {0xBF, 0x3C, 0x1B, 0x1B, 0x30, 0x30, 0x01, 0x04, 0x00, 0x00, 0x00, "Irix", "", "11mm f/4.0 Blackstone/Firefly"},
      //
      {0x4A, 0x40, 0x11, 0x11, 0x2C, 0x0C, 0x4D, 0x02, 0x00, 0x00, 0x00, "Samyang", "", "8mm f/3.5 Fish-Eye CS"},
      {0x4A, 0x48, 0x24, 0x24, 0x24, 0x0C, 0x4D, 0x02, 0x00, 0x02, 0x00, "Samyang", "", "10mm f/2.8 ED AS NCS CS"},
      {0x4A, 0x48, 0x1E, 0x1E, 0x24, 0x0C, 0x4D, 0x02, 0x00, 0x00, 0x00, "Samyang", "",
       "12mm f/2.8 ED AS NCS Fish-eye"},
      {0x4A, 0x4C, 0x24, 0x24, 0x1E, 0x6C, 0x4D, 0x06, 0x00, 0x00, 0x00, "Samyang", "", "14mm f/2.4 Premium"},
      {0x4A, 0x48, 0x24, 0x24, 0x24, 0x0C, 0x4D, 0x02, 0x00, 0x01, 0x00, "Samyang", "", "AE 14mm f/2.8 ED AS IF UMC"},
      {0x4A, 0x54, 0x29, 0x29, 0x18, 0x0C, 0x4D, 0x02, 0x00, 0x00, 0x00, "Samyang", "", "16mm F2.0 ED AS UMC CS"},
      {0x4A, 0x60, 0x36, 0x36, 0x0C, 0x0C, 0x4D, 0x02, 0x00, 0x00, 0x00, "Samyang", "", "24mm f/1.4 ED AS UMC"},
      {0x4A, 0x60, 0x44, 0x44, 0x0C, 0x0C, 0x4D, 0x02, 0x00, 0x00, 0x00, "Samyang", "", "35mm f/1.4 AS UMC"},
      {0x4A, 0x60, 0x62, 0x62, 0x0C, 0x0C, 0x4D, 0x02, 0x00, 0x00, 0x00, "Samyang", "", "AE 85mm f/1.4 AS IF UMC"},
      //
      {0x9F, 0x48, 0x23, 0x23, 0x24, 0x24, 0xA1, 0x06, 0x00, 0x00, 0x00, "Yongnuo", "", "YN14mm F2.8N"},
      {0x9F, 0x4C, 0x44, 0x44, 0x18, 0x18, 0xA1, 0x06, 0x00, 0x00, 0x00, "Yongnuo", "", "YN35mm F2N"},
      {0x9F, 0x48, 0x48, 0x48, 0x24, 0x24, 0xA1, 0x06, 0x00, 0x00, 0x00, "Yongnuo", "", "YN40mm F2.8N"},
      {0x9A, 0x4C, 0x50, 0x50, 0x14, 0x14, 0x9C, 0x06, 0x00, 0x00, 0x00, "Yongnuo", "", "YN50mm F1.8N"},
      {0x9F, 0x54, 0x68, 0x68, 0x18, 0x18, 0xA2, 0x06, 0x00, 0x00, 0x00, "Yongnuo", "", "YN100mm F2N"},
      //
      {0x02, 0x40, 0x44, 0x5C, 0x2C, 0x34, 0x02, 0x00, 0x00, 0x00, 0x00, "Exakta", "", "AF 35-70mm 1:3.5-4.5 MC"},
      {0x07, 0x3E, 0x30, 0x43, 0x2D, 0x35, 0x03, 0x00, 0x00, 0x00, 0x00, "Soligor", "", "AF Zoom 19-35mm 1:3.5-4.5 MC"},
      {0x03, 0x43, 0x5C, 0x81, 0x35, 0x35, 0x02, 0x00, 0x00, 0x00, 0x00, "Soligor", "",
       "AF C/D Zoom UMCS 70-210mm 1:4.5"},
      {0x12, 0x4A, 0x5C, 0x81, 0x31, 0x3D, 0x09, 0x00, 0x00, 0x00, 0x00, "Soligor", "",
       "AF C/D Auto Zoom+Macro 70-210mm 1:4-5.6 UMCS"},
      {0x12, 0x36, 0x69, 0x97, 0x35, 0x42, 0x09, 0x00, 0x00, 0x00, 0x00, "Soligor", "",
       "AF Zoom 100-400mm 1:4.5-6.7 MC"},
      //
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, "Manual Lens", "", "No CPU"},
      {0x00, 0x00, 0x48, 0x48, 0x53, 0x53, 0x00, 0x01, 0x00, 0x00, 0x00, "Loreo", "",
       "40mm F11-22 3D Lens in a Cap 9005"},
      //
      //------------------------------------------------------------------------------
      //
      // Lenses, that were upgraded with custom CPU
      //
      {0x00, 0x47, 0x10, 0x10, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA604AC",
       "Fisheye Nikkor 8mm f/2.8 AiS"},
      {0x00, 0x47, 0x3C, 0x3C, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, "Nikon", "", "Nikkor 28mm f/2.8 AiS"},
      //"00 54 44 44 0C 0C 00" "00" "00" "00" "00" "Nikon"   "JAA115AD" "Nikkor 35mm f/1.4 AiS" double ID with
      // Zeiss
      {0x00, 0x57, 0x50, 0x50, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, "Nikon", "", "Nikkor 50mm f/1.8 AI"},
      {0x00, 0x48, 0x50, 0x50, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, "Nikon", "", "Nikkor H 50mm f/2"},
      {0x00, 0x48, 0x68, 0x68, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA304AA", "Series E 100mm f/2.8"},
      {0x00, 0x4C, 0x6A, 0x6A, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA305AA", "Nikkor 105mm f/2.5 AiS"},
      {0x00, 0x48, 0x80, 0x80, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, "Nikon", "JAA313AA", "Nikkor 200mm f/4 AiS"},
      {0x00, 0x40, 0x11, 0x11, 0x2C, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, "Samyang", "", "8mm f/3.5 Fish-Eye"},
      {0x00, 0x58, 0x64, 0x64, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, "Soligor", "", "C/D Macro MC 90mm f/2.5"},
      // https://github.com/Exiv2/exiv2/issues/743
      {0xc9, 0x48, 0x37, 0x5c, 0x24, 0x24, 0x4b, 0x4e, 0x01, 0x00, 0x00, "Sigma", "", "24-70mm F2.8 DG OS HSM Art"},
      //  https://github.com/Exiv2/exiv2/issues/598 , https://github.com/Exiv2/exiv2/pull/891
      {0xCF, 0x47, 0x5C, 0x8E, 0x31, 0x3D, 0xDF, 0x0E, 0x00, 0x00, 0x00, "Tamron", "A030",
       "SP 70-300mm F/4-5.6 Di VC USD"},
      //
      {0xf4, 0x4c, 0x7c, 0x7c, 0x2c, 0x2c, 0x4b, 0x02, 0x00, 0x00, 0x00, "Sigma", "", "APO Macro 180mm F3.5 EX DG HSM"},
      // https://github.com/Exiv2/exiv2/issues/1078
      {0x80, 0x48, 0x1C, 0x29, 0x24, 0x24, 0x7A, 0x06, 0x00, 0x00, 0x00, "Tokina", "", "atx-i 11-16mm F2.8 CF"},
      // https://github.com/Exiv2/exiv2/issues/1069
      {0xc8, 0x54, 0x44, 0x44, 0x0d, 0x0d, 0xdf, 0x46, 0x00, 0x00, 0x00, "Tamron", "F045", "SP 35mm f/1.4 Di USD"},
      // https://github.com/Exiv2/exiv2/pull/1105
      {0xCB, 0x3C, 0x2B, 0x44, 0x24, 0x31, 0xDF, 0x46, 0x00, 0x00, 0x00, "Tamron", "A037", "17-35mm F/2.8-4 Di OSD"},
      // https://github.com/Exiv2/exiv2/issues/1208
      {0xC8, 0x54, 0x62, 0x62, 0x0C, 0x0C, 0x4B, 0x46, 0x00, 0x00, 0x00, "Sigma", "321550", "85mm F1.4 DG HSM | A"},
  };
//------------------------------------------------------------------------------
#endif
  // 8< - - - 8< do not remove this line >8 - - - >8

  /* if no meta obj is provided, try to use the value param that *may*
   * be the pre-parsed lensid
   */
  if (!metadata) {
    const auto vid = static_cast<unsigned char>(value.toInt64(0));

    /* the 'FMntLens' name is added to the anonymous struct for
     * fmountlens[]
     *
     * remember to name the struct when importing/updating the lens info
     * from:
     *
     * www.rottmerhusen.com/objektives/lensid/files/c-header/fmountlens4.h
     */
    if (auto pf = Exiv2::find(fmountlens, vid))
      return os << pf->manuf << " " << pf->lensname;
    return os << "(" << value << ")";
  }

  byte raw[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

  static constexpr auto tags = std::array{
      "LensIDNumber",          "LensFStops", "MinFocalLength", "MaxFocalLength", "MaxApertureAtMinFocal",
      "MaxApertureAtMaxFocal", "MCUVersion",
  };

  const std::string pre = std::string("Exif.") + group + std::string(".");
  for (unsigned int i = 0; i < 7; ++i) {
    ExifKey key(pre + std::string(tags[i]));
    auto md = metadata->findKey(key);
    if (md == metadata->end() || md->typeId() != unsignedByte || md->count() == 0) {
      return os << "(" << value << ")";
    }
    raw[i] = static_cast<byte>(md->toInt64());
  }

  auto md = metadata->findKey(ExifKey("Exif.Nikon3.LensType"));
  if (md == metadata->end() || md->typeId() != unsignedByte || md->count() == 0) {
    return os << "(" << value << ")";
  }
  raw[7] = static_cast<byte>(md->toInt64());

  for (const auto& f : fmountlens) {
    if (raw[0] == f.lid) {
      // #1034
      const std::string undefined("undefined");
      const std::string section("nikon");
      auto lensIDStream = std::to_string(raw[7]);
      if (Internal::readExiv2Config(section, lensIDStream, undefined) != undefined) {
        return os << Internal::readExiv2Config(section, lensIDStream, undefined);
      }
    }

    if (raw[0] == f.lid
        // stps varies with focal length for some Sigma zoom lenses.
        && (raw[1] == f.stps || f.manuf == "Sigma") && raw[2] == f.focs && raw[3] == f.focl && raw[4] == f.aps &&
        raw[5] == f.apl && raw[6] == f.lfw && raw[7] == f.ltype) {
      // Lens found in database
      return os << f.manuf << " " << f.lensname;
    }
  }
  // Lens not found in database
  return os << "(" << value << ")";
#else
  return os << "(" << value << ")";
#endif  // EXV_HAVE_LENSDATA
}

std::ostream& Nikon3MakerNote::printFocusDistance(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedByte)
    return os << "(" << value << ")";
  auto val = value.toInt64();
  if (val == 0)
    return os << _("n/a");

  return os << stringFormat("{:.2f} m", std::pow(10.0, (val / 40.0) - 2.0));
}

std::ostream& Nikon3MakerNote::printAperture(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedByte)
    return os << "(" << value << ")";

  auto val = value.toInt64();
  if (val == 0)
    return os << _("n/a");

  return os << stringFormat("F{:.1f}", std::exp2(val / 24.0));
}

std::ostream& Nikon3MakerNote::printFocal(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    return os << "(" << value << ")";
  }
  auto val = value.toInt64();
  if (val == 0)
    return os << _("n/a");

  return os << stringFormat("{:.1f} mm", 5.0 * std::exp2(val / 24.0));
}

std::ostream& Nikon3MakerNote::printFStops(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedByte)
    return os << "(" << value << ")";
  return os << stringFormat("F{:.1f}", value.toInt64() / 12.0);
}

std::ostream& Nikon3MakerNote::printExitPupilPosition(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedByte || value.toInt64() == 0)
    return os << "(" << value << ")";
  return os << stringFormat("{:.1f} mm", 2048.0 / value.toInt64());
}

std::ostream& Nikon3MakerNote::printFlashFocalLength(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    return os;
  }
  auto temp = value.toInt64();
  if (temp == 0 || temp == 255)
    return os << _("n/a");

  return os << stringFormat("{:1} mm", temp);
}

std::ostream& Nikon3MakerNote::printRepeatingFlashRate(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    return os << "(" << value << ")";
  }
  auto temp = value.toInt64();
  if (temp == 0 || temp == 255)
    return os << _("n/a");

  return os << stringFormat("{:2} Hz", temp);
}

std::ostream& Nikon3MakerNote::printRepeatingFlashCount(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    return os << "(" << value << ")";
  }
  auto temp = value.toInt64();
  if (temp == 0 || temp == 255)
    return os << _("n/a");

  return os << stringFormat("{:1}", temp);
}

std::ostream& Nikon3MakerNote::printExternalFlashData1Fl6(std::ostream& os, const Value& value,
                                                          const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }
  std::ostringstream oss;
  oss.copyfmt(os);
  const auto v0 = value.toUint32(0);
  os << (v0 & 0x01 ? _("Fired") : _("Did not fire"));

  if (v0 & 0x01) {
    std::ostringstream ossTemp;
    printTagBitmask<std::size(nikonFlashAdaptors), nikonFlashAdaptors>(ossTemp, value, metadata);
    std::string tempStr = ossTemp.str();
    if (!tempStr.empty()) {
      os << ", " << tempStr;
    }
  }
  os.copyfmt(oss);
  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printExternalFlashData2Fl6(std::ostream& os, const Value& value,
                                                          const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }
  std::ostringstream oss;
  oss.copyfmt(os);
  const auto v0 = value.toUint32(0);
  os << (v0 & 0x80 ? _("External flash on") : _("External flash off"));

  if (v0 & 0x80) {
    os << ", ";
    printTag<std::size(nikonFlashControlMode), nikonFlashControlMode>(os, (value.toUint32() & 0x0F), metadata);
  }
  os.copyfmt(oss);
  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printExternalFlashData1Fl7(std::ostream& os, const Value& value,
                                                          const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }
  std::ostringstream oss;
  oss.copyfmt(os);
  const auto v0 = value.toUint32();
  os << (v0 & 0x01 ? _("External flash on") : _("External flash off"));

  if (v0 & 0x01) {
    os << ", ";
    os << (v0 & 0x80 ? _("External flash zoom override") : _("No external flash zoom override"));

    std::ostringstream ossTemp;
    printTagBitmask<std::size(nikonFlashAdaptors), nikonFlashAdaptors>(ossTemp, value, metadata);
    std::string tempStr = ossTemp.str();
    if (!tempStr.empty()) {
      os << ", " << tempStr;
    }
  }
  os.copyfmt(oss);
  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printExternalFlashData2(std::ostream& os, const Value& value, const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  printTag<std::size(nikonFlashControlMode), nikonFlashControlMode>(os, (value.toUint32() & 0x0F), metadata);

  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printFlashMasterDataFl6(std::ostream& os, const Value& value, const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte || !metadata) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  // Check if using an automated or manual mode
  auto pos = metadata->findKey(ExifKey("Exif.NikonFl6.ExternalFlashData1"));
  if (pos == metadata->end() || pos->count() != 1 || pos->typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  const auto mode = pos->toUint32(0) & 0x0F;
  if (mode == 0) {
    os << _("n/a");
    os.flags(f);
    return os;
  }

  printFlashCompensationValue(os, static_cast<unsigned char>(value.toUint32(0)), flashModeUsesManualScale(mode));

  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printFlashMasterDataFl7(std::ostream& os, const Value& value, const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte || !metadata) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  // Check if using an automated or manual mode
  auto pos = metadata->findKey(ExifKey("Exif.NikonFl7.ExternalFlashData2"));
  if (pos == metadata->end() || pos->count() != 1 || pos->typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  const auto mode = pos->toUint32(0) & 0x0F;
  if (mode == 0) {
    os << _("n/a");
    os.flags(f);
    return os;
  }

  printFlashCompensationValue(os, static_cast<unsigned char>(value.toUint32(0)), flashModeUsesManualScale(mode));

  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printFlashGroupAControlData(std::ostream& os, const Value& value, const ExifData* data) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }
  std::ostringstream oss;
  oss.copyfmt(os);

  printTag<std::size(nikonFlashControlMode), nikonFlashControlMode>(os, (value.toUint32() & 0x0F), data);

  os.copyfmt(oss);
  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printFlashGroupBCControlData(std::ostream& os, const Value& value,
                                                            const ExifData* data) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }
  std::ostringstream oss;
  oss.copyfmt(os);
  const auto temp = value.toUint32();

  printTag<std::size(nikonFlashControlMode), nikonFlashControlMode>(os, (temp >> 4), data);
  os << ", ";
  printTag<std::size(nikonFlashControlMode), nikonFlashControlMode>(os, (temp & 0x0f), data);

  os.copyfmt(oss);
  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printFlashGroupADataFl6(std::ostream& os, const Value& value, const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte || !metadata) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  // Check if using an automated or manual mode
  auto pos = metadata->findKey(ExifKey("Exif.NikonFl6.FlashGroupAControlData"));
  if (pos == metadata->end() || pos->count() != 1 || pos->typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  const auto mode = pos->toUint32(0) & 0x0F;
  if (mode == 0) {
    os << _("n/a");
    os.flags(f);
    return os;
  }

  printFlashCompensationValue(os, static_cast<unsigned char>(value.toUint32(0)), flashModeUsesManualScale(mode));

  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printFlashGroupADataFl7(std::ostream& os, const Value& value, const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte || !metadata) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  // Check if using an automated or manual mode
  auto pos = metadata->findKey(ExifKey("Exif.NikonFl7.FlashGroupAControlData"));
  if (pos == metadata->end() || pos->count() != 1 || pos->typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  const auto mode = pos->toUint32(0) & 0x0F;
  if (mode == 0) {
    os << _("n/a");
    os.flags(f);
    return os;
  }

  printFlashCompensationValue(os, static_cast<unsigned char>(value.toUint32(0)), flashModeUsesManualScale(mode));

  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printFlashGroupBDataFl6(std::ostream& os, const Value& value, const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte || !metadata) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  // Check if using an automated or manual mode
  auto pos = metadata->findKey(ExifKey("Exif.NikonFl6.FlashGroupBCControlData"));
  if (pos == metadata->end() || pos->count() != 1 || pos->typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  const auto mode = pos->toUint32(0) >> 4;
  if (mode == 0) {
    os << _("n/a");
    os.flags(f);
    return os;
  }

  printFlashCompensationValue(os, static_cast<unsigned char>(value.toUint32(0)), flashModeUsesManualScale(mode));

  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printFlashGroupBDataFl7(std::ostream& os, const Value& value, const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte || !metadata) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  // Check if using an automated or manual mode
  auto pos = metadata->findKey(ExifKey("Exif.NikonFl7.FlashGroupBCControlData"));
  if (pos == metadata->end() || pos->count() != 1 || pos->typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  const auto mode = pos->toUint32(0) >> 4;
  if (mode == 0) {
    os << _("n/a");
    os.flags(f);
    return os;
  }

  printFlashCompensationValue(os, static_cast<unsigned char>(value.toUint32(0)), flashModeUsesManualScale(mode));

  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printFlashGroupCDataFl6(std::ostream& os, const Value& value, const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte || !metadata) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  // Check if using an automated or manual mode
  auto pos = metadata->findKey(ExifKey("Exif.NikonFl6.FlashGroupBCControlData"));
  if (pos == metadata->end() || pos->count() != 1 || pos->typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  const auto mode = pos->toUint32(0) & 0x000F;
  if (mode == 0) {
    os << _("n/a");
    os.flags(f);
    return os;
  }

  printFlashCompensationValue(os, static_cast<unsigned char>(value.toUint32(0)), flashModeUsesManualScale(mode));

  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printFlashGroupCDataFl7(std::ostream& os, const Value& value, const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte || !metadata) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  // Check if using an automated or manual mode
  auto pos = metadata->findKey(ExifKey("Exif.NikonFl7.FlashGroupBCControlData"));
  if (pos == metadata->end() || pos->count() != 1 || pos->typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  const auto mode = pos->toUint32(0) & 0x000F;
  if (mode == 0) {
    os << _("n/a");
    os.flags(f);
    return os;
  }

  printFlashCompensationValue(os, static_cast<unsigned char>(value.toUint32(0)), flashModeUsesManualScale(mode));

  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printExternalFlashData3(std::ostream& os, const Value& value, const ExifData* data) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  printTag<std::size(nikonFlashExposureComp), nikonFlashExposureComp>(os, (value.toUint32(0) & 0x04), data);

  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printCameraExposureCompensation(std::ostream& os, const Value& value, const ExifData*) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }
  printFlashCompensationValue(os, static_cast<unsigned char>(value.toUint32(0)), false);
  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printExternalFlashData4(std::ostream& os, const Value& value, const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  printTag<std::size(nikonFlashIlluminationPat), nikonFlashIlluminationPat>(os, value.toUint32(0), metadata);

  os.flags(f);
  return os;
}

std::ostream& Nikon3MakerNote::printFlashZoomHeadPosition(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    return os << "(" << value << ")";
  }

  auto v0 = value.toUint32(0);
  if (v0 == 0) {
    return os << _("n/a");
  }

  return os << stringFormat("{} mm", v0);
}

std::ostream& Nikon3MakerNote::printTimeZone(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != signedShort) {
    return os << "(" << value << ")";
  }
  char sign = value.toInt64() < 0 ? '-' : '+';
  long h = static_cast<long>(std::fabs(value.toFloat() / 60.0F)) % 24;
  long min = static_cast<long>(std::fabs(value.toFloat() - (h * 60))) % 60;
  return os << stringFormat("UTC {}{:02}:{:02}", sign, h, min);
}

std::ostream& Nikon3MakerNote::printPictureControl(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    return os << "(" << value << ")";
  }
  const auto pcval = value.toInt64() - 0x80;
  std::ostringstream oss;
  oss.copyfmt(os);
  switch (pcval) {
    case 0:
      os << _("Normal");
      break;
    case 127:
      os << _("n/a");
      break;
    case -127:
      os << _("User");
      break;
    case -128:
      os << _("Auto");
      break;
    default:
      os << pcval;
      break;
  }
  os.copyfmt(oss);
  return os;
}

std::ostream& Nikon3MakerNote::print0x009a(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 2 || value.typeId() != unsignedRational) {
    return os << value;
  }
  float f1 = value.toFloat(0);
  float f2 = value.toFloat(1);
  return os << f1 << " x " << f2 << " um";
}

std::ostream& Nikon3MakerNote::print0x009e(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 10 || value.typeId() != unsignedShort) {
    return os << value;
  }
  std::string s;
  bool trim = true;
  for (int i = 9; i >= 0; --i) {
    const auto l = value.toInt64(i);
    if (i > 0 && l == 0 && trim)
      continue;
    if (l != 0)
      trim = false;
    std::string d = s.empty() ? "" : "; ";
    if (auto td = Exiv2::find(nikonRetouchHistory, l)) {
      s = std::string(exvGettext(td->label_)).append(d).append(s);
    } else {
      s = std::string(_("Unknown")).append(" (").append(std::to_string(l)).append(")").append(d).append(s);
    }
  }
  return os << s;
}

std::ostream& Nikon3MakerNote::printApertureLd4(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedShort) {
    return os << "(" << value << ")";
  }
  auto temp = value.toInt64();
  if (temp == 0)
    return os << _("n/a");

  return os << stringFormat("F{:.1f}", std::exp2((temp / 384.0) - 1.0));
}
std::ostream& Nikon3MakerNote::printFocalLd4(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedShort) {
    return os << "(" << value << ")";
  }
  auto temp = value.toInt64();
  if (temp == 0)
    return os << _("n/a");

  return os << stringFormat("{} mm", temp);
}

std::ostream& Nikon3MakerNote::printFocusDistanceLd4(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedShort) {
    return os << "(" << value << ")";
  }
  auto temp = value.toInt64();
  if (temp == 0)
    return os << _("n/a");

  return os << stringFormat("{:.2f} m", std::pow(10.0, (temp / (256.0 * 40.0)) - 2.0));
}

}  // namespace Exiv2::Internal
