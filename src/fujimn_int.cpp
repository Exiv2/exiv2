// SPDX-License-Identifier: GPL-2.0-or-later
/*
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (gc) <caulier dot gilles at gmail dot com>
  History:   18-Feb-04, ahu: created
             07-Mar-04, ahu: isolated as a separate component
  Credits:   See header file.
 */
// *****************************************************************************
// included header files
#include "fujimn_int.hpp"
#include "i18n.h"  // NLS support.
#include "tags_int.hpp"
#include "types.hpp"

// + standard includes

// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {
//! OffOn, multiple tags
constexpr TagDetails fujiOffOn[] = {
    {0, N_("Off")},
    {1, N_("On")},
};

//! Sharpness, tag 0x1001
constexpr TagDetails fujiSharpness[] = {
    {0, N_("-4 (softest)")}, {1, N_("-3 (very soft)")},     {2, N_("-2 (soft)")},
    {3, N_("0 (normal)")},   {4, N_("+2 (hard)")},          {5, N_("+3 (very hard)")},
    {6, N_("+4 (hardest)")}, {130, N_("-1 (medium soft)")}, {132, N_("+1 (medium hard)")},
};

//! WhiteBalance, tag 0x1002
constexpr TagDetails fujiWhiteBalance[] = {
    {0, N_("Auto")},
    {1, N_("Auto White Priority")},
    {2, N_("Auto Ambience Priority")},
    {256, N_("Daylight")},
    {512, N_("Cloudy")},
    {768, N_("Fluorescent (daylight)")},
    {769, N_("Fluorescent (warm white)")},
    {770, N_("Fluorescent (cool white)")},
    {1024, N_("Incandescent")},
    {1536, N_("Underwater")},
    {3480, N_("Custom")},
    {3840, N_("Custom 1")},
    {3841, N_("Custom 2")},
    {3842, N_("Custom 3")},
    {3843, N_("Custom 4")},
    {3844, N_("Custom 5")},
    {4080, N_("Kelvin")},
};

//! Color, tag 0x1003
constexpr TagDetails fujiColor[] = {
    {0, N_("0 (normal)")},
    {128, N_("+1 (medium high)")},
    {192, N_("+3 (very high)")},
    {224, N_("+4 (highest)")},
    {256, N_("+2 (high)")},
    {384, N_("-1 (medium low)")},
    {512, N_("-2 (low)")},  // this is "Low" with exiftool
    {768, N_("Monochrome")},
    {769, N_("Monochrome + R Filter")},
    {770, N_("Monochrome + Ye Filter")},
    {771, N_("Monochrome + G Filter")},
    {784, N_("Sepia")},
    {1024, N_("-2 (low)")},
    {1216, N_("-3 (very low)")},
    {1248, N_("-4 (lowest)")},
    {1280, N_("ACROS")},
    {1281, N_("ACROS + R Filter")},
    {1282, N_("ACROS + Ye Filter")},
    {1283, N_("ACROS + G Filter")},
    {32768, N_("Film Simulation")},
};

//! Tone, tag 0x1004
constexpr TagDetails fujiTone[] = {
    {0, N_("Normal")},       {80, N_("Medium High")}, {256, N_("High")},
    {384, N_("Medium Low")}, {512, N_("Low")},        {32768, N_("Film Simulation")},
};

//! Tone, tag 0x1006
constexpr TagDetails fujiContrast[] = {
    {0, N_("Normal")},
    {256, N_("High")},
    {768, N_("Low")},
};

//! WhiteBalanceFineTune, tag 0x100a
static std::ostream& printFujiWhiteBalanceFineTune(std::ostream& os, const Value& value, const ExifData*) {
  if (value.typeId() == signedLong && value.size() == 8) {
    auto longValue = dynamic_cast<const LongValue&>(value);
    if (longValue.toInt64(0) % 20 == 0 && longValue.toInt64(1) % 20 == 0) {
      auto redShift = longValue.toInt64(0) / 20;
      auto blueShift = longValue.toInt64(1) / 20;
      os << "R: " << redShift << " B: " << blueShift;
      return os;
    }
  }
  os << "(" << value << ")";
  return os;
}

//! NoiseReduction, tag 0x100b
constexpr TagDetails fujiNoiseReduction[]{
    {64, N_("Low")},
    {128, N_("Normal")},
    {256, N_("n/a")},
};

//! High ISO NR, tag 0x100e
constexpr TagDetails fujiHighIsoNR[] = {
    {0, N_("0 (normal)")},         {256, N_("+2 (strong)")},    {384, N_("+1 (medium strong)")},
    {448, N_("+3 (very strong)")}, {480, N_("+4 (strongest)")}, {512, N_("-2 (weak)")},
    {640, N_("-1 (medium weak)")}, {704, N_("-3 (very weak)")}, {736, N_("-4 (weakest)")},
};

//! Clarity, tag 0x100f
constexpr TagDetails fujiClarity[] = {
    {-5000, N_("-5")}, {-4000, N_("-4")}, {-3000, N_("-3")}, {-2000, N_("-2")}, {-1000, N_("-1")}, {0, N_("0")},
    {1000, N_("+1")},  {2000, N_("+2")},  {3000, N_("+3")},  {4000, N_("+4")},  {5000, N_("+5")},
};

//! FlashMode, tag 0x1010
constexpr TagDetails fujiFlashMode[] = {
    {0x0000, N_("Auto")},
    {0x0001, N_("On")},
    {0x0002, N_("Off")},
    {0x0003, N_("Red-eye reduction")},
    {0x0004, N_("External")},
    {0x0010, N_("Commander")},
    {0x8000, N_("No flash")},
    {0x8120, N_("TTL")},
    {0x8320, N_("TTL Auto - Did not fire")},
    {0x9840, N_("Manual")},
    {0x9860, N_("Flash Commander")},
    {0x9880, N_("Multi-flash")},
    {0xa920, N_("1st Curtain (front)")},
    {0xaa20, N_("TTL Slow - 1st Curtain (front)")},
    {0xab20, N_("TTL Auto - 1st Curtain (front)")},
    {0xad20, N_("TTL - Red-eye Flash - 1st Curtain (front)")},
    {0xae20, N_("TTL - Slow - Red-eye Flash - 1st Curtain (front)")},
    {0xaf20, N_("TTL - Auto - Red-eye Flash - 1st Curtain (front)")},
    {0xc920, N_("2nd Curtain (rear)")},
    {0xca20, N_("TTL Slow - 2nd Curtain (rear)")},
    {0xcb20, N_("TTL Auto - 2nd Curtain (rear)")},
    {0xcd20, N_("TTL - Red-eye Flash - 2nd Curtain (rear)")},
    {0xce20, N_("TTL Slow - Red-eye Flash - 2nd Curtain (rear)")},
    {0xcf20, N_("TTL Auto - Red-eye Flash - 2nd Curtain (rear)")},
    {0xe920, N_("High Speed Sync (HSS)")},
};

//! FocusMode, tag 0x1021
constexpr TagDetails fujiFocusMode[] = {
    {0, N_("Auto")},
    {1, N_("Manual")},
    {65535, N_("Movie")},
};

//! FocusArea, tag 0x1022
constexpr TagDetails fujiFocusArea[] = {
    {0, N_("Wide")},
    {1, N_("Single Point")},
    {256, N_("Zone")},
    {512, N_("Tracking")},
};

//! PictureMode, tag 0x1031
constexpr TagDetails fujiPictureMode[] = {
    {0, N_("Auto")},
    {1, N_("Portrait")},
    {2, N_("Landscape")},
    {3, N_("Macro")},
    {4, N_("Sports")},
    {5, N_("Night scene")},
    {6, N_("Program AE")},
    {7, N_("Natural light")},
    {8, N_("Anti-blur")},
    {9, N_("Beach & Snow")},
    {10, N_("Sunset")},
    {11, N_("Museum")},
    {12, N_("Party")},
    {13, N_("Flower")},
    {14, N_("Text")},
    {15, N_("Natural Light & Flash")},
    {16, N_("Beach")},
    {17, N_("Snow")},
    {18, N_("Fireworks")},
    {19, N_("Underwater")},
    {20, N_("Portrait with Skin Correction")},
    {22, N_("Panorama")},
    {23, N_("Night (tripod)")},
    {24, N_("Pro Low-light")},
    {25, N_("Pro Focus")},
    {26, N_("Portrait 2")},
    {27, N_("Dog Face Detection")},
    {28, N_("Cat Face Detection")},
    {48, N_("HDR")},
    {64, N_("Advanced Filter")},
    {256, N_("Aperture-priority AE")},
    {512, N_("Shutter speed priority AE")},
    {768, N_("Manual")},
};

//! EXRMode, tag 0x1034
constexpr TagDetails fujiExrMode[] = {
    {0x100, N_("HR (High Resolution)")},
    {0x200, N_("SN (Signal to Noise priority)")},
    {0x300, N_("DR (Dynamic Range priority)")},
};

//! ShadowTone, tag 0x1040
//! HighlightTone, tag 0x041
constexpr TagDetails fujiSHTone[] = {
    {-64, N_("+4")},   {-56, N_("+3.5")}, {-48, N_("+3")},  {-40, N_("+2.5")}, {-32, N_("+2")},
    {-24, N_("+1.5")}, {-16, N_("+1")},   {-8, N_("+0.5")}, {0, N_("0")},      {8, N_("-0.5")},
    {16, N_("-1")},    {24, N_("-1.5")},  {32, N_("-2")},
};

//! DigitalZoom, tag 0x1044
static std::ostream& printFujiDigitalZoom(std::ostream& os, const Value& value, const ExifData*) {
  if (value.typeId() == unsignedLong && value.size() == 4) {
    os << (value.toFloat() / 8);
  }
  os << "(" << value << ")";
  return os;
}

//! GrainEffectRoughness, tag 0x104c
//! ColorChromeEffect, tag 0x1048
//! ColorChromeFXBlue, tag 0x104e
constexpr TagDetails fujiOff0Weak32Strong64[] = {
    {0, N_("Off")},
    {32, N_("Weak")},
    {64, N_("Strong")},
};

//! CropMode, tag 0x104d
constexpr TagDetails fujiCropMode[] = {
    {0, N_("None")},
    {1, N_("Full frame")},
    {2, N_("Sports Finder Mode")},
    {4, N_("Electronic Shutter 1.25x Crop")},
};

//! MonochromaticColor, tag 0x1049 and 0x104b
static std::ostream& printFujiMonochromaticColor(std::ostream& os, const Value& value, const ExifData*) {
  if (value.size() == 1) {
    auto v = static_cast<std::int8_t>(value.toInt64());
    os << (v > 0 ? "+" : "") << static_cast<int>(v);
  } else {
    os << "(" << value << ")";
  }
  return os;
}

//! ShutterType, tag 0x1050
constexpr TagDetails fujiShutterType[] = {
    {0, N_("Mechanical")},
    {1, N_("Electronic")},
    {2, N_("Electronic (long shutter speed)")},
    {3, N_("Electronic Front Curtain")},
};

//! Continuous, tag 0x1100
constexpr TagDetails fujiContinuous[] = {
    {0, N_("Off")},
    {1, N_("On")},
    {2, N_("Pre-shot/No flash & flash")},  // TODO newer cameras: Pre-shot
    {6, N_("Pixel Shift")},
};

//! DriveSetting Byte 1, tag 0x1100
constexpr TagDetails fujiDriveSettingByte1[] = {
    {0, N_("Single")},
    {1, N_("Continuous Low")},
    {2, N_("Continuous High")},
};

//! DriveSetting, tag 0x1103
static std::ostream& printFujiDriveSetting(std::ostream& os, const Value& value, const ExifData*) {
  auto byte1 = value.toInt64() & 0xff;
  auto byte2 = (value.toInt64() >> 8) & 0xff;
  auto byte3 = (value.toInt64() >> 16) & 0xff;
  auto fps = value.toInt64() >> 24;

  if (auto setting = Exiv2::find(fujiDriveSettingByte1, byte1)) {
    os << exvGettext(setting->label_);
  } else {
    os << "(" << byte1 << ")";
  }

  if (fps != 0) {
    os << ", " << fps << " fps";
  }

  if (byte1 != 0) {
    os << ", (" << byte2 << ", " << byte3 << ")";  // unknown values
  }
  return os;
}

//! PanoramaDirection, tag 0x1154
constexpr TagDetails fujiPanoramaDirection[] = {
    {1, N_("Right")},
    {2, N_("Up")},
    {3, N_("Left")},
    {4, N_("Down")},
};

//! AdvancedFilter, tag 0x1201
constexpr TagDetails fujiAdvancedFilter[] = {
    {0x10000, N_("Pop Color")},
    {0x20000, N_("Hi Key")},
    {0x30000, N_("Toy Camera")},
    {0x40000, N_("Miniature")},
    {0x50000, N_("Dynamic Tone")},
    {0x60001, N_("Partial Color Red")},
    {0x60002, N_("Partial Color Yellow")},
    {0x60003, N_("Partial Color Green")},
    {0x60004, N_("Partial Color Blue")},
    {0x60005, N_("Partial Color Orange")},
    {0x60006, N_("Partial Color Purple")},
    {0x70000, N_("Soft Focus")},
    {0x90000, N_("Low Key")},
};

//! FinePixColor, tag 0x1210
constexpr TagDetails fujiFinePixColor[] = {
    {0, N_("Standard")},
    {16, N_("Chrome")},
    {48, N_("Black & white")},
};

//! DynamicRange, tag 0x1400
constexpr TagDetails fujiDynamicRange[] = {
    {1, N_("Standard")},
    {3, N_("Wide")},
};

//! FilmMode, tag 0x1401
constexpr TagDetails fujiFilmMode[] = {
    {0, N_("PROVIA (F0/Standard)")},
    {256, N_("F1/Studio Portrait")},
    {272, N_("F1a/Studio Portrait Enhanced Saturation")},
    {288, N_("ASTIA (F1b/Studio Portrait Smooth Skin Tone)")},
    {304, N_("F1c/Studio Portrait Increased Sharpness")},
    {512, N_("Velvia (F2/Fujichrome)")},
    {768, N_("F3/Studio Portrait Ex")},
    {1024, N_("F4/Velvia")},
    {1280, N_("PRO Neg. Std")},
    {1281, N_("PRO Neg. Hi")},
    {1536, N_("CLASSIC CHROME")},
    {1792, N_("ETERNA")},
    {2048, N_("CLASSIC Neg.")},
    {2304, N_("ETERNA Bleach Bypass")},
    {2560, N_("Nostalgic Neg.")},
    {2816, N_("REALA ACE")},
};

//! DynamicRange, tag 0x1402
constexpr TagDetails fujiDynamicRangeSetting[] = {
    {0, N_("Auto")},
    {1, N_("Manual")},
    {256, N_("Standard (100%)")},
    {512, N_("Wide mode 1 (230%)")},
    {513, N_("Wide mode 2 (400%)")},
    {32768, N_("Film simulation mode")},
};

//! SceneRecognition, tag 0x1425
constexpr TagDetails fujiSceneRecognition[] = {
    {0x000, N_("Unrecognized")},    {0x100, N_("Portrait Image")},
    {0x103, N_("Night Portrait")},  {0x105, N_("Backlit Portrait")},
    {0x200, N_("Landscape Image")}, {0x300, N_("Night Scene")},
    {0x400, N_("Macro")},
};

//! ImageGeneration, tag 0x1426
constexpr TagDetails fujiImageGeneration[] = {
    {0, N_("Original Image")},
    {1, N_("Re-developed from RAW")},
};

//! DRangePriority, tag 0x1443
constexpr TagDetails fujiDRangePriority[] = {
    {0, N_("Auto")},
    {1, N_("Fixed")},
};

//! DRangePriorityAuto, tag 0x1444
constexpr TagDetails fujiDRangePriorityAuto[] = {
    {1, N_("Weak")},
    {2, N_("Strong")},
};

//! DRangePriorityFixed, tag 0x1445
constexpr TagDetails fujiDRangePriorityFixed[] = {
    {1, N_("Weak")},
    {2, N_("Strong")},
};

//! FaceElementType, every byte at tag 0x4201
constexpr TagDetails fujiFaceElementType[] = {
    {1, N_("Face")},
    {2, N_("Left Eye")},
    {3, N_("Right Eye")},
};

//! FaceElementType, tag 0x4203
static std::ostream& printFujiFaceElementTypes(std::ostream& os, const Value& value, const ExifData*) {
  if (value.typeId() == asciiString || value.typeId() == unsignedShort) {
    auto elements = value.count();

    if (value.typeId() == asciiString && elements > 0) {
      elements -= 1;  // null escaped
    }

    for (size_t n = 0; n < elements; n++) {
      auto longValue = value.toInt64(n);

      if (value.typeId() == asciiString) {
        longValue -= '0';
      }

      if (n != 0) {
        os << " ";
      }
      if (auto td = Exiv2::find(fujiFaceElementType, longValue)) {
        os << exvGettext(td->label_);
      } else {
        os << "(" << value.toInt64(n) << ")";
      }
    }
  } else {
    os << "(" << value << ")";
  }
  return os;
}

// Fujifilm MakerNote Tag Info
constexpr TagInfo FujiMakerNote::tagInfo_[] = {
    {0x0000, "Version", N_("Version"), N_("Fujifilm Makernote version"), IfdId::fujiId, SectionId::makerTags, undefined,
     -1, printValue},
    {0x0010, "SerialNumber", N_("Serial Number"),
     N_("This number is unique, and contains the date of manufacture, "
        "but is not the same as the number printed on the camera body."),
     IfdId::fujiId, SectionId::makerTags, asciiString, -1, printValue},
    {0x1000, "Quality", N_("Quality"), N_("Image quality setting"), IfdId::fujiId, SectionId::makerTags, asciiString,
     -1, printValue},
    {0x1001, N_("Sharpness"), N_("Sharpness"), N_("Sharpness setting"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(fujiSharpness)},
    {0x1002, "WhiteBalance", N_("White Balance"), N_("White balance setting"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(fujiWhiteBalance)},
    {0x1003, "Color", N_("Color"), N_("Chroma saturation setting"), IfdId::fujiId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(fujiColor)},
    {0x1004, "Tone", N_("Tone"), N_("Tone (contrast) setting"), IfdId::fujiId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(fujiTone)},
    {0x1005, "ColorTemperature", N_("Color Temperature"), N_("Color temperature setting"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1006, "Contrast", N_("Contrast"), N_("Contrast setting"), IfdId::fujiId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(fujiContrast)},
    {0x100a, "WhiteBalanceFineTune", N_("White Balance Fine Tune"), N_("White balance fine tune setting"),
     IfdId::fujiId, SectionId::makerTags, signedLong, -1, printFujiWhiteBalanceFineTune},
    {0x100b, "NoiseReduction", N_("Noise Reduction"), N_("Noise reduction setting"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiNoiseReduction)},
    {0x100e, "HighIsoNoiseReduction", N_("High ISO Noise Reduction"), N_("High ISO NR setting"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiHighIsoNR)},
    {0x100f, "Clarity", N_("Clarity"), N_("Clarity setting"), IfdId::fujiId, SectionId::makerTags, signedLong, -1,
     EXV_PRINT_TAG(fujiClarity)},
    {0x1010, "FlashMode", N_("Flash Mode"), N_("Flash firing mode setting"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(fujiFlashMode)},
    {0x1011, "FlashStrength", N_("Flash Strength"), N_("Flash firing strength compensation setting"), IfdId::fujiId,
     SectionId::makerTags, signedRational, -1, printValue},
    {0x1020, "Macro", N_("Macro"), N_("Macro mode setting"), IfdId::fujiId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(fujiOffOn)},
    {0x1021, "FocusMode", N_("Focus Mode"), N_("Focusing mode setting"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(fujiFocusMode)},
    {0x1022, "FocusArea", "Focus Area", N_("Focus area setting"), IfdId::fujiId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(fujiFocusArea)},
    {0x1023, "FocusPoint", N_("Focus Point"), N_("X and Y coordinate of focus point"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x102b, "FocusPrioritySetting", N_("Focus Priority Setting"), N_("Focus priority setting"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x102d, "FocusSetting", N_("Focus Setting"), N_("Focus setting"), IfdId::fujiId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    {0x102e, "ContinuousFocusSetting", N_("AF-C Focus Setting"), N_("AF-C focus setting"), IfdId::fujiId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x1030, "SlowSync", N_("Slow Sync"), N_("Slow synchro mode setting"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(fujiOffOn)},
    {0x1031, "PictureMode", N_("Picture Mode"), N_("Picture mode setting"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(fujiPictureMode)},
    {0x1032, "ExposureCount", "Exposure Count", N_("Number of exposures used for this image"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1033, "EXRAuto", "EXR Auto",
     N_("EXR Auto"),  // TODO find description
     IfdId::fujiId, SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiOffOn)},
    {0x1034, "EXRMode", "EXR Mode",
     N_("EXR Auto"),  // TODO find description
     IfdId::fujiId, SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiExrMode)},
    {0x1040, "ShadowTone", N_("Shadow Tone"), N_("Shadow tone"), IfdId::fujiId, SectionId::makerTags, signedLong, -1,
     EXV_PRINT_TAG(fujiSHTone)},
    {0x1041, "HighlightTone", N_("Highlight Tone"), N_("Highlight tone"), IfdId::fujiId, SectionId::makerTags,
     signedLong, -1, EXV_PRINT_TAG(fujiSHTone)},
    {0x1044, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom"), IfdId::fujiId, SectionId::makerTags, unsignedLong,
     -1, printFujiDigitalZoom},
    {0x1045, "LensModulationOptimizer", N_("Lens Modulation Optimizer"), N_("Lens modulation optimizer setting"),
     IfdId::fujiId, SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(fujiOffOn)},
    {0x1047, "GrainEffectRoughness", N_("Grain Effect Roughness"), N_("Grain effect roughness setting"), IfdId::fujiId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(fujiOff0Weak32Strong64)},
    {0x1048, "ColorChromeEffect", N_("Color Chrome Effect"), N_("Color Chrome Effect"), IfdId::fujiId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(fujiOff0Weak32Strong64)},
    {0x1049, "MonochromaticColorWC", N_("Monochromatic Color (Warm-Cool)"),
     N_("Monochromatic color (warm-cool) setting. High value results in warm color shift and low values in cold color "
        "shift."),
     IfdId::fujiId, SectionId::makerTags, unsignedByte, -1, printFujiMonochromaticColor},
    {0x104b, "MonochromaticColorMG", N_("Monochromatic Color (Magenta-Green)"),
     N_("Monochromatic color (magenta-green) setting. High value results in magenta color shift and low values in "
        "green color shift."),
     IfdId::fujiId, SectionId::makerTags, unsignedByte, -1, printFujiMonochromaticColor},
    {0x104c, "GrainEffectSize", N_("Grain Effect Size"), N_("Grain effect size setting"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiOff0Weak32Strong64)},
    {0x104d, "CropMode", N_("Crop Mode"), N_("Crop mode"), IfdId::fujiId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(fujiCropMode)},
    {0x104e, "ColorChromeFXBlue", N_("Color Chrome FX Blue"), N_("Color Chrome FX Blue"), IfdId::fujiId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(fujiOff0Weak32Strong64)},
    {0x1050, "ShutterType", N_("Shutter Type"), N_("Shutter type"), IfdId::fujiId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(fujiShutterType)},
    {0x1100, "Continuous", N_("Continuous"), N_("Continuous shooting or auto bracketing setting"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiContinuous)},
    {0x1101, "SequenceNumber", N_("Sequence Number"), N_("Sequence number"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x1103, "DriveSetting", N_("Drive Setting"), N_("Drive setting"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, printFujiDriveSetting},
    {0x1105, "PixelShiftShots", N_("Pixel Shift Shots"), N_("Pixel shift shots"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x1105, "PixelShiftOfffset", N_("Pixel Shift Offset"), N_("Pixel shift offset"), IfdId::fujiId,
     SectionId::makerTags, signedRational, -1, printValue},
    {0x1153, "PanoramaAngle", N_("Panorama angle"), N_("Panorama angle"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x1154, "PanoramaDirection", N_("Panorama direction"), N_("Panorama direction"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiPanoramaDirection)},
    {0x1201, "AdvancedFilter", N_("Advanced filter"), N_("Advanced filter setting"), IfdId::fujiId,
     SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(fujiAdvancedFilter)},
    {0x1210, "FinePixColor", N_("FinePix Color"), N_("Fuji FinePix color setting"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(fujiFinePixColor)},
    {0x1300, "BlurWarning", N_("Blur Warning"), N_("Blur warning status"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(fujiOffOn)},
    {0x1301, "FocusWarning", N_("Focus Warning"), N_("Auto Focus warning status"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(fujiOffOn)},
    {0x1302, "ExposureWarning", N_("Exposure Warning"), N_("Auto exposure warning status"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiOffOn)},
    {0x1400, "DynamicRange", N_("Dynamic Range"), N_("Dynamic range"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(fujiDynamicRange)},
    {0x1401, "FilmMode", N_("Film Mode"), N_("Film mode"), IfdId::fujiId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(fujiFilmMode)},
    {0x1402, "DynamicRangeSetting", N_("Dynamic Range Setting"), N_("Dynamic range settings"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiDynamicRangeSetting)},
    {0x1403, "DevelopmentDynamicRange", N_("Development Dynamic Range"), N_("Development dynamic range"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1404, "MinFocalLength", N_("Minimum Focal Length"), N_("Minimum focal length"), IfdId::fujiId,
     SectionId::makerTags, unsignedRational, -1, printValue},
    {0x1405, "MaxFocalLength", N_("Maximum Focal Length"), N_("Maximum focal length"), IfdId::fujiId,
     SectionId::makerTags, unsignedRational, -1, printValue},
    {0x1406, "MaxApertureAtMinFocal", N_("Maximum Aperture at Minimum Focal"), N_("Maximum aperture at minimum focal"),
     IfdId::fujiId, SectionId::makerTags, unsignedRational, -1, printValue},
    {0x1407, "MaxApertureAtMaxFocal", N_("Maximum Aperture at Maximum Focal"), N_("Maximum aperture at maximum focal"),
     IfdId::fujiId, SectionId::makerTags, unsignedRational, -1, printValue},
    {0x140b, "AutoDynamicRange", N_("Auto Dynamic Range"), N_("Auto dynamic range"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1422, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1425, "SceneRecognition", N_("Scene recognition"), N_("Scene recognition"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(fujiSceneRecognition)},
    {0x1431, "Rating", N_("Rating"), N_("Rating"), IfdId::fujiId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x1436, "ImageGeneration", N_("Image Generation"), N_("Image generation"), IfdId::fujiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(fujiImageGeneration)},
    {0x1438, "ImageNumber", N_("Image Number"), N_("Image Number"), IfdId::fujiId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x1443, "DRangePriority", N_("D Range Priority"), N_("Dynamic range priority"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiDRangePriority)},
    {0x1444, "DRangePriorityFixed", N_("D Range Priority Fixed"), N_("Dynamic range priority fixed"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiDRangePriorityFixed)},
    {0x1445, "DRangePriorityAuto", N_("D Range Priority Auto"), N_("Dynamic range priority auto"), IfdId::fujiId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(fujiDRangePriorityAuto)},
    {0x4005, "FaceElementSelected", N_("Face Element Selected"), N_("Face element selected"), IfdId::fujiId,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x4100, "FacesDetected", N_("Faces Detected"), N_("Faces detected"), IfdId::fujiId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x4103, "FacePositions", N_("Face Positions"),
     N_("Left, top, right and bottom coordinates in full-sized image for each face detected"), IfdId::fujiId,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x4200, "NumberFaceElements", N_("Number of Face Elements"), N_("Number of face elements"), IfdId::fujiId,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x4201, "FaceElementTypes", N_("Face Element Types"), N_("Type of every face element"), IfdId::fujiId,
     SectionId::makerTags, asciiString, -1, printFujiFaceElementTypes},
    {0x4203, "FaceElementPositions", N_("Face Element Positions"),
     N_("Left, top, right and bottom coordinates in full-sized image for each face element)"), IfdId::fujiId,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x4282, "FaceRecInfo", N_("Face Recognition Information"), N_("Face Recognition Information"), IfdId::fujiId,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x8000, "FileSource", N_("File Source"), N_("File source"), IfdId::fujiId, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x8002, "OrderNumber", N_("Order Number"), N_("Order number"), IfdId::fujiId, SectionId::makerTags, unsignedLong,
     -1, printValue},
    {0x8003, "FrameNumber", N_("Frame Number"), N_("Frame number"), IfdId::fujiId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    // #1402
    {0xf000, "FujiIFD", N_("FujiIFD"), N_("Fujifilm IFD"), IfdId::fujiId, SectionId::makerTags, undefined, -1,
     printValue},
    {0xf001, "RawImageFullWidth", N_("Raw Image Full Width"), N_("Raw Image Full Width"), IfdId::fujiId,
     SectionId::makerTags, undefined, -1, printValue},
    {0xf002, "RawImageFullHeight", N_("Raw Image Full Height"), N_("Raw Image Full Height"), IfdId::fujiId,
     SectionId::makerTags, undefined, -1, printValue},
    {0xf003, "BitsPerSample", N_("Bits Per Sample"), N_("Bits Per Sample"), IfdId::fujiId, SectionId::makerTags,
     undefined, -1, printValue},
    {0xf007, "StripOffsets", N_("Strip Offsets"), N_("Strip Offsets"), IfdId::fujiId, SectionId::makerTags, undefined,
     -1, printValue},
    {0xf008, "StripByteCounts", N_("Strip Byte Counts"), N_("Strip Byte Counts"), IfdId::fujiId, SectionId::makerTags,
     undefined, -1, printValue},
    {0xf00a, "BlackLevel", N_("Black Level"), N_("Black Level"), IfdId::fujiId, SectionId::makerTags, undefined, -1,
     printValue},
    {0xf00b, "GeometricDistortionParams", N_("Geometric Distortion Params"), N_("Geometric Distortion Params"),
     IfdId::fujiId, SectionId::makerTags, undefined, -1, printValue},
    {0xf00c, "WB_GRBLevelsStandard", N_("WB GRB Levels Standard"), N_("WB GRB Levels Standard"), IfdId::fujiId,
     SectionId::makerTags, undefined, -1, printValue},
    {0xf00d, "WB_GRBLevelsAuto", N_("WB GRB Levels Auto"), N_("WB GRB Levels Auto"), IfdId::fujiId,
     SectionId::makerTags, undefined, -1, printValue},
    {0xf00e, "WB_GRBLevels", N_("WB GRB Levels"), N_("WB GRB Levels"), IfdId::fujiId, SectionId::makerTags, undefined,
     -1, printValue},
    {0xf00f, "ChromaticAberrationParams", N_("Chromatic Aberration Params"), N_("Chromatic Aberration Params"),
     IfdId::fujiId, SectionId::makerTags, undefined, -1, printValue},
    {0xf010, "VignettingParams", N_("Vignetting Params"), N_("Vignetting Params"), IfdId::fujiId, SectionId::makerTags,
     undefined, -1, printValue},

    // End of list marker
    {0xffff, "(UnknownFujiMakerNoteTag)", "(UnknownFujiMakerNoteTag)", N_("Unknown FujiMakerNote tag"), IfdId::fujiId,
     SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* FujiMakerNote::tagList() {
  return tagInfo_;
}

}  // namespace Exiv2::Internal
