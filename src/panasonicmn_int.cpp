// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "panasonicmn_int.hpp"
#include "i18n.h"  // NLS support.
#include "image_int.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "types.hpp"
#include "value.hpp"

// + standard includes
#include <cstddef>
#include <cstdint>
#include <ostream>

// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {
//! Quality, tag 0x0001
constexpr TagDetails panasonicQuality[] = {
    {1, N_("TIFF")}, {2, N_("High")},           {3, N_("Normal")},         {6, N_("Very High")},
    {7, N_("Raw")},  {9, N_("Motion Picture")}, {11, N_("Full HD Movie")}, {12, N_("4k Movie")},
};

//! WhiteBalance, tag 0x0003
constexpr TagDetails panasonicWhiteBalance[] = {
    {1, N_("Auto")},  {2, N_("Daylight")},         {3, N_("Cloudy")},  {4, N_("Halogen")}, {5, N_("Manual")},
    {8, N_("Flash")}, {10, N_("Black and white")}, {11, N_("Manual")}, {12, N_("Shade")},  {13, N_("Kelvin")},
};

//! FocusMode, tag 0x0007
constexpr TagDetails panasonicFocusMode[] = {
    {1, N_("Auto")}, {2, N_("Manual")}, {4, N_("Auto, focus button")}, {5, N_("Auto, continuous")}, {6, N_("AF-S")},
    {7, N_("AF-C")}, {8, N_("AF-F")},
};

//! ImageStabilizer, tag 0x001a
constexpr TagDetails panasonicImageStabilizer[] = {
    {2, N_("On, Mode 1")}, {3, N_("Off")}, {4, N_("On, Mode 2")}, {5, N_("Panning")}, {6, N_("On, Mode 3")},
};

//! Macro, tag 0x001c
constexpr TagDetails panasonicMacro[] = {
    {1, N_("On")},
    {2, N_("Off")},
    {257, N_("Tele-macro")},
    {513, N_("Macro-zoom")},
};

//! ShootingMode, tag 0x001f and SceneMode, tag 0x8001
constexpr TagDetails panasonicShootingMode[] = {
    {0, N_("Off")},  // only SceneMode
    {1, N_("Normal")},
    {2, N_("Portrait")},
    {3, N_("Scenery")},
    {4, N_("Sports")},
    {5, N_("Night portrait")},
    {6, N_("Program")},
    {7, N_("Aperture priority")},
    {8, N_("Shutter-speed priority")},
    {9, N_("Macro")},
    {10, N_("Spot")},
    {11, N_("Manual")},
    {12, N_("Movie preview")},
    {13, N_("Panning")},
    {14, N_("Simple")},
    {15, N_("Color effects")},
    {16, N_("Self Portrait")},
    {17, N_("Economy")},
    {18, N_("Fireworks")},
    {19, N_("Party")},
    {20, N_("Snow")},
    {21, N_("Night scenery")},
    {22, N_("Food")},
    {23, N_("Baby")},
    {24, N_("Soft skin")},
    {25, N_("Candlelight")},
    {26, N_("Starry night")},
    {27, N_("High sensitivity")},
    {28, N_("Panorama assist")},
    {29, N_("Underwater")},
    {30, N_("Beach")},
    {31, N_("Aerial photo")},
    {32, N_("Sunset")},
    {33, N_("Pet")},
    {34, N_("Intelligent ISO")},
    {35, N_("Clipboard")},
    {36, N_("High speed continuous shooting")},
    {37, N_("Intelligent auto")},
    {39, N_("Multi-aspect")},
    {41, N_("Transform")},
    {42, N_("Flash Burst")},
    {43, N_("Pin Hole")},
    {44, N_("Film Grain")},
    {45, N_("My Color")},
    {46, N_("Photo Frame")},
    {51, N_("HDR")},
    {55, N_("Handheld Night Shot")},
    {57, N_("3D")},
    {59, N_("Creative Control")},
    {62, N_("Panorama")},
    {63, N_("Glass Through")},
    {64, N_("HDR")},
    {66, N_("Digital Filter")},
    {67, N_("Clear Portrait")},
    {68, N_("Silky Skin")},
    {69, N_("Backlit Softness")},
    {70, N_("Clear in Backlight")},
    {71, N_("Relaxing Tone")},
    {72, N_("Sweet Child's Face")},
    {73, N_("Distinct Scenery")},
    {74, N_("Bright Blue Sky")},
    {75, N_("Romantic Sunset Glow")},
    {76, N_("Vivid Sunset Glow")},
    {77, N_("Glistening Water")},
    {78, N_("Clear Nightscape")},
    {79, N_("Cool Night Sky")},
    {80, N_("Warm Glowing Nightscape")},
    {81, N_("Artistic Nightscape")},
    {82, N_("Glittering Illuminations")},
    {83, N_("Clear Night Portrait")},
    {84, N_("Soft Image of a Flower")},
    {85, N_("Appetizing Food")},
    {86, N_("Cute Desert")},
    {87, N_("Freeze Animal Motion")},
    {88, N_("Clear Sports Shot")},
    {89, N_("Monochrome")},
    {90, N_("Creative Control")},
};

//! Audio, tag 0x0020
constexpr TagDetails panasonicAudio[] = {
    {1, N_("Yes")},
    {2, N_("No")},
    {3, N_("Stereo")},
};

//! ColorEffect, tag 0x0028
constexpr TagDetails panasonicColorEffect[] = {
    {1, N_("Off")}, {2, N_("Warm")}, {3, N_("Cool")}, {4, N_("Black and white")}, {5, N_("Sepia")}, {6, N_("Happy")},
};

//! BustMode, tag 0x002a
constexpr TagDetails panasonicBurstMode[] = {
    {0, N_("Off")},
    {1, N_("Low/High quality")},
    {2, N_("Infinite")},
};

//! Contrast, tag 0x002c
constexpr TagDetails panasonicContrast[] = {
    {0, N_("Normal")},      {1, N_("Low")},   {2, N_("High")},       {6, N_("Medium low")},
    {7, N_("Medium high")}, {256, N_("Low")}, {272, N_("Standard")}, {288, N_("High")},
};

//! NoiseReduction, tag 0x002d
constexpr TagDetails panasonicNoiseReduction[] = {
    {0, N_("Standard")}, {1, N_("Low (-1)")}, {2, N_("High (+1)")}, {3, N_("Lowest (-2)")}, {4, N_("Highest (+2)")},
};

//! SelfTimer, tag 0x002e
constexpr TagDetails panasonicSelfTimer[] = {
    {1, N_("Off")},
    {2, "10 s"},
    {3, "2 s"},
    {4, "10 s / 3 pictures"},
};

//! Rotation, tag 0x0030
constexpr TagDetails panasonicRotation[] = {
    {1, N_("Horizontal (normal)")},
    {3, N_("Rotate 180")},
    {6, N_("Rotate 90 CW")},
    {8, N_("Rotate 270 CW")},
};

//! AFAssistLamp, tag 0x0031
constexpr TagDetails panasonicAFAssistLamp[] = {
    {1, N_("Fired")},
    {2, N_("Enabled but Not Used")},
    {3, N_("Disabled but Required")},
    {4, N_("Disabled and Not Required")},
};

//! ColorMode, tag 0x0032
constexpr TagDetails panasonicColorMode[] = {
    {0, N_("Normal")},
    {1, N_("Natural")},
    {2, N_("Vivid")},
};

//! OpticalZoomMode, tag 0x0034
constexpr TagDetails panasonicOpticalZoomMode[] = {
    {1, N_("Standard")},
    {2, N_("EX optics")},
};

//! ConversionLens, tag 0x0035
constexpr TagDetails panasonicConversionLens[] = {
    {1, N_("Off")},
    {2, N_("Wide")},
    {3, N_("Telephoto")},
    {4, N_("Macro")},
};

//! WorldTimeLocation, tag 0x003a
constexpr TagDetails panasonicWorldTimeLocation[] = {
    {1, N_("Home")},
    {2, N_("Destination")},
};

//! TextStamp, tag 0x003b, 0x003e, 000x8008 and 0x8009
constexpr TagDetails panasonicTextStamp[] = {
    {1, N_("Off")},
    {2, N_("On")},
};

//! FilmMode, tag 0x0042
constexpr TagDetails panasonicFilmMode[] = {
    {1, N_("Standard (color)")}, {2, N_("Dynamic (color)")}, {3, N_("Nature (color)")},
    {4, N_("Smooth (color)")},   {5, N_("Standard (B&W)")},  {6, N_("Dynamic (B&W)")},
    {7, N_("Smooth (B&W)")},     {10, N_("Nostalgic")},      {11, N_("Vibrant")},
};

//! Bracket Settings, tag 0x0045
constexpr TagDetails panasonicBracketSettings[] = {
    {0, N_("No Bracket")},
    {1, N_("3 images, Sequence 0/-/+")},
    {2, N_("3 images, Sequence -/0/+")},
    {3, N_("5 images, Sequence 0/-/+")},
    {4, N_("5 images, Sequence -/0/+")},
    {5, N_("7 images, Sequence 0/-/+")},
    {6, N_("7 images, Sequence -/0/+")},
};

//! Flash curtain, tag 0x0048
constexpr TagDetails panasonicFlashCurtain[] = {
    {0, N_("n/a")},
    {1, N_("1st")},
    {2, N_("2nd")},
};

//! Long Shutter Noise Reduction, tag 0x0049
constexpr TagDetails panasonicLongShutterNoiseReduction[] = {
    {1, N_("Off")},
    {2, N_("On")},
};

//! Intelligent exposure, tag 0x005d
constexpr TagDetails panasonicIntelligentExposure[] = {
    {0, N_("Off")},
    {1, N_("Low")},
    {2, N_("Standard")},
    {3, N_("High")},
};

//! Flash warning, tag 0x0062
constexpr TagDetails panasonicFlashWarning[] = {
    {0, N_("No")},
    {1, N_("Yes (flash required but disabled")},
};

//! Intelligent resolution, tag 0x0070
constexpr TagDetails panasonicIntelligentResolution[] = {
    {0, N_("Off")}, {1, N_("Low")}, {2, N_("Standard")}, {3, N_("High")}, {4, N_("Extended")},
};

//! Intelligent D-Range, tag 0x0079
constexpr TagDetails panasonicIntelligentDRange[] = {
    {0, N_("Off")},
    {1, N_("Low")},
    {2, N_("Standard")},
    {3, N_("High")},
};

//! Clear Retouch, tag 0x007c
constexpr TagDetails panasonicClearRetouch[] = {
    {0, N_("Off")},
    {1, N_("On")},
};

//! Photo Style, tag 0x0089
constexpr TagDetails panasonicPhotoStyle[] = {
    {0, N_("NoAuto")},  {1, N_("Standard or Custom")}, {2, N_("Vivid")}, {3, N_("Natural")}, {4, N_("Monochrome")},
    {5, N_("Scenery")}, {6, N_("Portrait")},
};

//! Shading compensation, tag 0x008a
constexpr TagDetails panasonicShadingCompensation[] = {
    {0, N_("Off")},
    {1, N_("On")},
};

//! Camera orientation, tag 0x008f
constexpr TagDetails panasonicCameraOrientation[] = {
    {0, N_("Normal")},     {1, N_("Rotate CW")},    {2, N_("Rotate 180")},
    {3, N_("Rotate CCW")}, {4, N_("Tilt upwards")}, {5, N_("Tilt downwards")},
};

//! Sweep panorama direction, tag 0x0093
constexpr TagDetails panasonicSweepPanoramaDirection[] = {
    {0, N_("Off")},           {1, N_("Left to Right")}, {2, N_("Right to Left")},
    {3, N_("Top to Bottom")}, {4, N_("Bottom to Top")},
};

//! Timer recording, tag 0x0096
constexpr TagDetails panasonicTimerRecording[] = {
    {0, N_("Off")},
    {1, N_("Time Lapse")},
    {2, N_("Stop-Motion Animation")},
};

//! HDR, tag 0x009e
constexpr TagDetails panasonicHDR[] = {
    {0, N_("Off")},
    {100, N_("1 EV")},
    {200, N_("2 EV")},
    {300, N_("3 EV")},
    {32868, N_("1 EV (Auto)")},
    {32968, N_("2 EV (Auto)")},
    {33068, N_("3 EV (Auto)")},
};

//! Shutter Type, tag 0x009f
constexpr TagDetails panasonicShutterType[] = {
    {0, N_("Mechanical")},
    {1, N_("Electronic")},
    {2, N_("Hybrid")},
};

//! Touch AE, tag 0x00ab
constexpr TagDetails panasonicTouchAE[] = {
    {0, N_("Off")},
    {1, N_("On")},
};

//! Flash Fired, tag 0x8007
constexpr TagDetails panasonicFlashFired[] = {
    {1, N_("No")},
    {2, N_("Yes")},
};

// Panasonic MakerNote Tag Info
constexpr TagInfo PanasonicMakerNote::tagInfo_[] = {
    {0x0001, "Quality", N_("Quality"), N_("Image Quality"), IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(panasonicQuality)},
    {0x0002, "FirmwareVersion", N_("Firmware Version"), N_("Firmware version"), IfdId::panasonicId,
     SectionId::makerTags, undefined, -1, printValue},
    {0x0003, "WhiteBalance", N_("White Balance"), N_("White balance setting"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicWhiteBalance)},
    {0x0004, "0x0004", "0x0004", N_("Unknown"), IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0007, "FocusMode", N_("Focus Mode"), N_("Focus mode"), IfdId::panasonicId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(panasonicFocusMode)},
    {0x000f, "AFMode", N_("AF Mode"), N_("AF mode"), IfdId::panasonicId, SectionId::makerTags, unsignedByte, -1,
     print0x000f},
    {0x001a, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicImageStabilizer)},
    {0x001c, "Macro", N_("Macro"), N_("Macro mode"), IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(panasonicMacro)},
    {0x001f, "ShootingMode", N_("Shooting Mode"), N_("Shooting mode"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicShootingMode)},
    {0x0020, "Audio", N_("Audio"), N_("Audio"), IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(panasonicAudio)},
    {0x0021, "DataDump", N_("Data Dump"), N_("Data dump"), IfdId::panasonicId, SectionId::makerTags, undefined, -1,
     printValue},
    {0x0022, "0x0022", "0x0022", N_("Unknown"), IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0023, "WhiteBalanceBias", N_("White Balance Bias"), N_("White balance adjustment"), IfdId::panasonicId,
     SectionId::makerTags, signedShort, -1, print0x0023},
    {0x0024, "FlashBias", N_("FlashBias"), N_("Flash bias"), IfdId::panasonicId, SectionId::makerTags, signedShort, -1,
     printValue},
    {0x0025, "InternalSerialNumber", N_("Internal Serial Number"),
     N_("This number is unique, and contains the date of manufacture, but is not the same as the number printed "
        "on "
        "the camera body."),
     IfdId::panasonicId, SectionId::makerTags, undefined, -1, printPanasonicText},
    {0x0026, "ExifVersion", "Exif Version", N_("Exif version"), IfdId::panasonicId, SectionId::makerTags, undefined, -1,
     printExifVersion},
    {0x0027, "0x0027", "0x0027", N_("Unknown"), IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0028, "ColorEffect", N_("Color Effect"), N_("Color effect"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicColorEffect)},
    {0x0029, "TimeSincePowerOn", "Time since Power On",
     N_("Time in 1/100 s from when the camera was powered on to when the image is written to memory card"),
     IfdId::panasonicId, SectionId::makerTags, unsignedLong, -1, print0x0029},
    {0x002a, "BurstMode", N_("Burst Mode"), N_("Burst mode"), IfdId::panasonicId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(panasonicBurstMode)},
    {0x002b, "SequenceNumber", N_("Sequence Number"), N_("Sequence number"), IfdId::panasonicId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    {0x002c, "Contrast", N_("Contrast"), N_("Contrast setting"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicContrast)},
    {0x002d, "NoiseReduction", N_("NoiseReduction"), N_("Noise reduction"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicNoiseReduction)},
    {0x002e, "SelfTimer", N_("Self Timer"), N_("Self timer"), IfdId::panasonicId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(panasonicSelfTimer)},
    {0x002f, "0x002f", "0x002f", N_("Unknown"), IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0030, "Rotation", N_("Rotation"), N_("Rotation"), IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(panasonicRotation)},
    {0x0031, "AFAssistLamp", N_("AF Assist Lamp"), N_("AF Assist Lamp"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicAFAssistLamp)},
    {0x0032, "ColorMode", N_("Color Mode"), N_("Color mode"), IfdId::panasonicId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(panasonicColorMode)},
    {0x0033, "BabyAge1", N_("Baby Age 1"), N_("Baby (or pet) age 1"), IfdId::panasonicId, SectionId::makerTags,
     asciiString, -1, print0x0033},
    {0x0034, "OpticalZoomMode", N_("Optical Zoom Mode"), N_("Optical zoom mode"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicOpticalZoomMode)},
    {0x0035, "ConversionLens", N_("Conversion Lens"), N_("Conversion lens"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicConversionLens)},
    {0x0036, "TravelDay", N_("Travel Day"), N_("Travel day"), IfdId::panasonicId, SectionId::makerTags, unsignedShort,
     -1, print0x0036},
    {0x0039, "Contrast2", N_("Contrast 2"), N_("Contrast 2"), IfdId::panasonicId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x003a, "WorldTimeLocation", N_("World Time Location"), N_("World time location"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicWorldTimeLocation)},
    {0x003b, "TextStamp1", N_("Text Stamp 1"), N_("Text Stamp 1"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicTextStamp)},
    {0x003c, "ProgramISO", N_("Program ISO"), N_("Program ISO"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, print0x003c},
    {0x003d, "AdvancedSceneType", N_("Advanced Scene Type"), N_("Advanced Scene Type"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x003e, "TextStamp2", N_("Text Stamp 2"), N_("Text Stamp 2"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicTextStamp)},
    {0x003f, "FacesDetected", N_("Faces detected"), N_("Faces detected"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0040, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::panasonicId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0041, "Sharpness", N_("Sharpness"), N_("Sharpness"), IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0042, "FilmMode", N_("Film Mode"), N_("Film mode"), IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(panasonicFilmMode)},
    {0x0044, "ColorTempKelvin", N_("Color Temp Kelvin"), N_("Color Temperature in Kelvin"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0045, "BracketSettings", N_("Bracket Settings"), N_("Bracket Settings"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicBracketSettings)},
    {0x0046, "WBAdjustAB", N_("WB Adjust AB"), N_("WB adjust AB. Positive is a shift toward blue."), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0047, "WBAdjustGM", N_("WB Adjust GM"), N_("WBAdjustGM. Positive is a shift toward green."), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0048, "FlashCurtain", N_("Flash Curtain"), N_("Flash Curtain"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicFlashCurtain)},
    {0x0049, "LongShutterNoiseReduction", N_("Long Shutter Noise Reduction"), N_("Long Shutter Noise Reduction"),
     IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicLongShutterNoiseReduction)},
    {0x004b, "ImageWidth", N_("Image width"), N_("Image width"), IfdId::panasonicId, SectionId::makerTags, unsignedLong,
     -1, printValue},
    {0x004c, "ImageHeight", N_("Image height"), N_("Image height"), IfdId::panasonicId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    {0x004d, "AFPointPosition", N_("AF Point Position"), N_("AF Point Position"), IfdId::panasonicId,
     SectionId::makerTags, unsignedRational, -1, printValue},
    {0x004e, "FaceDetInfo", N_("Face detection info"), N_("Face detection info"), IfdId::panasonicId,
     SectionId::makerTags, undefined, -1, printValue},
    {0x0051, "LensType", N_("Lens Type"), N_("Lens type"), IfdId::panasonicId, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x0052, "LensSerialNumber", N_("Lens Serial Number"), N_("Lens serial number"), IfdId::panasonicId,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x0053, "AccessoryType", N_("Accessory Type"), N_("Accessory type"), IfdId::panasonicId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0054, "AccessorySerialNumber", N_("Accessory Serial Number"), N_("Accessory Serial Number"), IfdId::panasonicId,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x0059, "Transform1", N_("Transform 1"), N_("Transform 1"), IfdId::panasonicId, SectionId::makerTags, undefined,
     -1, printValue},
    {0x005d, "IntelligentExposure", N_("Intelligent Exposure"), N_("Intelligent Exposure"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicIntelligentExposure)},
    {0x0060, "LensFirmwareVersion", N_("Firmware Version of the Lens"), N_("Firmware Version of the Lens"),
     IfdId::panasonicId, SectionId::makerTags, undefined, -1, printValue},
    {0x0061, "FaceRecInfo", N_("Face recognition info"), N_("Face recognition info"), IfdId::panasonicId,
     SectionId::makerTags, undefined, -1, printValue},
    {0x0062, "FlashWarning", N_("Flash Warning"), N_("Flash warning"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicFlashWarning)},
    {0x0065, "Title", N_("Title"), N_("Title"), IfdId::panasonicId, SectionId::makerTags, undefined, -1,
     printPanasonicText},
    {0x0066, "BabyName", N_("Baby Name"), N_("Baby name (or pet name)"), IfdId::panasonicId, SectionId::makerTags,
     undefined, -1, printPanasonicText},
    {0x0067, "Location", N_("Location"), N_("Location"), IfdId::panasonicId, SectionId::makerTags, undefined, -1,
     printPanasonicText},
    {0x0069, "Country", N_("Country"), N_("Country"), IfdId::panasonicId, SectionId::makerTags, undefined, -1,
     printPanasonicText},
    {0x006b, "State", N_("State"), N_("State"), IfdId::panasonicId, SectionId::makerTags, undefined, -1,
     printPanasonicText},
    {0x006d, "City", N_("City"), N_("City"), IfdId::panasonicId, SectionId::makerTags, undefined, -1,
     printPanasonicText},
    {0x006f, "Landmark", N_("Landmark"), N_("Landmark"), IfdId::panasonicId, SectionId::makerTags, undefined, -1,
     printPanasonicText},
    {0x0070, "IntelligentResolution", N_("Intelligent resolution"), N_("Intelligent resolution"), IfdId::panasonicId,
     SectionId::makerTags, unsignedByte, -1, EXV_PRINT_TAG(panasonicIntelligentResolution)},
    {0x0077, "BurstSpeed", N_("Burst Speed"), N_("Burst Speed in pictures per second"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0079, "IntelligentDRange", N_("Intelligent Dynamic Range"), N_("Intelligent Dynamic Range"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicIntelligentDRange)},
    {0x007c, "ClearRetouch", N_("Clear Retouch"), N_("Clear Retouch"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicClearRetouch)},
    {0x0080, "City2", N_("City2"), N_("City2"), IfdId::panasonicId, SectionId::makerTags, undefined, -1,
     printPanasonicText},
    {0x0086, "ManometerPressure", N_("Manometer Pressure"), N_("Manometer pressure"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, printPressure},
    {0x0089, "PhotoStyle", N_("Photo style"), N_("Photo style"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicPhotoStyle)},
    {0x008a, "ShadingCompensation", N_("Shading Compensation"), N_("Shading Compensation"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(panasonicShadingCompensation)},
    {0x008c, "AccelerometerZ", N_("Accelerometer Z"), N_("positive is acceleration upwards"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, printAccelerometer},
    {0x008d, "AccelerometerX", N_("Accelerometer X"), N_("positive is acceleration to the left"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, printAccelerometer},
    {0x008e, "AccelerometerY", N_("Accelerometer Y"), N_("positive is acceleration backwards"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, printAccelerometer},
    {0x008f, "CameraOrientation", N_("Camera Orientation"), N_("Camera Orientation"), IfdId::panasonicId,
     SectionId::makerTags, unsignedByte, -1, EXV_PRINT_TAG(panasonicCameraOrientation)},
    {0x0090, "RollAngle", N_("Roll Angle"), N_("degrees of clockwise camera rotation"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, printRollAngle},
    {0x0091, "PitchAngle", N_("Pitch Angle"), N_("degrees of upwards camera tilt"), IfdId::panasonicId,
     SectionId::makerTags, unsignedShort, -1, printPitchAngle},
    {0x0093, "SweepPanoramaDirection", N_("Sweep Panorama Direction"), N_("Sweep Panorama Direction"),
     IfdId::panasonicId, SectionId::makerTags, unsignedByte, -1, EXV_PRINT_TAG(panasonicSweepPanoramaDirection)},
    {0x0094, "PanoramaFieldOfView", N_("Field of View of Panorama"), N_("Field of View of Panorama"),
     IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0096, "TimerRecording", N_("Timer Recording"), N_("Timer Recording"), IfdId::panasonicId, SectionId::makerTags,
     unsignedByte, -1, EXV_PRINT_TAG(panasonicTimerRecording)},
    {0x009d, "InternalNDFilter", N_("Internal ND Filter"), N_("Internal ND Filter"), IfdId::panasonicId,
     SectionId::makerTags, unsignedRational, -1, printValue},
    {0x009e, "HDR", N_("HDR"), N_("HDR"), IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(panasonicHDR)},
    {0x009f, "ShutterType", N_("Shutter Type"), N_("Shutter Type"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicShutterType)},
    {0x00a3, "ClearRetouchValue", N_("Clear Retouch Value"), N_("Clear Retouch Value"), IfdId::panasonicId,
     SectionId::makerTags, unsignedRational, -1, printValue},
    {0x00ab, "TouchAE", N_("TouchAE"), N_("TouchAE"), IfdId::panasonicId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(panasonicTouchAE)},
    {0x0e00, "PrintIM", N_("Print IM"), N_("PrintIM information"), IfdId::panasonicId, SectionId::makerTags, undefined,
     -1, printValue},
    {0x4449, "0x4449", "0x4449", N_("Unknown"), IfdId::panasonicId, SectionId::makerTags, undefined, -1, printValue},
    {0x8000, "MakerNoteVersion", N_("MakerNote Version"), N_("MakerNote version"), IfdId::panasonicId,
     SectionId::makerTags, undefined, -1, printExifVersion},
    {0x8001, "SceneMode", N_("Scene Mode"), N_("Scene mode"), IfdId::panasonicId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(panasonicShootingMode)},
    {0x8004, "WBRedLevel", N_("WB Red Level"), N_("WB red level"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x8005, "WBGreenLevel", N_("WB Green Level"), N_("WB green level"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x8006, "WBBlueLevel", N_("WB Blue Level"), N_("WB blue level"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x8007, "FlashFired", N_("Flash Fired"), N_("Flash Fired"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicFlashFired)},
    {0x8008, "TextStamp3", N_("Text Stamp 3"), N_("Text Stamp 3"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicTextStamp)},
    {0x8009, "TextStamp4", N_("Text Stamp 4"), N_("Text Stamp 4"), IfdId::panasonicId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(panasonicTextStamp)},
    {0x8010, "BabyAge2", N_("Baby Age 2"), N_("Baby (or pet) age 2"), IfdId::panasonicId, SectionId::makerTags,
     asciiString, -1, print0x0033},
    {0x8012, "Transform2", N_("Transform 2"), N_("Transform 2"), IfdId::panasonicId, SectionId::makerTags, undefined,
     -1, printValue},
    // End of list marker
    {0xffff, "(UnknownPanasonicMakerNoteTag)", "(UnknownPanasonicMakerNoteTag)", N_("Unknown PanasonicMakerNote tag"),
     IfdId::panasonicId, SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* PanasonicMakerNote::tagList() {
  return tagInfo_;
}

// tag Focus Mode
std::ostream& PanasonicMakerNote::print0x000f(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() < 2 || value.typeId() != unsignedByte) {
    return os << value;
  }
  const auto l0 = value.toInt64(0);
  const auto l1 = value.toInt64(1);
  if (l0 == 0 && l1 == 1)
    os << _("Spot mode on or 9 area");
  else if (l0 == 0 && l1 == 16)
    os << _("Spot mode off or 3-area (high speed)");
  else if (l0 == 0 && l1 == 23)
    os << _("23-area");
  else if (l0 == 0 && l1 == 49)
    os << _("49-area");
  else if (l0 == 0 && l1 == 225)
    os << _("225-area");
  else if (l0 == 1 && l1 == 0)
    os << _("Spot focusing");
  else if (l0 == 1 && l1 == 1)
    os << _("5-area");
  else if (l0 == 16 && l1 == 0)
    os << _("1-area");
  else if (l0 == 16 && l1 == 16)
    os << _("1-area (high speed)");
  else if (l0 == 32 && l1 == 0)
    os << _("3-area (auto)");
  else if (l0 == 32 && l1 == 1)
    os << _("3-area (left)");
  else if (l0 == 32 && l1 == 2)
    os << _("3-area (center)");
  else if (l0 == 32 && l1 == 3)
    os << _("3-area (right)");
  else if (l0 == 64 && l1 == 0)
    os << _("Face Detect");
  else if (l0 == 128 && l1 == 0)
    os << _("Spot Focusing 2");
  else if (l0 == 240 && l1 == 0)
    os << _("Tracking");
  else
    os << value;
  return os;
}  // PanasonicMakerNote::print0x000f

// tag White balance bias
std::ostream& PanasonicMakerNote::print0x0023(std::ostream& os, const Value& value, const ExifData*) {
  return os << stringFormat("{:1}{}", value.toInt64() / 3, _(" EV"));
}  // PanasonicMakerNote::print0x0023

// Time since power on
std::ostream& PanasonicMakerNote::print0x0029(std::ostream& os, const Value& value, const ExifData*) {
  auto time = value.toInt64();
  return os << stringFormat("{:02}:{:02}:{:02}.{:02}", time / 360000, (time % 360000) / 6000, (time % 6000) / 100,
                            time % 100);

}  // PanasonicMakerNote::print0x0029

// baby age
std::ostream& PanasonicMakerNote::print0x0033(std::ostream& os, const Value& value, const ExifData*) {
  if (value.toString() == "9999:99:99 00:00:00") {
    os << N_("not set");
  } else {
    os << value;
  }
  return os;
}  // PanasonicMakerNote::print0x0033

// Travel days
std::ostream& PanasonicMakerNote::print0x0036(std::ostream& os, const Value& value, const ExifData*) {
  if (value.toInt64() == 65535)
    os << N_("not set");
  else
    os << value;
  return os;
}  // PanasonicMakerNote::print0x0036

// Program ISO
std::ostream& PanasonicMakerNote::print0x003c(std::ostream& os, const Value& value, const ExifData*) {
  switch (value.toInt64()) {
    case 65534:
      os << N_("Intelligent ISO");
      break;
    case 65535:
      os << N_("n/a");
      break;
    default:
      os << value;
      break;
  }
  return os;
}  // PanasonicMakerNote::print0x003c

std::ostream& PanasonicMakerNote::printPanasonicText(std::ostream& os, const Value& value, const ExifData*) {
  if (value.size() > 0 && value.typeId() == undefined) {
    for (size_t i = 0; i < value.size(); i++) {
      if (value.toInt64(i) == 0) {
        break;
      }
      os << static_cast<char>(value.toInt64(i));
    }
    return os;
  }

  return os << value;
}  // PanasonicMakerNote::printPanasonicText

// Manometer Pressure
std::ostream& PanasonicMakerNote::printPressure(std::ostream& os, const Value& value, const ExifData*) {
  if (value.toInt64() == 65535)
    os << N_("infinite");
  else
    os << value << N_(" hPa");
  return os;
}  // PanasonicMakerNote::printPressure

std::ostream& PanasonicMakerNote::printAccelerometer(std::ostream& os, const Value& value, const ExifData*) {
  // value is stored as unsigned int, but should be read as int16_t.
  const auto i = static_cast<int16_t>(value.toInt64());
  return os << i;
}  // PanasonicMakerNote::printAccelerometer

std::ostream& PanasonicMakerNote::printRollAngle(std::ostream& os, const Value& value, const ExifData*) {
  // value is stored as unsigned int, but should be read as int16_t.
  const auto i = static_cast<int16_t>(value.toInt64());
  return os << stringFormat("{:.1f}", i / 10.0);
}  // PanasonicMakerNote::printRollAngle

std::ostream& PanasonicMakerNote::printPitchAngle(std::ostream& os, const Value& value, const ExifData*) {
  // value is stored as unsigned int, but should be read as int16_t.
  const auto i = static_cast<int16_t>(value.toInt64());
  return os << stringFormat("{:.1f}", -i / 10.0);
}  // PanasonicMakerNote::printPitchAngle

// Panasonic MakerNote Tag Info
constexpr TagInfo PanasonicMakerNote::tagInfoRaw_[] = {
    {0x0001, "Version", N_("Version"), N_("Panasonic raw version"), IfdId::panaRawId, SectionId::panaRaw, undefined, -1,
     printExifVersion},
    {0x0002, "SensorWidth", N_("Sensor Width"), N_("Sensor width"), IfdId::panaRawId, SectionId::panaRaw, unsignedShort,
     -1, printValue},
    {0x0003, "SensorHeight", N_("Sensor Height"), N_("Sensor height"), IfdId::panaRawId, SectionId::panaRaw,
     unsignedShort, -1, printValue},
    {0x0004, "SensorTopBorder", N_("Sensor Top Border"), N_("Sensor top border"), IfdId::panaRawId, SectionId::panaRaw,
     unsignedShort, -1, printValue},
    {0x0005, "SensorLeftBorder", N_("Sensor Left Border"), N_("Sensor left border"), IfdId::panaRawId,
     SectionId::panaRaw, unsignedShort, -1, printValue},
    {0x0006, "ImageHeight", N_("Image Height"), N_("Image height"), IfdId::panaRawId, SectionId::panaRaw, unsignedShort,
     -1, printValue},
    {0x0007, "ImageWidth", N_("Image Width"), N_("Image width"), IfdId::panaRawId, SectionId::panaRaw, unsignedShort,
     -1, printValue},
    {0x0011, "RedBalance", N_("Red Balance"), N_("Red balance (found in Digilux 2 RAW images)"), IfdId::panaRawId,
     SectionId::panaRaw, unsignedShort, -1, printValue},
    {0x0012, "BlueBalance", N_("Blue Balance"), N_("Blue balance"), IfdId::panaRawId, SectionId::panaRaw, unsignedShort,
     -1, printValue},
    {0x0017, "ISOSpeed", N_("ISO Speed"), N_("ISO speed setting"), IfdId::panaRawId, SectionId::panaRaw, unsignedShort,
     -1, printValue},
    {0x0024, "WBRedLevel", N_("WB Red Level"), N_("WB red level"), IfdId::panaRawId, SectionId::panaRaw, unsignedShort,
     -1, printValue},
    {0x0025, "WBGreenLevel", N_("WB Green Level"), N_("WB green level"), IfdId::panaRawId, SectionId::panaRaw,
     unsignedShort, -1, printValue},
    {0x0026, "WBBlueLevel", N_("WB Blue Level"), N_("WB blue level"), IfdId::panaRawId, SectionId::panaRaw,
     unsignedShort, -1, printValue},
    {0x002e, "PreviewImage", N_("Preview Image"), N_("Preview image"), IfdId::panaRawId, SectionId::panaRaw, undefined,
     -1, printValue},
    {0x010f, "Make", N_("Manufacturer"), N_("The manufacturer of the recording equipment"), IfdId::panaRawId,
     SectionId::panaRaw, asciiString, -1, printValue},
    {0x0110, "Model", N_("Model"), N_("The model name or model number of the equipment"), IfdId::panaRawId,
     SectionId::panaRaw, asciiString, -1, printValue},
    {0x0111, "StripOffsets", N_("Strip Offsets"), N_("Strip offsets"), IfdId::panaRawId, SectionId::panaRaw,
     unsignedLong, -1, printValue},
    {0x0112, "Orientation", N_("Orientation"), N_("Orientation"), IfdId::panaRawId, SectionId::panaRaw, unsignedShort,
     -1, print0x0112},
    {0x0116, "RowsPerStrip", N_("Rows Per Strip"), N_("The number of rows per strip"), IfdId::panaRawId,
     SectionId::panaRaw, unsignedShort, -1, printValue},
    {0x0117, "StripByteCounts", N_("Strip Byte Counts"), N_("Strip byte counts"), IfdId::panaRawId, SectionId::panaRaw,
     unsignedLong, -1, printValue},
    {0x0118, "RawDataOffset", N_("Raw Data Offset"), N_("Raw data offset"), IfdId::panaRawId, SectionId::panaRaw,
     unsignedLong, -1, printValue},
    {0x0119, "DistortionInfo", N_("Distortion Info"), N_("Distortion info"), IfdId::panaRawId, SectionId::panaRaw,
     signedShort, -1, printValue},
    {0x011c, "Gamma", N_("Gamma"), N_("Gamma"), IfdId::panaRawId, SectionId::panaRaw, unsignedShort, -1, printValue},
    {0x013b, "Artist", N_("Artist"), N_("Artist"), IfdId::panaRawId, SectionId::panaRaw, asciiString, -1, printValue},
    {0x8298, "Copyright", N_("Copyright"), N_("Copyright"), IfdId::panaRawId, SectionId::panaRaw, asciiString, -1,
     printValue},
    {0x8769, "ExifTag", N_("Exif IFD Pointer"), N_("A pointer to the Exif IFD"), IfdId::panaRawId, SectionId::panaRaw,
     unsignedLong, -1, printValue},
    {0x8825, "GPSTag", N_("GPS Info IFD Pointer"), N_("A pointer to the GPS Info IFD"), IfdId::panaRawId,
     SectionId::panaRaw, unsignedLong, -1, printValue},
    // End of list marker
    {0xffff, "(UnknownPanasonicRawTag)", "(UnknownPanasonicRawTag)", N_("Unknown PanasonicRaw tag"), IfdId::panaRawId,
     SectionId::panaRaw, asciiString, -1, printValue},
};

const TagInfo* PanasonicMakerNote::tagListRaw() {
  return tagInfoRaw_;
}

}  // namespace Exiv2::Internal
