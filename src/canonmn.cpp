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
  File   : canonmn.cpp
  Version: $Rev$
  Authors: Andreas Huggel (ahu) <ahuggel@gmx.net>
           David Cannings (dc) <david@edeca.net>
  Credits: EXIF MakerNote of Canon by David Burren <http://www.burren.cx/david/canon.html>
           Canon makernote tags by Phil Harvey <http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/Canon.html>
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "canonmn.hpp"
#include "makernote.hpp"
#include "value.hpp"
#include "ifd.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! @cond IGNORE
    CanonMakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("Canon", "*", createCanonMakerNote);

        MakerNoteFactory::registerMakerNote(
            canonIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(
            canonCsIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(
            canonSiIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(
            canonPaIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(
            canonCfIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(
            canonPiIfdId, MakerNote::AutoPtr(new CanonMakerNote));

        ExifTags::registerMakerTagInfo(canonIfdId, tagInfo_);
        ExifTags::registerMakerTagInfo(canonCsIfdId, tagInfoCs_);
        ExifTags::registerMakerTagInfo(canonSiIfdId, tagInfoSi_);
        ExifTags::registerMakerTagInfo(canonPaIfdId, tagInfoPa_);
        ExifTags::registerMakerTagInfo(canonCfIfdId, tagInfoCf_);
        ExifTags::registerMakerTagInfo(canonPiIfdId, tagInfoPi_);
    }
    //! @endcond

    //! ModelId, tag 0x0010
    extern const TagDetails canonModelId[] = {
        { 0x1010000, N_("PowerShot A30") },
        { 0x1040000, N_("PowerShot S300 / Digital IXUS 300 / IXY Digital 300") },
        { 0x1060000, N_("PowerShot A20") },
        { 0x1080000, N_("PowerShot A10") },
        { 0x1090000, N_("PowerShot S110 / Digital IXUS v / IXY Digital 200") },
        { 0x1100000, N_("PowerShot G2") },
        { 0x1110000, N_("PowerShot S40") },
        { 0x1120000, N_("PowerShot S30") },
        { 0x1130000, N_("PowerShot A40") },
        { 0x1140000, N_("EOS D30") },
        { 0x1150000, N_("PowerShot A100") },
        { 0x1160000, N_("PowerShot S200 / Digital IXUS v2 / IXY Digital 200a") },
        { 0x1170000, N_("PowerShot A200") },
        { 0x1180000, N_("PowerShot S330 / Digital IXUS 330 / IXY Digital 300a") },
        { 0x1190000, N_("PowerShot G3") },
        { 0x1210000, N_("PowerShot S45") },
        { 0x1230000, N_("PowerShot SD100 / Digital IXUS II / IXY Digital 30") },
        { 0x1240000, N_("PowerShot S230 / Digital IXUS v3 / IXY Digital 320") },
        { 0x1250000, N_("PowerShot A70") },
        { 0x1260000, N_("PowerShot A60") },
        { 0x1270000, N_("PowerShot S400 / Digital IXUS 400 / IXY Digital 400") },
        { 0x1290000, N_("PowerShot G5") },
        { 0x1300000, N_("PowerShot A300") },
        { 0x1310000, N_("PowerShot S50") },
        { 0x1340000, N_("PowerShot A80") },
        { 0x1350000, N_("PowerShot SD10 / Digital IXUS i / IXY Digital L") },
        { 0x1360000, N_("PowerShot S1 IS") },
        { 0x1370000, N_("PowerShot Pro1") },
        { 0x1380000, N_("PowerShot S70") },
        { 0x1390000, N_("PowerShot S60") },
        { 0x1400000, N_("PowerShot G6") },
        { 0x1410000, N_("PowerShot S500 / Digital IXUS 500 / IXY Digital 500") },
        { 0x1420000, N_("PowerShot A75") },
        { 0x1440000, N_("PowerShot SD110 / Digital IXUS IIs / IXY Digital 30a") },
        { 0x1450000, N_("PowerShot A400") },
        { 0x1470000, N_("PowerShot A310") },
        { 0x1490000, N_("PowerShot A85") },
        { 0x1520000, N_("PowerShot S410 / Digital IXUS 430 / IXY Digital 450") },
        { 0x1530000, N_("PowerShot A95") },
        { 0x1540000, N_("PowerShot SD300 / Digital IXUS 40 / IXY Digital 50") },
        { 0x1550000, N_("PowerShot SD200 / Digital IXUS 30 / IXY Digital 40") },
        { 0x1560000, N_("PowerShot A520") },
        { 0x1570000, N_("PowerShot A510") },
        { 0x1590000, N_("PowerShot SD20 / Digital IXUS i5 / IXY Digital L2") },
        { 0x1640000, N_("PowerShot S2 IS") },
        { 0x1650000, N_("PowerShot SD430 / IXUS Wireless / IXY Wireless") },
        { 0x1660000, N_("PowerShot SD500 / Digital IXUS 700 / IXY Digital 600") },
        { 0x1668000, N_("EOS D60") },
        { 0x1700000, N_("PowerShot SD30 / Digital IXUS i zoom / IXY Digital L3") },
        { 0x1740000, N_("PowerShot A430") },
        { 0x1750000, N_("PowerShot A410") },
        { 0x1760000, N_("PowerShot S80") },
        { 0x1780000, N_("PowerShot A620") },
        { 0x1790000, N_("PowerShot A610") },
        { 0x1800000, N_("PowerShot SD630 / Digital IXUS 65 / IXY Digital 80") },
        { 0x1810000, N_("PowerShot SD450 / Digital IXUS 55 / IXY Digital 60") },
        { 0x1820000, N_("PowerShot TX1") },
        { 0x1870000, N_("PowerShot SD400 / Digital IXUS 50 / IXY Digital 55") },
        { 0x1880000, N_("PowerShot A420") },
        { 0x1890000, N_("PowerShot SD900 / Digital IXUS 900 Ti / IXY Digital 1000") },
        { 0x1900000, N_("PowerShot SD550 / Digital IXUS 750 / IXY Digital 700") },
        { 0x1920000, N_("PowerShot A700") },
        { 0x1940000, N_("PowerShot SD700 IS / Digital IXUS 800 IS / IXY Digital 800 IS") },
        { 0x1950000, N_("PowerShot S3 IS") },
        { 0x1960000, N_("PowerShot A540") },
        { 0x1970000, N_("PowerShot SD600 / Digital IXUS 60 / IXY Digital 70") },
        { 0x1980000, N_("PowerShot G7") },
        { 0x1990000, N_("PowerShot A530") },
        { 0x2000000, N_("PowerShot SD800 IS / Digital IXUS 850 IS / IXY Digital 900 IS") },
        { 0x2010000, N_("PowerShot SD40 / Digital IXUS i7 / IXY Digital L4") },
        { 0x2020000, N_("PowerShot A710 IS") },
        { 0x2030000, N_("PowerShot A640") },
        { 0x2040000, N_("PowerShot A630") },
        { 0x2090000, N_("PowerShot S5 IS") },
        { 0x2100000, N_("PowerShot A460") },
        { 0x2120000, N_("PowerShot SD850 IS / Digital IXUS 950 IS") },
        { 0x2130000, N_("PowerShot A570 IS") },
        { 0x2140000, N_("PowerShot A560") },
        { 0x2150000, N_("PowerShot SD750 / Digital IXUS 75 / IXY Digital 90") },
        { 0x2160000, N_("PowerShot SD1000 / Digital IXUS 70 / IXY Digital 10") },
        { 0x2180000, N_("PowerShot A550") },
        { 0x2190000, N_("PowerShot A450") },
        { 0x3010000, N_("PowerShot Pro90 IS") },
        { 0x4040000, N_("PowerShot G1") },
        { 0x6040000, N_("PowerShot S100 / Digital IXUS / IXY Digital") },
        { 0x4007d675, N_("HV10") },
        { 0x4007d777, N_("iVIS DC50") },
        { 0x4007d778, N_("iVIS HV20") },
        { 0x80000001, N_("EOS-1D") },
        { 0x80000167, N_("EOS-1DS") },
        { 0x80000168, N_("EOS 10D") },
        { 0x80000169, N_("EOS-1D Mark III") },
        { 0x80000170, N_("EOS Digital Rebel / 300D / Kiss Digital") },
        { 0x80000174, N_("EOS-1D Mark II") },
        { 0x80000175, N_("EOS 20D") },
        { 0x80000188, N_("EOS-1Ds Mark II") },
        { 0x80000189, N_("EOS Digital Rebel XT / 350D / Kiss Digital N") },
        { 0x80000213, N_("EOS 5D") },
        { 0x80000232, N_("EOS-1D Mark II N") },
        { 0x80000234, N_("EOS 30D") },
        { 0x80000236, N_("EOS Digital Rebel XTi / 400D / Kiss Digital X") },
    };

    // Canon MakerNote Tag Info
    const TagInfo CanonMakerNote::tagInfo_[] = {
        TagInfo(0x0000, "0x0000", "0x0000", N_("Unknown"), canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0001, "CameraSettings", N_("Camera Settings"), N_("Various camera settings"), canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0002, "0x0002", "0x0002", N_("Unknown"), canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0003, "0x0003", "0x0003", N_("Unknown"), canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0004, "ShotInfo", N_("Shot Info"), N_("Shot information"), canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0005, "Panorama", N_("Panorama"), N_("Panorama"), canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0006, "ImageType", N_("Image Type"), N_("Image type"), canonIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "FirmwareVersion", N_("Firmware Version"), N_("Firmware version"), canonIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0008, "ImageNumber", N_("Image Number"), N_("Image number"), canonIfdId, makerTags, unsignedLong, print0x0008),
        TagInfo(0x0009, "OwnerName", N_("Owner Name"), N_("Owner Name"), canonIfdId, makerTags, asciiString, printValue),
        TagInfo(0x000c, "SerialNumber", N_("Serial Number"), N_("Camera serial number"), canonIfdId, makerTags, unsignedLong, print0x000c),
        TagInfo(0x000d, "0x000d", "0x000d", N_("Unknown"), canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000f, "CustomFunctions", N_("Custom Functions"), N_("Custom Functions"), canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0010, "ModelID", N_("ModelID"), N_("Model ID"), canonIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(canonModelId)),
        TagInfo(0x0012, "PictureInfo", N_("Picture Info"), N_("Picture info"), canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00a9, "WhiteBalanceTable", N_("White Balance Table"), N_("White balance table"), canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00b5, "0x00b5", "0x00b5", N_("Unknown"), canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00c0, "0x00c0", "0x00c0", N_("Unknown"), canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00c1, "0x00c1", "0x00c1", N_("Unknown"), canonIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonMakerNoteTag)", "(UnknownCanonMakerNoteTag)", N_("Unknown CanonMakerNote tag"), canonIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* CanonMakerNote::tagList()
    {
        return tagInfo_;
    }

    //! Macro, tag 0x0001
    extern const TagDetails canonCsMacro[] = {
        { 1, N_("On")  },
        { 2, N_("Off") }
    };

    //! Quality, tag 0x0003
    extern const TagDetails canonCsQuality[] = {
        { 1, N_("Economy")   },
        { 2, N_("Normal")    },
        { 3, N_("Fine")      },
        { 4, N_("RAW")       },
        { 5, N_("Superfine") }
    };

    //! FlashMode, tag 0x0004
    extern const TagDetails canonCsFlashMode[] = {
        {  0, N_("Off")            },
        {  1, N_("Auto")           },
        {  2, N_("On")             },
        {  3, N_("Red-eye")        },
        {  4, N_("Slow sync")      },
        {  5, N_("Auto + red-eye") },
        {  6, N_("On + red-eye")   },
        { 16, N_("External")       }
    };

    //! DriveMode, tag 0x0005
    extern const TagDetails canonCsDriveMode[] = {
        { 0, N_("Single / timer")             },
        { 1, N_("Continuous")                 },
        { 2, N_("Movie")                      },
        { 3, N_("Continuous, speed priority") },
        { 4, N_("Continuous, low")            },
        { 5, N_("Continuous, high")           }
    };

    //! FocusMode, tag 0x0007
    extern const TagDetails canonCsFocusMode[] = {
        {  0, N_("One shot AF")  },
        {  1, N_("AI servo AF")  },
        {  2, N_("AI focus AF")  },
        {  3, N_("Manual focus") },
        {  4, N_("Single")       },
        {  5, N_("Continuous")   },
        {  6, N_("Manual focus") },
        { 16, N_("Pan focus")    }
    };

    //! ImageSize, tag 0x000a
    extern const TagDetails canonCsImageSize[] = {
        { 0, N_("Large")    },
        { 1, N_("Medium")   },
        { 2, N_("Small")    },
        { 5, N_("Medium 1") },
        { 6, N_("Medium 2") },
        { 7, N_("Medium 3") }
    };

    //! EasyMode, tag 0x000b
    extern const TagDetails canonCsEasyMode[] = {
        {  0, N_("Full auto")        },
        {  1, N_("Manual")           },
        {  2, N_("Landscape")        },
        {  3, N_("Fast shutter")     },
        {  4, N_("Slow shutter")     },
        {  5, N_("Night Scene")      },
        {  6, N_("Gray scale")       },
        {  7, N_("Sepia")            },
        {  8, N_("Portrait")         },
        {  9, N_("Sports")           },
        { 10, N_("Macro / close-up") },
        { 11, N_("Black & white")    },
        { 12, N_("Pan focus")        },
        { 13, N_("Vivid")            },
        { 14, N_("Neutral")          },
        { 15, N_("Flash off")        },
        { 16, N_("Long shutter")     },
        { 17, N_("Super macro")      },
        { 18, N_("Foliage")          },
        { 19, N_("Indoor")           },
        { 20, N_("Fireworks")        },
        { 21, N_("Beach")            },
        { 22, N_("Underwater")       },
        { 23, N_("Snow")             },
        { 24, N_("Kids & pets")      },
        { 25, N_("Night SnapShot")   },
        { 26, N_("Digital macro")    },
        { 27, N_("My Colors")        },
        { 28, N_("Still image")      }
    };

    //! DigitalZoom, tag 0x000c
    extern const TagDetails canonCsDigitalZoom[] = {
        { 0, N_("None")  },
        { 1, "2x"        },
        { 2, "4x"        },
        { 3, N_("Other") }
    };

    //! Contrast, Saturation Sharpness, tags 0x000d, 0x000e, 0x000f
    extern const TagDetails canonCsLnh[] = {
        { 0xffff, N_("Low")    },
        { 0x0000, N_("Normal") },
        { 0x0001, N_("High")   }
    };

    //! ISOSpeeds, tag 0x0010
    extern const TagDetails canonCsISOSpeed[] = {
        {     0, N_("n/a")       },
        {    14, N_("Auto High") },
        {    15, N_("Auto")      },
        {    16,   "50"      },
        {    17,  "100"      },
        {    18,  "200"      },
        {    19,  "400"      },
        {    20,  "800"      },
        { 16464,   "80"      },
        { 16484,  "100"      },
        { 16584,  "200"      },
        { 16784,  "400"      },
        { 17184,  "800"      },
        { 17984, "1600"      },
        { 19584, "3200"      }
    };

    //! MeteringMode, tag 0x0011
    extern const TagDetails canonCsMeteringMode[] = {
        { 0, N_("Default")         },
        { 1, N_("Spot")            },
        { 2, N_("Average")         },
        { 3, N_("Evaluative")      },
        { 4, N_("Partial")         },
        { 5, N_("Center weighted") }
    };

    //! FocusType, tag 0x0012
    extern const TagDetails canonCsFocusType[] = {
        {  0, N_("Manual")       },
        {  1, N_("Auto")         },
        {  2, N_("Not known")    },
        {  3, N_("Macro")        },
        {  4, N_("Very close")   },
        {  5, N_("Close")        },
        {  6, N_("Middle range") },
        {  7, N_("Far range")    },
        {  8, N_("Pan focus")    },
        {  9, N_("Super macro")  },
        { 10, N_("Infinity")     }
    };

    //! AFPoint, tag 0x0013
    extern const TagDetails canonCsAfPoint[] = {
        { 0x2005, N_("Manual AF point selection") },
        { 0x3000, N_("None (MF)")                 },
        { 0x3001, N_("Auto-selected")             },
        { 0x3002, N_("Right")                     },
        { 0x3003, N_("Center")                    },
        { 0x3004, N_("Left")                      },
        { 0x4001, N_("Auto AF point selection")   }
    };

    //! ExposureProgram, tag 0x0014
    extern const TagDetails canonCsExposureProgram[] = {
        { 0, N_("Easy shooting (Auto)")   },
        { 1, N_("Program (P)")            },
        { 2, N_("Shutter priority (Tv)")  },
        { 3, N_("Aperture priority (Av)") },
        { 4, N_("Manual (M)")             },
        { 5, N_("A-DEP")                  },
        { 6, N_("M-DEP")                  }
    };

    //! FlashActivity, tag 0x001c
    extern const TagDetails canonCsFlashActivity[] = {
        { 0, N_("Did not fire") },
        { 1, N_("Fired")        }
    };

    //! FlashDetails, tag 0x001d
    extern const TagDetailsBitmask canonCsFlashDetails[] = {
        { 0x4000, N_("External flash")        },
        { 0x2000, N_("Internal flash")        },
        { 0x0001, N_("Manual")                },
        { 0x0002, N_("TTL")                   },
        { 0x0004, N_("A-TTL")                 },
        { 0x0008, N_("E-TTL")                 },
        { 0x0010, N_("FP sync enabled")       },
        { 0x0080, N_("2nd-curtain sync used") },
        { 0x0800, N_("FP sync used")          }
    };

    //! FocusContinuous, tag 0x0020
    extern const TagDetails canonCsFocusContinuous[] = {
        { 0, N_("Single")     },
        { 1, N_("Continuous") }
    };

    //! AESetting, tag 0x0021
    extern const TagDetails canonCsAESetting[] = {
        { 0, N_("Normal AE")                       },
        { 1, N_("Exposure compensation")           },
        { 2, N_("AE lock")                         },
        { 3, N_("AE lock + exposure compensation") },
        { 4, N_("No AE")                           }
    };

    //! ImageStabilization, tag 0x0022
    extern const TagDetails canonCsImageStabilization[] = {
        { 0, N_("Off")           },
        { 1, N_("On")            },
        { 2, N_("On, shot only") }
    };

    //! PhotoEffect, tag 0x0028
    extern const TagDetails canonCsPhotoEffect[] = {
        { 0,   N_("Off")           },
        { 1,   N_("Vivid")         },
        { 2,   N_("Neutral")       },
        { 3,   N_("Smooth")        },
        { 4,   N_("Sepia")         },
        { 5,   N_("B&W")           },
        { 6,   N_("Custom")        },
        { 100, N_("My color data") }
    };

    // Canon Camera Settings Tag Info
    const TagInfo CanonMakerNote::tagInfoCs_[] = {
        TagInfo(0x0001, "Macro", N_("Macro"), N_("Macro mode"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsMacro)),
        TagInfo(0x0002, "Selftimer", N_("Selftimer"), N_("Self timer"), canonCsIfdId, makerTags, unsignedShort, printCs0x0002),
        TagInfo(0x0003, "Quality", N_("Quality"), N_("Quality"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsQuality)),
        TagInfo(0x0004, "FlashMode", N_("Flash Mode"), N_("Flash mode setting"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsFlashMode)),
        TagInfo(0x0005, "DriveMode", N_("Drive Mode"), N_("Drive mode setting"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsDriveMode)),
        TagInfo(0x0006, "0x0006", "0x0006", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "FocusMode", N_("Focus Mode"), N_("Focus mode setting"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsFocusMode)),
        TagInfo(0x0008, "0x0008", "0x0008", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0009, "0x0009", "0x0009", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000a, "ImageSize", N_("Image Size"), N_("Image size"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsImageSize)),
        TagInfo(0x000b, "EasyMode", N_("Easy Mode"), N_("Easy shooting mode"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsEasyMode)),
        TagInfo(0x000c, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsDigitalZoom)),
        TagInfo(0x000d, "Contrast", N_("Contrast"), N_("Contrast setting"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsLnh)),
        TagInfo(0x000e, "Saturation", N_("Saturation"), N_("Saturation setting"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsLnh)),
        TagInfo(0x000f, "Sharpness", N_("Sharpness"), N_("Sharpness setting"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsLnh)),
        TagInfo(0x0010, "ISOSpeed", N_("ISO Speed Mode"), N_("ISO speed setting"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsISOSpeed)),
        TagInfo(0x0011, "MeteringMode", N_("Metering Mode"), N_("Metering mode setting"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsMeteringMode)),
        TagInfo(0x0012, "FocusType", N_("Focus Type"), N_("Focus type setting"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsFocusType)),
        TagInfo(0x0013, "AFPoint", N_("AF Point"), N_("AF point selected"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsAfPoint)),
        TagInfo(0x0014, "ExposureProgram", N_("Exposure Program"), N_("Exposure mode setting"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsExposureProgram)),
        TagInfo(0x0015, "0x0015", "0x0015", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0016, "0x0016", "0x0016", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0017, "Lens", N_("Lens"), N_("'long' and 'short' focal length of lens (in 'focal units') and 'focal units' per mm"), canonCsIfdId, makerTags, unsignedShort, printCsLens),
        TagInfo(0x0018, "0x0018", "0x0018", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0019, "0x0019", "0x0019", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001a, "0x001a", "0x001a", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001b, "0x001b", "0x001b", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001c, "FlashActivity", N_("Flash Activity"), N_("Flash activity"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsFlashActivity)),
        TagInfo(0x001d, "FlashDetails", N_("Flash Details"), N_("Flash details"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG_BITMASK(canonCsFlashDetails)),
        TagInfo(0x001e, "0x001e", "0x001e", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001f, "0x001f", "0x001f", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0020, "FocusContinuous", N_("Focus Continuous"), N_("Focus continuous setting"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsFocusContinuous)),
        TagInfo(0x0021, "AESetting", N_("AESetting"), N_("AE setting"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsAESetting)),
        TagInfo(0x0022, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsImageStabilization)),
        TagInfo(0x0023, "DisplayAperture", N_("Display Aperture"), N_("Display aperture"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0024, "ZoomSourceWidth", N_("Zoom Source Width"), N_("Zoom source width"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0025, "ZoomTargetWidth", N_("Zoom Target Width"), N_("Zoom target width"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0026, "0x0026", "0x0026", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0027, "0x0027", "0x0027", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0028, "PhotoEffect", N_("Photo Effect"), N_("Photo effect"), canonCsIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCsPhotoEffect)),
        TagInfo(0x0029, "0x0029", "0x0029", N_("Unknown"), canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x002a, "ColorTone", N_("Color Tone"), N_("Color tone"), canonCsIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCsTag)", "(UnknownCanonCsTag)", N_("Unknown Canon Camera Settings 1 tag"), canonCsIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* CanonMakerNote::tagListCs()
    {
        return tagInfoCs_;
    }

    //! WhiteBalance, tag 0x0007
    extern const TagDetails canonSiWhiteBalance[] = {
        {  0, N_("Auto")                        },
        {  1, N_("Sunny")                       },
        {  2, N_("Cloudy")                      },
        {  3, N_("Tungsten")                    },
        {  4, N_("Fluorescent")                 },
        {  5, N_("Flash")                       },
        {  6, N_("Custom")                      },
        {  7, N_("Black & White")               },
        {  8, N_("Shade")                       },
        {  9, N_("Manual Temperature (Kelvin)") },
        { 10, N_("PC Set 1")                    },
        { 11, N_("PC Set 2")                    },
        { 12, N_("PC Set 3")                    },
        { 14, N_("Daylight Fluorescent")        },
        { 15, N_("Custom 1")                    },
        { 16, N_("Custom 2")                    },
        { 17, N_("Underwater")                  }
    };

    //! AFPointUsed, tag 0x000e
    extern const TagDetailsBitmask canonSiAFPointUsed[] = {
        { 0x0004, N_("left")   },
        { 0x0002, N_("center") },
        { 0x0001, N_("right")  }
    };

    //! FlashBias, tag 0x000f
    extern const TagDetails canonSiFlashBias[] = {
        { 0xffc0, "-2 EV"    },
        { 0xffcc, "-1.67 EV" },
        { 0xffd0, "-1.50 EV" },
        { 0xffd4, "-1.33 EV" },
        { 0xffe0, "-1 EV"    },
        { 0xffec, "-0.67 EV" },
        { 0xfff0, "-0.50 EV" },
        { 0xfff4, "-0.33 EV" },
        { 0x0000, "0 EV"     },
        { 0x000c, "0.33 EV"  },
        { 0x0010, "0.50 EV"  },
        { 0x0014, "0.67 EV"  },
        { 0x0020, "1 EV"     },
        { 0x002c, "1.33 EV"  },
        { 0x0030, "1.50 EV"  },
        { 0x0034, "1.67 EV"  },
        { 0x0040, "2 EV"     }
    };

    // Canon Shot Info Tag
    const TagInfo CanonMakerNote::tagInfoSi_[] = {
        TagInfo(0x0001, "0x0001", "0x0001", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0002, "ISOSpeed", N_("ISO Speed Used"), N_("ISO speed used"), canonSiIfdId, makerTags, unsignedShort, printSi0x0002),
        TagInfo(0x0003, "0x0003", "0x0003", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0004, "TargetAperture", N_("Target Aperture"), N_("Target Aperture"), canonSiIfdId, makerTags, unsignedShort, printSi0x0015),
        TagInfo(0x0005, "TargetShutterSpeed", N_("Target Shutter Speed"), N_("Target shutter speed"), canonSiIfdId, makerTags, unsignedShort, printSi0x0016),
        TagInfo(0x0006, "0x0006", "0x0006", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "WhiteBalance", N_("White Balance"), N_("White balance setting"), canonSiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonSiWhiteBalance)),
        TagInfo(0x0008, "0x0008", "0x0008", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0009, "Sequence", N_("Sequence"), N_("Sequence number (if in a continuous burst)"), canonSiIfdId, makerTags, unsignedShort, printSi0x0009),
        TagInfo(0x000a, "0x000a", "0x000a", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000b, "0x000b", "0x000b", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000c, "0x000c", "0x000c", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000d, "0x000d", "0x000d", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000e, "AFPointUsed", N_("AF Point Used"), N_("AF point used"), canonSiIfdId, makerTags, unsignedShort, printSi0x000e),
        TagInfo(0x000f, "FlashBias", N_("Flash Bias"), N_("Flash bias"), canonSiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonSiFlashBias)),
        TagInfo(0x0010, "0x0010", "0x0010", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0011, "0x0011", "0x0011", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0012, "0x0012", "0x0012", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0013, "SubjectDistance", N_("Subject Distance"), N_("Subject distance (units are not clear)"), canonSiIfdId, makerTags, unsignedShort, printSi0x0013),
        TagInfo(0x0014, "0x0014", "0x0014", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0015, "ApertureValue", N_("Aperture Value"), N_("Aperture"), canonSiIfdId, makerTags, unsignedShort, printSi0x0015),
        TagInfo(0x0016, "ShutterSpeedValue", N_("Shutter Speed Value"), N_("Shutter speed"), canonSiIfdId, makerTags, unsignedShort, printSi0x0016),
        TagInfo(0x0017, "0x0017", "0x0017", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0018, "0x0018", "0x0018", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0019, "0x0019", "0x0019", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001a, "0x001a", "0x001a", N_("Unknown"), canonSiIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonSiTag)", "(UnknownCanonSiTag)", N_("Unknown Canon Camera Settings 2 tag"), canonSiIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* CanonMakerNote::tagListSi()
    {
        return tagInfoSi_;
    }

    //! PanoramaDirection, tag 0x0005
    extern const TagDetails canonPaDirection[] = {
        { 0, N_("Left to right")          },
        { 1, N_("Right to left")          },
        { 2, N_("Bottom to top")          },
        { 3, N_("Top to bottom")          },
        { 4, N_("2x2 matrix (Clockwise)") }
    };

    // Canon Panorama Info
    const TagInfo CanonMakerNote::tagInfoPa_[] = {
        TagInfo(0x0002, "PanoramaFrame", N_("Panorama Frame"), N_("Panorama frame number"), canonPaIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0005, "PanoramaDirection", N_("Panorama Direction"), N_("Panorama direction"), canonPaIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonPaDirection)),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCs2Tag)", "(UnknownCanonCs2Tag)", N_("Unknown Canon Panorama tag"), canonPaIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* CanonMakerNote::tagListPa()
    {
        return tagInfoPa_;
    }

    // Canon Custom Function Tag Info
    const TagInfo CanonMakerNote::tagInfoCf_[] = {
        TagInfo(0x0001, "NoiseReduction", N_("Noise Reduction"), N_("Long exposure noise reduction"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0002, "ShutterAeLock", N_("Shutter Ae Lock"), N_("Shutter/AE lock buttons"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0003, "MirrorLockup", N_("Mirror Lockup"), N_("Mirror lockup"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0004, "ExposureLevelIncrements", N_("Exposure Level Increments"), N_("Tv/Av and exposure level"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0005, "AFAssist", N_("AF Assist"), N_("AF assist light"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0006, "FlashSyncSpeedAv", N_("Flash Sync Speed Av"), N_("Shutter speed in Av mode"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "AEBSequence", N_("AEB Sequence"), N_("AEB sequence/auto cancellation"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0008, "ShutterCurtainSync", N_("Shutter Curtain Sync"), N_("Shutter curtain sync"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0009, "LensAFStopButton", N_("Lens AF Stop Button"), N_("Lens AF stop button Fn. Switch"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000a, "FillFlashAutoReduction", N_("Fill Flash Auto Reduction"), N_("Auto reduction of fill flash"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000b, "MenuButtonReturn", N_("Menu Button Return"), N_("Menu button return position"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000c, "SetButtonFunction", N_("Set Button Function"), N_("SET button func. when shooting"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000d, "SensorCleaning", N_("Sensor Cleaning"), N_("Sensor cleaning"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000e, "SuperimposedDisplay", N_("Superimposed Display"), N_("Superimposed display"), canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000f, "ShutterReleaseNoCFCard", N_("Shutter Release No CF Card"), N_("Shutter Release W/O CF Card"), canonCfIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCfTag)", "(UnknownCanonCfTag)", N_("Unknown Canon Custom Function tag"), canonCfIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* CanonMakerNote::tagListCf()
    {
        return tagInfoCf_;
    }

    //! AFPointsUsed, tag 0x0016
    extern const TagDetailsBitmask canonPiAFPointsUsed[] = {
        { 0x01, N_("right")     },
        { 0x02, N_("mid-right") },
        { 0x04, N_("bottom")    },
        { 0x08, N_("center")    },
        { 0x10, N_("top")       },
        { 0x20, N_("mid-left")  },
        { 0x40, N_("left")      }
    };

    //! AFPointsUsed20D, tag 0x001a
    extern const TagDetailsBitmask canonPiAFPointsUsed20D[] = {
        { 0x001, N_("top")         },
        { 0x002, N_("upper-left")  },
        { 0x004, N_("upper-right") },
        { 0x008, N_("left")        },
        { 0x010, N_("center")      },
        { 0x020, N_("right")       },
        { 0x040, N_("lower-left")  },
        { 0x080, N_("lower-right") },
        { 0x100, N_("bottom")      }
    };

    // Canon Picture Info Tag
    const TagInfo CanonMakerNote::tagInfoPi_[] = {
        TagInfo(0x0002, "ImageWidth", N_("Image Width"), N_("Image width"), canonPiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0003, "ImageHeight", N_("Image Height"), N_("Image height"), canonPiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0004, "ImageWidthAsShot", N_("Image Width As Shot"), N_("Image width (as shot)"), canonPiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0005, "ImageHeightAsShot", N_("Image Height As Shot"), N_("Image height (as shot)"), canonPiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0016, "AFPointsUsed", N_("AF Points Used"), N_("AF points used"), canonPiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG_BITMASK(canonPiAFPointsUsed)),
        TagInfo(0x001a, "AFPointsUsed20D", N_("AF Points Used 20D"), N_("AF points used (20D)"), canonPiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG_BITMASK(canonPiAFPointsUsed20D)),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonPiTag)", "(UnknownCanonPiTag)", N_("Unknown Canon Picture Info tag"), canonPiIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* CanonMakerNote::tagListPi()
    {
        return tagInfoPi_;
    }

    int CanonMakerNote::read(const byte* buf,
                             long len,
                             long start,
                             ByteOrder byteOrder,
                             long shift)
    {
        int rc = IfdMakerNote::read(buf, len, start, byteOrder, shift);
        if (rc) return rc;

        // Decode camera settings 1 and add settings as additional entries
        Entries::iterator cs = ifd_.findTag(0x0001);
        if (cs != ifd_.end() && cs->type() == unsignedShort) {
            for (uint16_t c = 1; cs->count() > c; ++c) {
                if (c == 23 && cs->count() > 25) {
                    // Pack related lens info into one tag
                    addCsEntry(canonCsIfdId, c, cs->offset() + c*2,
                               cs->data() + c*2, 3);
                    c += 2;
                }
                else {
                    addCsEntry(canonCsIfdId, c, cs->offset() + c*2,
                               cs->data() + c*2, 1);
                }
            }
            // Discard the original entry
            ifd_.erase(cs);
        }

        // Decode camera settings 2 and add settings as additional entries
        cs = ifd_.findTag(0x0004);
        if (cs != ifd_.end() && cs->type() == unsignedShort) {
            for (uint16_t c = 1; cs->count() > c; ++c) {
                addCsEntry(canonSiIfdId, c, cs->offset() + c*2,
                           cs->data() + c*2, 1);
            }
            // Discard the original entry
            ifd_.erase(cs);
        }

        // Decode panorama information and add each as an additional entry
        cs = ifd_.findTag(0x0005);
        if (cs != ifd_.end() && cs->type() == unsignedShort) {
            for (uint16_t c = 1; cs->count() > c; ++c) {
                addCsEntry(canonPaIfdId, c, cs->offset() + c*2,
                           cs->data() + c*2, 1);
            }
            // Discard the original entry
            ifd_.erase(cs);
        }

        // Decode custom functions and add each as an additional entry
        cs = ifd_.findTag(0x000f);
        if (cs != ifd_.end() && cs->type() == unsignedShort) {
            for (uint16_t c = 1; cs->count() > c; ++c) {
                addCsEntry(canonCfIfdId, c, cs->offset() + c*2,
                           cs->data() + c*2, 1);
            }
            // Discard the original entry
            ifd_.erase(cs);
        }

        // Decode picture info and add each as an additional entry
        cs = ifd_.findTag(0x0012);
        if (cs != ifd_.end() && cs->type() == unsignedShort) {
            for (uint16_t c = 1; cs->count() > c; ++c) {
                addCsEntry(canonPiIfdId, c, cs->offset() + c*2,
                           cs->data() + c*2, 1);
            }
            // Discard the original entry
            ifd_.erase(cs);
        }

        // Copy remaining ifd entries
        entries_.insert(entries_.begin(), ifd_.begin(), ifd_.end());

        // Set idx
        int idx = 0;
        Entries::iterator e = entries_.end();
        for (Entries::iterator i = entries_.begin(); i != e; ++i) {
            i->setIdx(++idx);
        }

        return 0;
    }

    void CanonMakerNote::addCsEntry(IfdId ifdId,
                                    uint16_t tag,
                                    long offset,
                                    const byte* data,
                                    int count)
    {
        Entry e(false);
        e.setIfdId(ifdId);
        e.setTag(tag);
        e.setOffset(offset);
        e.setValue(unsignedShort, count, data, 2*count);
        add(e);
    }

    void CanonMakerNote::add(const Entry& entry)
    {
        assert(alloc_ == entry.alloc());
        assert(   entry.ifdId() == canonIfdId
               || entry.ifdId() == canonCsIfdId
               || entry.ifdId() == canonSiIfdId
               || entry.ifdId() == canonPaIfdId
               || entry.ifdId() == canonCfIfdId
               || entry.ifdId() == canonPiIfdId);
        // allow duplicates
        entries_.push_back(entry);
    }

    long CanonMakerNote::copy(byte* buf, ByteOrder byteOrder, long offset)
    {
        if (byteOrder_ == invalidByteOrder) byteOrder_ = byteOrder;

        assert(ifd_.alloc());
        ifd_.clear();

        // Add all standard Canon entries to the IFD
        Entries::const_iterator end = entries_.end();
        for (Entries::const_iterator i = entries_.begin(); i != end; ++i) {
            if (i->ifdId() == canonIfdId) {
                ifd_.add(*i);
            }
        }
        // Collect camera settings entries and add the original Canon tag
        Entry cs;
        if (assemble(cs, canonCsIfdId, 0x0001, byteOrder_)) {
            ifd_.erase(0x0001);
            ifd_.add(cs);
        }
        // Collect shot info entries and add the original Canon tag
        Entry si;
        if (assemble(si, canonSiIfdId, 0x0004, byteOrder_)) {
            ifd_.erase(0x0004);
            ifd_.add(si);
        }
        // Collect panorama entries and add the original Canon tag
        Entry pa;
        if (assemble(pa, canonPaIfdId, 0x0005, byteOrder_)) {
            ifd_.erase(0x0005);
            ifd_.add(pa);
        }
        // Collect custom function entries and add the original Canon tag
        Entry cf;
        if (assemble(cf, canonCfIfdId, 0x000f, byteOrder_)) {
            ifd_.erase(0x000f);
            ifd_.add(cf);
        }
        // Collect picture info entries and add the original Canon tag
        Entry pi;
        if (assemble(pi, canonPiIfdId, 0x0012, byteOrder_)) {
            ifd_.erase(0x0012);
            ifd_.add(pi);
        }

        return IfdMakerNote::copy(buf, byteOrder_, offset);
    } // CanonMakerNote::copy

    void CanonMakerNote::updateBase(byte* pNewBase)
    {
        byte* pBase = ifd_.updateBase(pNewBase);
        if (absShift_ && !alloc_) {
            Entries::iterator end = entries_.end();
            for (Entries::iterator pos = entries_.begin(); pos != end; ++pos) {
                pos->updateBase(pBase, pNewBase);
            }
        }
    } // CanonMakerNote::updateBase

    long CanonMakerNote::size() const
    {
        Ifd ifd(canonIfdId, 0, alloc_); // offset doesn't matter

        // Add all standard Canon entries to the IFD
        Entries::const_iterator end = entries_.end();
        for (Entries::const_iterator i = entries_.begin(); i != end; ++i) {
            if (i->ifdId() == canonIfdId) {
                ifd.add(*i);
            }
        }
        // Collect camera settings entries and add the original Canon tag
        Entry cs(alloc_);
        if (assemble(cs, canonCsIfdId, 0x0001, littleEndian)) {
            ifd.erase(0x0001);
            ifd.add(cs);
        }
        // Collect shot info entries and add the original Canon tag
        Entry si(alloc_);
        if (assemble(si, canonSiIfdId, 0x0004, littleEndian)) {
            ifd.erase(0x0004);
            ifd.add(si);
        }
        // Collect panorama entries and add the original Canon tag
        Entry pa(alloc_);
        if (assemble(pa, canonPaIfdId, 0x0005, littleEndian)) {
            ifd.erase(0x0005);
            ifd.add(pa);
        }
        // Collect custom function entries and add the original Canon tag
        Entry cf(alloc_);
        if (assemble(cf, canonCfIfdId, 0x000f, littleEndian)) {
            ifd.erase(0x000f);
            ifd.add(cf);
        }
        // Collect picture info entries and add the original Canon tag
        Entry pi(alloc_);
        if (assemble(pi, canonPiIfdId, 0x0012, littleEndian)) {
            ifd.erase(0x0012);
            ifd.add(pi);
        }

        return headerSize() + ifd.size() + ifd.dataSize();
    } // CanonMakerNote::size

    long CanonMakerNote::assemble(Entry& e,
                                  IfdId ifdId,
                                  uint16_t tag,
                                  ByteOrder byteOrder) const
    {
        DataBuf buf(1024);
        std::memset(buf.pData_, 0x0, 1024);
        uint16_t len = 0;
        Entries::const_iterator end = entries_.end();
        for (Entries::const_iterator i = entries_.begin(); i != end; ++i) {
            if (i->ifdId() == ifdId) {
                uint16_t pos = i->tag() * 2;
                uint16_t size = pos + static_cast<uint16_t>(i->size());
                assert(size <= 1024);
                std::memcpy(buf.pData_ + pos, i->data(), i->size());
                if (len < size) len = size;
            }
        }
        if (len > 0) {
            // Number of shorts in the buffer (rounded up)
            uint16_t s = (len+1) / 2;
            us2Data(buf.pData_, s*2, byteOrder);

            e.setIfdId(canonIfdId);
            e.setIdx(0); // don't care
            e.setTag(tag);
            e.setOffset(0); // will be calculated when the IFD is written
            e.setValue(unsignedShort, s, buf.pData_, s*2);
        }
        return len;
    } // CanonMakerNote::assemble

    Entries::const_iterator CanonMakerNote::findIdx(int idx) const
    {
        return std::find_if(entries_.begin(), entries_.end(),
                            FindEntryByIdx(idx));
    }

    CanonMakerNote::CanonMakerNote(bool alloc)
        : IfdMakerNote(canonIfdId, alloc)
    {
    }

    CanonMakerNote::CanonMakerNote(const CanonMakerNote& rhs)
        : IfdMakerNote(rhs)
    {
        entries_ = rhs.entries_;
    }

    CanonMakerNote::AutoPtr CanonMakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    CanonMakerNote* CanonMakerNote::create_(bool alloc) const
    {
        return new CanonMakerNote(alloc);
    }

    CanonMakerNote::AutoPtr CanonMakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    CanonMakerNote* CanonMakerNote::clone_() const
    {
        return new CanonMakerNote(*this);
    }

    std::ostream& CanonMakerNote::print0x0008(std::ostream& os,
                                              const Value& value)
    {
        std::string n = value.toString();
        if (n.length() < 4) return os << "(" << n << ")";
        return os << n.substr(0, n.length() - 4) << "-"
                  << n.substr(n.length() - 4);
    }

    std::ostream& CanonMakerNote::print0x000c(std::ostream& os,
                                              const Value& value)
    {
        std::istringstream is(value.toString());
        uint32_t l;
        is >> l;
        return os << std::setw(4) << std::setfill('0') << std::hex
                  << ((l & 0xffff0000) >> 16)
                  << std::setw(5) << std::setfill('0') << std::dec
                  << (l & 0x0000ffff);
    }

    std::ostream& CanonMakerNote::printCs0x0002(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        if (l == 0) {
            os << "Off";
        }
        else {
            os << l / 10.0 << " s";
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCsLens(std::ostream& os,
                                                const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        if (value.count() < 3) return os << value;

        float fu = value.toFloat(2);
        if (fu == 0.0) return os << value;
        float len1 = value.toLong(0) / fu;
        float len2 = value.toLong(1) / fu;
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1);
        if (len1 == len2) {
            os << len1 << " mm";
        } else {
            os << len2 << " - " << len1 << " mm";
        }
        os.copyfmt(oss);
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0002(std::ostream& os,
                                                 const Value& value)
    {
        // Ported from Exiftool by Will Stokes
        return os << exp(canonEv(value.toLong()) * log(2.0)) * 100.0 / 32.0;
    }

    std::ostream& CanonMakerNote::printSi0x0009(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        os << l << "";
        // Todo: determine unit
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x000e(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        long num = (l & 0xf000) >> 12;
        os << num << " focus points; ";
        long used = l & 0x0fff;
        if (used == 0) {
            os << "none";
        }
        else {
            EXV_PRINT_TAG_BITMASK(canonSiAFPointUsed)(os, value);
        }
        os << " used";
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0013(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        if (l == 0xffff) {
            os << "Infinite";
        }
        else {
            os << l << "";
        }
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0015(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;

        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::setprecision(2)
           << "F" << fnumber(canonEv(value.toLong()));
        os.copyfmt(oss);

        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0016(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;

        URational ur = exposureTime(canonEv(value.toLong()));
        os << ur.first;
        if (ur.second > 1) {
            os << "/" << ur.second;
        }
        return os << " s";
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createCanonMakerNote(      bool      alloc,
                                            const byte*     /*buf*/,
                                                  long      /*len*/,
                                                  ByteOrder /*byteOrder*/,
                                                  long      /*offset*/)
    {
        return MakerNote::AutoPtr(new CanonMakerNote(alloc));
    }

    float canonEv(long val)
    {
        // temporarily remove sign
        int sign = 1;
        if (val < 0) {
            sign = -1;
            val = -val;
        }
        // remove fraction
        float frac = static_cast<float>(val & 0x1f);
        val -= long(frac);
        // convert 1/3 (0x0c) and 2/3 (0x14) codes
        if (frac == 0x0c) {
            frac = 32.0f / 3;
        }
        else if (frac == 0x14) {
            frac = 64.0f / 3;
        }
        return sign * (val + frac) / 32.0f;
    }

}                                       // namespace Exiv2
