// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  File:      olympusmn.cpp
  Version:   $Rev$
  Author(s): Will Stokes (wuz) <wstokes@gmail.com>
             Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (gc) <caulier.gilles@kdemail.net>
  History:   10-Mar-05, wuz: created
  Credits:   See header file.
 */

// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "olympusmn.hpp"
#include "value.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exiv2 {


    //! OffOn, multiple tags
    extern const TagDetails olympusOffOn[] = {
        {  0, N_("Off") },
        {  1, N_("On")  }
    };

    //! NoYes, multiple tags
    extern const TagDetails olympusNoYes[] = {
        {  0, N_("No") },
        {  1, N_("Yes")  }
    };

    //! Quality, tag 0x0201
    extern const TagDetails olympusQuality[] = {
        { 1, N_("Standard Quality (SQ)")    },
        { 2, N_("High Quality (HQ)")        },
        { 3, N_("Super High Quality (SHQ)") },
        { 6, N_("Raw")                      }
    };

    //! Macro, tag 0x0202
    extern const TagDetails olympusMacro[] = {
        {  0, N_("Off")         },
        {  1, N_("On")          },
        {  2, N_("Super macro") }
    };

    //! OneTouchWB, tag 0x0302
    extern const TagDetails olympusOneTouchWb[] = {
        {  0, N_("Off")         },
        {  1, N_("On")          },
        {  2, N_("On (preset)") }
    };

    //! FlashDevice, tag 0x1005
    extern const TagDetails olympusFlashDevice[] = {
        {  0, N_("None")                },
        {  1, N_("Internal")            },
        {  4, N_("External")            },
        {  5, N_("Internal + External") }
    };

    //! FocusMode, tag 0x100b
    extern const TagDetails olympusFocusMode[] = {
        {  0, N_("Auto")   },
        {  1, N_("Manual") }
    };

    //! Sharpness, tag 0x100f
    extern const TagDetails olympusSharpness[] = {
        { 0, N_("Normal") },
        { 1, N_("Hard")   },
        { 2, N_("Soft")   }
    };

    //! Contrast, tag 0x1029
    extern const TagDetails olympusContrast[] = {
        { 0, N_("High")   },
        { 1, N_("Normal") },
        { 2, N_("Low")    }
    };

    //! CCDScanMode, tag 0x1039
    extern const TagDetails olympusCCDScanMode[] = {
        {  0, N_("Interlaced")  },
        {  1, N_("Progressive") }
    };

    // Olympus Tag Info
    const TagInfo OlympusMakerNote::tagInfo_[] = {

        /* TODO:
           add Minolta makenotes tags here (0x0000-0x0103). See Exiftool database.*/
        TagInfo(0x0100, "ThumbnailImage", N_("Thumbnail Image"),
                N_("Thumbnail image"),
                olympusIfdId, makerTags, undefined, printValue),

        TagInfo(0x0200, "SpecialMode", N_("Special Mode"),
                N_("Picture taking mode"),
                olympusIfdId, makerTags, unsignedLong, print0x0200),
        TagInfo(0x0201, "Quality", N_("Quality"),
                N_("Image quality setting"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusQuality)),
        TagInfo(0x0202, "Macro", N_("Macro"),
                N_("Macro mode"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusMacro)),
        TagInfo(0x0203, "BWMode", N_("Black & White Mode"),
                N_("Black and white mode"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOffOn)),
        TagInfo(0x0204, "DigitalZoom", N_("Digital Zoom"),
                N_("Digital zoom ratio"),
                olympusIfdId, makerTags, unsignedRational, print0x0204),
        TagInfo(0x0205, "FocalPlaneDiagonal", N_("Focal Plane Diagonal"),
                N_("Focal plane diagonal"),
                olympusIfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0206, "LensDistortionParams", N_("Lens Distortion Parameters"),
                N_("Lens distortion parameters"),
                olympusIfdId, makerTags, signedShort, printValue),
        TagInfo(0x0207, "FirmwareVersion", N_("Firmware Version"),
                N_("Software firmware version"),
                olympusIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0208, "PictureInfo", N_("Picture Info"),
                N_("ASCII format data such as [PictureInfo]"),
                olympusIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0209, "CameraID", N_("Camera ID"),
                N_("Camera ID data"),
                olympusIfdId, makerTags, undefined, printValue),

        /* TODO: Epson Camera tags. See ExifTool database.

        TagInfo(0x020b, "ImageWidth", N_("Image Width"),
                N_("Image width"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x020c, "ImageHeight", N_("Image Height"),
                N_("Image height"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x020d, "Software", N_("Software"),
                N_("Software"),
                olympusIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0280, "PreviewImage", N_("Preview Image"),
                N_("Preview image"),
                olympusIfdId, makerTags, unsignedByte, printValue),
        */

        TagInfo(0x0300, "PreCaptureFrames", N_("Pre Capture Frames"),
                N_("Pre-capture frames"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0301, "0x0301", "0x0301",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0302, "OneTouchWB", N_("One Touch WB"),
                N_("One touch white balance"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOneTouchWb)),
        TagInfo(0x0303, "0x0303", "0x0303",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0304, "0x0304", "0x0304",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0404, "SerialNumber", N_("Serial Number"),
                N_("Serial number"),
                olympusIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0e00, "PrintIM", N_("Print IM"),
                N_("PrintIM information"),
                olympusIfdId, makerTags, undefined, printValue),
        TagInfo(0x0f00, "DataDump1", N_("Data Dump 1"),
                N_("Various camera settings 1"),
                olympusIfdId, makerTags, undefined, printValue),
        TagInfo(0x0f01, "DataDump2", N_("Data Dump 2"),
                N_("Various camera settings 2"),
                olympusIfdId, makerTags, undefined, printValue),
        TagInfo(0x1000, "ShutterSpeed", N_("Shutter Speed"),
                N_("Shutter speed value"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1001, "ISOSpeed", N_("ISO Speed"),
                N_("ISO speed value"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1002, "ApertureValue", N_("Aperture Value"),
                N_("Aperture value"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1003, "Brightness", N_("Brightness"),
                N_("Brightness value"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1004, "FlashMode", N_("Flash Mode"),
                N_("Flash mode"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOffOn)),
        TagInfo(0x1005, "FlashDevice", N_("Flash Device"),
                N_("Flash device"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusFlashDevice)),
        TagInfo(0x1006, "Bracket", N_("Bracket"),
                N_("Exposure compensation value"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1007, "SensorTemperature", N_("Sensor Temperature"),
                N_("Sensor temperature"),
                olympusIfdId, makerTags, signedShort, printValue),
        TagInfo(0x1008, "LensTemperature", N_("Lens Temperature"),
                N_("Lens temperature"),
                olympusIfdId, makerTags, signedShort, printValue),
        TagInfo(0x1009, "0x1009", "0x1009",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x100a, "0x100a", "0x100a",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x100b, "FocusMode", N_("Focus Mode"),
                N_("Focus mode"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusFocusMode)),
        TagInfo(0x100c, "FocusDistance", N_("Focus Distance"),
                N_("Manual focus distance"),
                olympusIfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x100d, "Zoom", N_("Zoom"),
                N_("Zoom step count"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x100e, "MacroFocus", N_("Macro Focus"),
                N_("Macro focus step count"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x100f, "SharpnessFactor", N_("Sharpness Factor"),
                N_("Sharpness factor"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusSharpness)),
        TagInfo(0x1010, "FlashChargeLevel", N_("Flash Charge Level"),
                N_("Flash charge level"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1011, "ColorMatrix", N_("Color Matrix"),
                N_("Color matrix"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1012, "BlackLevel", N_("BlackLevel"),
                N_("Black level"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1013, "0x1013", "0x1013",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1014, "0x1014", "0x1014",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1015, "WhiteBalance", N_("White Balance"),
                N_("White balance mode"),
                olympusIfdId, makerTags, unsignedShort, print0x1015),
        TagInfo(0x1016, "0x1016", "0x1016",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1017, "RedBalance", N_("Red Balance"),
                N_("Red balance"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1018, "BlueBalance", N_("Blue Balance"),
                N_("Blue balance"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1019, "0x1019", "0x1019",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x101a, "SerialNumber2", N_("Serial Number 2"),
                N_("Serial number 2"),
                olympusIfdId, makerTags, asciiString, printValue),
        TagInfo(0x101b, "0x101b", "0x101b",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x101c, "0x101c", "0x101c",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x101d, "0x101d", "0x101d",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x101e, "0x101e", "0x101e",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x101f, "0x101f", "0x101f",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1020, "0x1020", "0x1020",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1021, "0x1021", "0x1021",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1022, "0x1022", "0x1022",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1023, "FlashBias", N_("Flash Bias"),
                N_("Flash exposure compensation"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1024, "0x1024", "0x1024",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1025, "0x1025", "0x1025",
                N_("Unknown"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1026, "ExternalFlashBounce", N_("External Flash Bounce"),
                N_("External flash bounce"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOffOn)),
        TagInfo(0x1027, "ExternalFlashZoom", N_("External Flash Zoom"),
                N_("External flash zoom"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1028, "ExternalFlashMode", N_("External Flash Mode"),
                N_("External flash mode"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1029, "Contrast", N_("Contrast"),
                N_("Contrast setting"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusContrast)),
        TagInfo(0x102a, "SharpnessFactor", N_("Sharpness Factor"),
                N_("Sharpness factor"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x102b, "ColorControl", N_("Color Control"),
                N_("Color control"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x102c, "ValidBits", N_("ValidBits"),
                N_("Valid bits"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x102d, "CoringFilter", N_("CoringFilter"),
                N_("Coring filter"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x102e, "ImageWidth", N_("Image Width"),
                N_("Image width"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x102f, "ImageHeight", N_("Image Height"),
                N_("Image height"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1030, "0x1030", "0x1030",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1031, "0x1031", "0x1031",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1032, "0x1032", "0x1032",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1033, "0x1033", "0x1033",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1034, "CompressionRatio", N_("Compression Ratio"),
                N_("Compression ratio"),
                olympusIfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x1035, "Thumbnail", N_("Thumbnail"),
                N_("Preview image embedded"),
                olympusIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(olympusOffOn)),
        TagInfo(0x1036, "ThumbnailOffset", N_("Thumbnail Offset"),
                N_("Offset of the preview image"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1037, "ThumbnailLength", N_("Thumbnail Length"),
                N_("Size of the preview image"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1039, "CCDScanMode", N_("CCD Scan Mode"),
                N_("CCD scan mode"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusCCDScanMode)),
        TagInfo(0x103a, "NoiseReduction", N_("Noise Reduction"),
                N_("Noise reduction"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOffOn)),
        TagInfo(0x103b, "InfinityLensStep", N_("Infinity Lens Step"),
                N_("Infinity lens step"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x103c, "NearLensStep", N_("Near Lens Step"),
                N_("Near lens step"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x2010, "Equipment", N_("Equipment Info"),
                N_("Camera equipment information"),
                minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x2020, "CameraSettings", N_("Camera Settings"),
                N_("Camera Settings information"),
                minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x2030, "RawDevelopment", N_("Raw Development"),
                N_("Raw development information"),
                minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x2040, "ImageProcessing", N_("Image Processing"),
                N_("Image processing information"),
                minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x2050, "FocusInfo", N_("Focus Info"),
                N_("Focus information"),
                minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x3000, "RawInfo", N_("Raw Info"),
                N_("Raw information"),
                minoltaIfdId, makerTags, undefined, printValue),

        // End of list marker
        TagInfo(0xffff, "(UnknownOlympusMakerNoteTag)", "(UnknownOlympusMakerNoteTag)",
                N_("Unknown OlympusMakerNote tag"),
                olympusIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* OlympusMakerNote::tagList()
    {
        return tagInfo_;
    }

    //! ExposureMode, tag 0x0200
    extern const TagDetails olympusExposureMode[] = {
        { 1, N_("Manual")                    },
        { 2, N_("Program")                   },
        { 3, N_("Aperture-priority AE")      },
        { 4, N_("Shutter speed priority AE") },
        { 5, N_("Program-shift")             }
    };

    //! MeteringMode
    extern const TagDetails olympusMeteringMode[] = {
        {    2, N_("Center-weighted average") },
        {    3, N_("Spot")                    },
        {    5, N_("ESP")                     },
        {  261, N_("Pattern+AF")              },
        {  515, N_("Spot+Highlight control")  },
        { 1027, N_("Spot+Shadow control")     }
    };

    //! MacroMode
    extern const TagDetails olympusMacroMode[] = { 
        { 0, N_("Off")         },
        { 1, N_("On")          },
        { 2, N_("Super Macro") }
    };

    //! FocusMode
    extern const TagDetails olympusCsFocusMode[] = {
        { 0, N_("Single AF")              },
        { 1, N_("Sequential shooting AF") },
        { 2, N_("Continuous AF")          },
        { 3, N_("Multi AF")               },
        { 10, N_("MF")                    }
    };

    //! FocusProcess
    extern const TagDetails olympusFocusProcess[] = {
        { 0, N_("AF Not Used") },
        { 1, N_("AF Used")     }
    };

    //! AFSearch
    extern const TagDetails olympusAFSearch[] = {
        { 0, N_("Not Ready") },
        { 1, N_("Ready")     }
    };

    //! FlashMode
    extern const TagDetailsBitmask olympusFlashMode[] = {
        { 0x0000, N_("Off")         },
        { 0x0001, N_("On")          },
        { 0x0002, N_("Fill-in")     },
        { 0x0004, N_("Red-eye")     },
        { 0x0008, N_("Slow-sync")   },
        { 0x0010, N_("Forced On")   },
        { 0x0020, N_("2nd Curtain") }
    };

    //! OlympusCs WhiteBalance
    extern const TagDetails olympusWhiteBalance[] = {
        {   0, N_("Auto")                              },
        {  16, N_("7500K (Fine Weather with Shade)")   },
        {  17, N_("6000K (Cloudy)")                    },
        {  18, N_("5300K (Fine Weather)")              },
        {  20, N_("3000K (Tungsten light)")            },
        {  21, N_("3600K (Tungsten light-like)")       },
        {  33, N_("6600K (Daylight fluorescent)")      },
        {  34, N_("4500K (Neutral white fluorescent)") },
        {  35, N_("4000K (Cool white fluorescent)")    },
        {  48, N_("3600K (Tungsten light-like)")       },
        { 256, N_("Custom WB 1")                       },
        { 257, N_("Custom WB 2")                       },
        { 258, N_("Custom WB 3")                       },
        { 259, N_("Custom WB 4")                       },
        { 512, N_("Custom WB 5400K")                   },
        { 513, N_("Custom WB 2900K")                   },
        { 514, N_("Custom WB 8000K")                   }
    };

    //! ModifiedSaturation
    extern const TagDetails olympusModifiedSaturation[] = {
        { 0, N_("Off")                 },
        { 1, N_("CM1 (Red Enhance)")   },
        { 2, N_("CM2 (Green Enhance)") },
        { 3, N_("CM3 (Blue Enhance)")  },
        { 4, N_("CM4 (Skin Tones)")    }
    };

    //! ColorSpace
    extern const TagDetails olympusColorSpace[] = {
        { 0, N_("sRGB")          },
        { 1, N_("Adobe RGB")     },
        { 2, N_("Pro Photo RGB") }
    };

    //! SceneMode
    extern const TagDetails olympusSceneMode[] = {
        {  0, N_("Standard")                    },
        {  6, N_("Auto")                        },
        {  7, N_("Sport")                       },
        {  8, N_("Portrait")                    },
        {  9, N_("Landscape+Portrait")          },
        { 10, N_("Landscape")                   },
        { 11, N_("Night Scene")                 },
        { 12, N_("Self Portrait")               },
        { 13, N_("Panorama")                    },
        { 14, N_("2 in 1")                      },
        { 15, N_("Movie")                       },
        { 16, N_("Landscape+Portrait")          },
        { 17, N_("Night+Portrait")              },
        { 18, N_("Indoor")                      },
        { 19, N_("Fireworks")                   },
        { 20, N_("Sunset")                      },
        { 22, N_("Macro")                       },
        { 23, N_("Super Macro")                 },
        { 24, N_("Food")                        },
        { 25, N_("Documents")                   },
        { 26, N_("Museum")                      },
        { 27, N_("Shoot & Select")              },
        { 28, N_("Beach & Snow")                },
        { 29, N_("Self Protrait+Timer")         },
        { 30, N_("Candle")                      },
        { 31, N_("Available Light")             },
        { 32, N_("Behind Glass")                },
        { 33, N_("My Mode")                     },
        { 34, N_("Pet")                         },
        { 35, N_("Underwater Wide1")            },
        { 36, N_("Underwater Macro")            },
        { 37, N_("Shoot & Select1")             },
        { 38, N_("Shoot & Select2")             },
        { 39, N_("High Key")                    },
        { 40, N_("Digital Image Stabilization") },
        { 41, N_("Auction")                     },
        { 42, N_("Beach")                       },
        { 43, N_("Snow")                        },
        { 44, N_("Underwater Wide2")            },
        { 45, N_("Low Key")                     },
        { 46, N_("Children")                    },
        { 47, N_("Vivid")                       },
        { 48, N_("Nature Macro")                },
        { 49, N_("Underwater Snapshot")         },
        { 50, N_("Shooting Guide")              }
    };

    //! NoiseReduction
    extern const TagDetailsBitmask olympusNoiseReduction[] = {
        { 0x0001, N_("Noise Reduction")          },
        { 0x0002, N_("Noise Filter")             },
        { 0x0004, N_("Noise Filter (ISO Boost)") }
    };

    //! PictureMode
    extern const TagDetails olympusPictureMode[] = {
        {   1, N_("Vivid")    },
        {   2, N_("Natural")  },
        {   3, N_("Muted")    },
        {   4, N_("Portrait") },
        { 256, N_("Monotone") },
        { 512, N_("Sepia")    }
    };

    //! PictureModeBWFilter
    extern const TagDetails olympusPictureModeBWFilter[] = {
        { 0, N_("n/a")     },
        { 1, N_("Neutral") },
        { 2, N_("Yellow")  },
        { 3, N_("Orange")  },
        { 4, N_("Red")     },
        { 5, N_("Green")   }
    };

    //! PictureModeTone
    extern const TagDetails olympusPictureModeTone[] = {
        { 0, N_("n/a")     },
        { 1, N_("Neutral") },
        { 2, N_("Sepia")   },
        { 3, N_("Blue")    },
        { 4, N_("Purple")  },
        { 5, N_("Green")   }
    };

    //! OlympusCs Quality
    extern const TagDetails olympusCsQuality[] = {
        { 1, N_("SQ")  },
        { 2, N_("HQ")  },
        { 3, N_("SHQ") },
        { 4, N_("RAW") }
    };

    const TagInfo OlympusMakerNote::tagInfoCs_[] = {
        TagInfo(0x0000, "CameraSettingsVersion", N_("Camera Settings Version"), N_("Camera settings version"), olympusCsIfdId, makerTags, undefined, printExifVersion),
        TagInfo(0x0100, "PreviewImageValid", N_("PreviewImage Valid"), N_("Preview image valid"), olympusCsIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(olympusNoYes)),
        TagInfo(0x0101, "PreviewImageStart", N_("PreviewImage Start"), N_("Preview image start"), olympusCsIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0102, "PreviewImageLength", N_("PreviewImage Length"), N_("Preview image length"), olympusCsIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0200, "ExposureMode", N_("Exposure Mode"), N_("Exposure mode"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusExposureMode)),
        TagInfo(0x0201, "AELock", N_("AE Lock"), N_("Auto exposure lock"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOffOn)),
        TagInfo(0x0202, "MeteringMode", N_("Metering Mode"), N_("Metering mode"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusMeteringMode)),
        TagInfo(0x0300, "MacroMode", N_("Macro Mode"), N_("Macro mode"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusMacroMode)),
        TagInfo(0x0301, "FocusMode", N_("Focus Mode"), N_("Focus mode"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusCsFocusMode)),
        TagInfo(0x0302, "FocusProcess", N_("Focus Process"), N_("Focus process"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusFocusProcess)),
        TagInfo(0x0303, "AFSearch", N_("AF Search"), N_("AF search"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusAFSearch)),
        TagInfo(0x0304, "AFAreas", N_("AF Areas"), N_("AF areas"), olympusCsIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0400, "FlashMode", N_("Flash Mode"), N_("Flash mode"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG_BITMASK(olympusFlashMode)),
        TagInfo(0x0401, "FlashExposureComp", N_("Flash Exposure Compensation"), N_("Flash exposure compensation"), olympusCsIfdId, makerTags, signedRational, printValue),
        TagInfo(0x0500, "WhiteBalance", N_("White Balance 2"), N_("White balance 2"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusWhiteBalance)),
        TagInfo(0x0501, "WhiteBalanceTemperature", N_("White Balance Temperature"), N_("White balance temperature"), olympusCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0502, "WhiteBalanceBracket", N_("White Balance Bracket"), N_("White balance bracket"), olympusCsIfdId, makerTags, signedShort, printValue),
        TagInfo(0x0503, "CustomSaturation", N_("Custom Saturation"), N_("Custom saturation"), olympusCsIfdId, makerTags, signedShort, printValue),
        TagInfo(0x0504, "ModifiedSaturation", N_("Modified Saturation"), N_("Modified saturation"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusModifiedSaturation)),
        TagInfo(0x0505, "ContrastSetting", N_("Contrast Setting"), N_("Contrast setting"), olympusCsIfdId, makerTags, signedShort, printValue),
        TagInfo(0x0506, "SharpnessSetting", N_("Sharpness Setting"), N_("Sharpness setting"), olympusCsIfdId, makerTags, signedShort, printValue),
        TagInfo(0x0507, "ColorSpace", N_("Color Space"), N_("Color space"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusColorSpace)),
        TagInfo(0x0509, "SceneMode", N_("Scene Mode"), N_("Scene mode"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusSceneMode)),
        TagInfo(0x050a, "NoiseReduction", N_("Noise Reduction"), N_("Noise reduction"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG_BITMASK(olympusNoiseReduction)),
        TagInfo(0x050b, "DistortionCorrection", N_("Distortion Correction"), N_("Distortion correction"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOffOn)),
        TagInfo(0x050c, "ShadingCompensation", N_("Shading Compensation"), N_("Shading compensation"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOffOn)),
        TagInfo(0x050d, "CompressionFactor", N_("Compression Factor"), N_("Compression factor"), olympusCsIfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x050f, "Gradation", N_("Gradation"), N_("Gradation"), olympusCsIfdId, makerTags, signedShort, print0x050f),
        TagInfo(0x0520, "PictureMode", N_("Picture Mode"), N_("Picture mode"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusPictureMode)),
        TagInfo(0x0521, "PictureModeSaturation", N_("Picture Mode Saturation"), N_("Picture mode saturation"), olympusCsIfdId, makerTags, signedShort, printValue),
        TagInfo(0x0522, "PictureModeHue", N_("Picture Mode Hue"), N_("Picture mode hue"), olympusCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0523, "PictureModeContrast", N_("Picture Mode Contrast"), N_("Picture mode contrast"), olympusCsIfdId, makerTags, signedShort, printValue),
        TagInfo(0x0524, "PictureModeSharpness", N_("Picture Mode Sharpness"), N_("Picture mode sharpness"), olympusCsIfdId, makerTags, signedShort, printValue),
        TagInfo(0x0525, "PictureModeBWFilter", N_("Picture Mode BW Filter"), N_("Picture mode BW filter"), olympusCsIfdId, makerTags, signedShort, EXV_PRINT_TAG(olympusPictureModeBWFilter)),
        TagInfo(0x0526, "PictureModeTone", N_("Picture Mode Tone"), N_("Picture mode tone"), olympusCsIfdId, makerTags, signedShort, EXV_PRINT_TAG(olympusPictureModeTone)),
        TagInfo(0x0527, "NoiseFilter", N_("Noise Filter"), N_("Noise filter"), olympusCsIfdId, makerTags, signedShort, print0x0527),
        TagInfo(0x0600, "DriveMode", N_("Drive Mode"), N_("Drive mode"), olympusCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0601, "PanoramaMode", N_("Panorama Mode"), N_("Panorama mode"), olympusCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0603, "Quality", N_("Image Quality 2"), N_("Image quality 2"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusCsQuality)),
        TagInfo(0x0900, "ManometerPressure", N_("Manometer Pressure"), N_("Manometer pressure"), olympusCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0901, "ManometerReading", N_("Manometer Reading"), N_("Manometer reading"), olympusCsIfdId, makerTags, signedLong, printValue),
        TagInfo(0x0902, "ExtendedWBDetect", N_("Extended WB Detect"), N_("Extended WB detect"), olympusCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOffOn)),
        // End of list marker
        TagInfo(0xffff, "(UnknownOlympusCsTag)", "(UnknownOlympusCsTag)", N_("Unknown OlympusCs tag"), olympusCsIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* OlympusMakerNote::tagListCs()
    {
        return tagInfoCs_;
    }

    const TagInfo OlympusMakerNote::tagInfoEq_[] = {
        TagInfo(0x0000, "EquipmentVersion", N_("Equipment Version"), N_("Equipment version"), olympusEqIfdId, makerTags, undefined, printExifVersion),

        // Todo: Add Olympus equipment tags

        // End of list marker
        TagInfo(0xffff, "(UnknownOlympusEqTag)", "(UnknownOlympusEqTag)", N_("Unknown OlympusEq tag"), olympusEqIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* OlympusMakerNote::tagListEq()
    {
        return tagInfoEq_;
    }

    const TagInfo OlympusMakerNote::tagInfoRd_[] = {
        TagInfo(0x0000, "RawDevVersion", N_("Raw Development Version"), N_("Raw development version"), olympusRdIfdId, makerTags, undefined, printExifVersion),

        // Todo: Add Olympus raw development tags

        // End of list marker
        TagInfo(0xffff, "(UnknownOlympusRdTag)", "(UnknownOlympusRdTag)", N_("Unknown OlympusRd tag"), olympusRdIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* OlympusMakerNote::tagListRd()
    {
        return tagInfoRd_;
    }

    const TagInfo OlympusMakerNote::tagInfoRd2_[] = {
        TagInfo(0x0000, "RawDev2Version", N_("Raw Development 2 Version"), N_("Raw development 2 version"), olympusRd2IfdId, makerTags, undefined, printExifVersion),

        // Todo: Add Olympus raw development 2 tags

        // End of list marker
        TagInfo(0xffff, "(UnknownOlympusRd2Tag)", "(UnknownOlympusRd2Tag)", N_("Unknown OlympusRd2 tag"), olympusRd2IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* OlympusMakerNote::tagListRd2()
    {
        return tagInfoRd2_;
    }

    const TagInfo OlympusMakerNote::tagInfoIp_[] = {
        TagInfo(0x0000, "ImageProcessingVersion", N_("Image Processing Version"), N_("Image processing version"), olympusIpIfdId, makerTags, undefined, printExifVersion),

        // Todo: Add Olympus image processing tags

        // End of list marker
        TagInfo(0xffff, "(UnknownOlympusIpTag)", "(UnknownOlympusIpTag)", N_("Unknown OlympusIp tag"), olympusIpIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* OlympusMakerNote::tagListIp()
    {
        return tagInfoIp_;
    }

    const TagInfo OlympusMakerNote::tagInfoFi_[] = {
        TagInfo(0x0000, "FocusInfoVersion", N_("Focus Info Version"), N_("Focus info version"), olympusFiIfdId, makerTags, undefined, printExifVersion),

        // Todo: Add Olympus focus info tags

        // End of list marker
        TagInfo(0xffff, "(UnknownOlympusFiTag)", "(UnknownOlympusFiTag)", N_("Unknown OlympusFi tag"), olympusFiIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* OlympusMakerNote::tagListFi()
    {
        return tagInfoFi_;
    }

    // Gradation
    std::ostream& OlympusMakerNote::print0x050f(std::ostream& os, const Value& value, const ExifData*)
    {
        if (   !(value.count() == 3 || value.count() == 4)
            || value.typeId() != signedShort) {
            return os << value;
        }

        if      (value.toLong(0) == -1 && value.toLong(1) == -1 && value.toLong(2) == 1) os << _("Low Key");
        else if (value.toLong(0) ==  0 && value.toLong(1) == -1 && value.toLong(2) == 1) os << _("Normal");
        else if (value.toLong(0) ==  1 && value.toLong(1) == -1 && value.toLong(2) == 1) os << _("High Key");
        else os << value.toLong(0) << " " << value.toLong(1) << " " << value.toLong(2);

        if (value.count() == 4) {
            switch (value.toLong(3)) {
                case 0: os << ", " << _("User-Selected"); break;
                case 1: os << ", " << _("Auto-Override"); break;
                default: os << value.toLong(3); break;
            }
        }    
        return os;
    }

    // NoiseFilter
    std::ostream& OlympusMakerNote::print0x0527(std::ostream& os, const Value& value, const ExifData*)
    {
        if (   value.count() != 3
               || value.typeId() != signedShort
               || value.toLong(1) != -2
               || value.toLong(2) != 1) {
            return os << value;
        }

        switch (value.toLong(0)) {
            case -2: os << _("Off"); break;
            case -1: os << _("Low"); break;
            case  0: os << _("Standard"); break;
            case  1: os << _("High"); break;
            default: os << value.toLong(0); break;
        }

        return os;
    }

    std::ostream& OlympusMakerNote::print0x0200(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 3 || value.typeId() != unsignedLong) {
            return os << value;
        }
        long l0 = value.toLong(0);
        switch (l0) {
        case 0: os << _("Normal"); break;
        case 2: os << _("Fast"); break;
        case 3: os << _("Panorama"); break;
        default: os << "(" << l0 << ")"; break;
        }
        if (l0 != 0) {
            os << ", ";
            long l1 = value.toLong(1);
            os << _("Sequence number") << " " << l1;
        }
        if (l0 != 0 && l0 != 2) {
            os << ", ";
            long l2 = value.toLong(2);
            switch (l2) {
            case 1: os << _("Left to right"); break;
            case 2: os << _("Right to left"); break;
            case 3: os << _("Bottom to top"); break;
            case 4: os << _("Top to bottom"); break;
            default: os << "(" << l2 << ")"; break;
            }
        }
        return os;
    } // OlympusMakerNote::print0x0200

    std::ostream& OlympusMakerNote::print0x0204(std::ostream& os, const Value& value, const ExifData*)
    {
        if (   value.count() == 0
            || value.toRational().second == 0) {
            return os << "(" << value << ")";
        }
        float f = value.toFloat();
        if (f == 0.0 || f == 1.0) return os << _("None");
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1) << f << "x";
        os.copyfmt(oss);
        return os;
    } // OlympusMakerNote::print0x0204

    std::ostream& OlympusMakerNote::print0x1015(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 2 || value.typeId() != unsignedShort) {
            return os << value;
        }
        short l0 = (short)value.toLong(0);
        if (l0 != 1) {
            os << _("Auto");
        }
        else {
            short l1 = (short)value.toLong(1);
            if (l1 != 1) {
                switch (l0) {
                case 0: os << _("Auto"); break;
                default: os << _("Auto") << " (" << l0 << ")"; break;
                }
            }
            else if (l1 != 2) {
                switch (l0) {
                case 2: os << _("3000 Kelvin"); break;
                case 3: os << _("3700 Kelvin"); break;
                case 4: os << _("4000 Kelvin"); break;
                case 5: os << _("4500 Kelvin"); break;
                case 6: os << _("5500 Kelvin"); break;
                case 7: os << _("6500 Kelvin"); break;
                case 8: os << _("7500 Kelvin"); break;
                default: os << value; break;
                }
            }
            else if (l1 != 3) {
                switch (l0) {
                case 0: os << _("One-touch"); break;
                default: os << value; break;
                }
            }
            else {
                return os << value;
            }
        }
        return os;
    } // OlympusMakerNote::print0x1015

}                                       // namespace Exiv2
