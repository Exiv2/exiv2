// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
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
  File:      canonmn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Feb-04, ahu: created
             07-Mar-04, ahu: isolated as a separate component
  Credits:   Canon MakerNote implemented according to the specification
             "EXIF MakerNote of Canon" <http://www.burren.cx/david/canon.html>
             by David Burren
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
            canonCs1IfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(
            canonCs2IfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(
            canonCfIfdId, MakerNote::AutoPtr(new CanonMakerNote));

        ExifTags::registerMakerTagInfo(canonIfdId, tagInfo_);
        ExifTags::registerMakerTagInfo(canonCs1IfdId, tagInfoCs1_);
        ExifTags::registerMakerTagInfo(canonCs2IfdId, tagInfoCs2_);
        ExifTags::registerMakerTagInfo(canonCfIfdId, tagInfoCf_);
    }
    //! @endcond

    // Canon MakerNote Tag Info
    const TagInfo CanonMakerNote::tagInfo_[] = {
        TagInfo(0x0000, "0x0000", "0x0000", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0001, "CameraSettings1", "CameraSettings1", "Various camera settings (1)", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0002, "0x0002", "0x0002", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0003, "0x0003", "0x0003", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0004, "CameraSettings2", "CameraSettings2", "Various camera settings (2)", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0006, "ImageType", "ImageType", "Image type", canonIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "FirmwareVersion", "Firmware Version", "Firmware version", canonIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0008, "ImageNumber", "ImageNumber", "Image number", canonIfdId, makerTags, unsignedLong, print0x0008),
        TagInfo(0x0009, "OwnerName", "OwnerName", "Owner Name", canonIfdId, makerTags, asciiString, printValue),
        TagInfo(0x000c, "SerialNumber", "SerialNumber", "Camera serial number", canonIfdId, makerTags, unsignedLong, print0x000c),
        TagInfo(0x000d, "0x000d", "0x000d", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000f, "CustomFunctions", "CustomFunctions", "Custom Functions", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0012, "PictureInfo", "PictureInfo", "Picture info", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00a9, "WhiteBalanceTable", "WhiteBalanceTable", "White balance table", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00b5, "0x00b5", "0x00b5", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00c0, "0x00c0", "0x00c0", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00c1, "0x00c1", "0x00c1", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonMakerNoteTag)", "(UnknownCanonMakerNoteTag)", "Unknown CanonMakerNote tag", canonIfdId, makerTags, invalidTypeId, printValue)
    };

    //! Macro, tag 0x0001
    extern const TagDetails canonCs1Macro[] = {
        { 1, "On"  },
        { 2, "Off" }
    };

    //! Quality, tag 0x0003
    extern const TagDetails canonCs1Quality[] = {
        { 1, "Economy"   },
        { 2, "Normal"    },
        { 3, "Fine"      },
        { 4, "RAW"       },
        { 5, "Superfine" }
    };

    //! FlashMode, tag 0x0004
    extern const TagDetails canonCs1FlashMode[] = {
        {  0, "Off"            },
        {  1, "Auto"           },
        {  2, "On"             },
        {  3, "Red-eye"        },
        {  4, "Slow sync"      },
        {  5, "Auto + red-eye" },
        {  6, "On + red-eye"   },
        { 16, "External"       }
    };

    //! DriveMode, tag 0x0005
    extern const TagDetails canonCs1DriveMode[] = {
        { 0, "Single / timer"             },
        { 1, "Continuous"                 },
        { 2, "Movie"                      },
        { 3, "Continuous, speed priority" },
        { 4, "Continuous, low"            },
        { 5, "Continuous, high"           }
    };

    //! FocusMode, tag 0x0007
    extern const TagDetails canonCs1FocusMode[] = {
        {  0, "One shot AF"  },
        {  1, "AI servo AF"  },
        {  2, "AI focus AF"  },
        {  3, "Manual focus" },
        {  4, "Single"       },
        {  5, "Continuous"   },
        {  6, "Manual focus" },
        { 16, "Pan focus"    }
    };

    //! ImageSize, tag 0x000a
    extern const TagDetails canonCs1ImageSize[] = {
        { 0, "Large"    },
        { 1, "Medium"   },
        { 2, "Small"    },
        { 5, "Medium 1" },
        { 6, "Medium 2" },
        { 7, "Medium 3" }
    };

    //! EasyMode, tag 0x000b
    extern const TagDetails canonCs1EasyMode[] = {
        {  0, "Full auto"        },
        {  1, "Manual"           },
        {  2, "Landscape"        },
        {  3, "Fast shutter"     },
        {  4, "Slow shutter"     },
        {  5, "Night Scene"      },
        {  6, "Gray scale"       },
        {  7, "Sepia"            },
        {  8, "Portrait"         },
        {  9, "Sports"           },
        { 10, "Macro / close-up" },
        { 11, "Black & white"    },
        { 12, "Pan focus"        },
        { 13, "Vivid"            },
        { 14, "Neutral"          },
        { 15, "Flash off"        },
        { 16, "Long shutter"     },
        { 17, "Super macro"      },
        { 18, "Foliage"          },
        { 19, "Indoor"           },
        { 20, "Fireworks"        },
        { 21, "Beach"            },
        { 22, "Underwater"       },
        { 23, "Snow"             },
        { 24, "Kids & pets"      },
        { 25, "Night SnapShot"   },
        { 26, "Digital macro"    },
        { 27, "My Colors"        },
        { 28, "Still image"      }
    };

    //! DigitalZoom, tag 0x000c
    extern const TagDetails canonCs1DigitalZoom[] = {
        { 0, "None"  },
        { 1, "2x"    },
        { 2, "4x"    },
        { 3, "Other" }
    };

    //! Contrast, Saturation Sharpness, tags 0x000d, 0x000e, 0x000f
    extern const TagDetails canonCs1Lnh[] = {
        { 0xffff, "Low"    },
        { 0x0000, "Normal" },
        { 0x0001, "High"   }
    };

    //! ISOSpeeds, tag 0x0010
    extern const TagDetails canonCs1ISOSpeed[] = {
        {  0, "n/a"  },
        { 15, "Auto" },
        { 16, "50"   },
        { 17, "100"  },
        { 18, "200"  },
        { 19, "400"  }
    };

    //! MeteringMode, tag 0x0011
    extern const TagDetails canonCs1MeteringMode[] = {
        { 0, "Default"         },
        { 1, "Spot"            },
        { 2, "Average"         },
        { 3, "Evaluative"      },
        { 4, "Partial"         },
        { 5, "Center weighted" }
    };

    //! FocusType, tag 0x0012
    extern const TagDetails canonCs1FocusType[] = {
        {  0, "Manual"       },
        {  1, "Auto"         },
        {  2, "Not known"    },
        {  3, "Macro"        },
        {  4, "Very close"   },
        {  5, "Close"        },
        {  6, "Middle range" },
        {  7, "Far range"    },
        {  8, "Pan focus"    },
        {  9, "Super macro"  },
        { 10, "Infinity"     }
    };

    //! AFPoint, tag 0x0013
    extern const TagDetails canonCs1AfPoint[] = {
        { 0x2005, "Manual AF point selection" },
        { 0x3000, "None (MF)"                 },
        { 0x3001, "Auto-selected"             },
        { 0x3002, "Right"                     },
        { 0x3003, "Center"                    },
        { 0x3004, "Left"                      },
        { 0x4001, "Auto AF point selection"   }
    };

    //! ExposureProgram, tag 0x0014
    extern const TagDetails canonCs1ExposureProgram[] = {
        { 0, "Easy shooting (Auto)"   },
        { 1, "Program (P)"            },
        { 2, "Shutter priority (Tv)"  },
        { 3, "Aperture priority (Av)" },
        { 4, "Manual (M)"             },
        { 5, "A-DEP"                  },
        { 6, "M-DEP"                  }
    };

    //! FlashActivity, tag 0x001c
    extern const TagDetails canonCs1FlashActivity[] = {
        { 0, "Did not fire" },
        { 1, "Fired"        }
    };

    //! FlashDetails, tag 0x001d
    extern const TagDetailsBitmask canonCs1FlashDetails[] = {
        { 0x4000, "External flash"        },
        { 0x2000, "Internal flash"        },
        { 0x0001, "Manual"                },
        { 0x0002, "TTL"                   },
        { 0x0004, "A-TTL"                 },
        { 0x0008, "E-TTL"                 },
        { 0x0010, "FP sync enabled"       },
        { 0x0080, "2nd-curtain sync used" },
        { 0x0800, "FP sync used"          }
    };

    //! FocusContinuous, tag 0x0020
    extern const TagDetails canonCs1FocusContinuous[] = {
        { 0, "Single"     },
        { 1, "Continuous" }
    };

    // Canon Camera Settings 1 Tag Info
    const TagInfo CanonMakerNote::tagInfoCs1_[] = {
        TagInfo(0x0001, "Macro", "Macro", "Macro mode", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1Macro)),
        TagInfo(0x0002, "Selftimer", "Selftimer", "Self timer", canonCs1IfdId, makerTags, unsignedShort, printCs10x0002),
        TagInfo(0x0003, "Quality", "Quality", "Quality", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1Quality)),
        TagInfo(0x0004, "FlashMode", "Flash Mode", "Flash mode setting", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1FlashMode)),
        TagInfo(0x0005, "DriveMode", "Drive Mode", "Drive mode setting", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1DriveMode)),
        TagInfo(0x0006, "0x0006", "0x0006", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "FocusMode", "Focus Mode", "Focus mode setting", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1FocusMode)),
        TagInfo(0x0008, "0x0008", "0x0008", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0009, "0x0009", "0x0009", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000a, "ImageSize", "Image Size", "Image size", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1ImageSize)),
        TagInfo(0x000b, "EasyMode", "Easy Mode", "Easy shooting mode", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1EasyMode)),
        TagInfo(0x000c, "DigitalZoom", "Digital Zoom", "Digital zoom", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1DigitalZoom)),
        TagInfo(0x000d, "Contrast", "Contrast", "Contrast setting", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1Lnh)),
        TagInfo(0x000e, "Saturation", "Saturation", "Saturation setting", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1Lnh)),
        TagInfo(0x000f, "Sharpness", "Sharpness", "Sharpness setting", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1Lnh)),
        TagInfo(0x0010, "ISOSpeed", "ISO Speed Mode", "ISO speed setting", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1ISOSpeed)),
        TagInfo(0x0011, "MeteringMode", "Metering Mode", "Metering mode setting", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1MeteringMode)),
        TagInfo(0x0012, "FocusType", "Focus Type", "Focus type setting", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1FocusType)),
        TagInfo(0x0013, "AFPoint", "AFPoint", "AF point selected", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1AfPoint)),
        TagInfo(0x0014, "ExposureProgram", "Exposure Program", "Exposure mode setting", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1ExposureProgram)),
        TagInfo(0x0015, "0x0015", "0x0015", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0016, "0x0016", "0x0016", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0017, "Lens", "Lens", "'long' and 'short' focal length of lens (in 'focal units') and 'focal units' per mm", canonCs1IfdId, makerTags, unsignedShort, printCs1Lens),
        TagInfo(0x0018, "0x0018", "0x0018", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0019, "0x0019", "0x0019", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001a, "0x001a", "0x001a", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001b, "0x001b", "0x001b", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001c, "FlashActivity", "FlashActivity", "Flash activity", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1FlashActivity)),
        TagInfo(0x001d, "FlashDetails", "FlashDetails", "Flash details", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG_BITMASK(canonCs1FlashDetails)),
        TagInfo(0x001e, "0x001e", "0x001e", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001f, "0x001f", "0x001f", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0020, "FocusContinuous", "Focus Continuous", "Focus continuous setting", canonCs1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs1FocusContinuous)),
        TagInfo(0x0021, "0x0021", "0x0021", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0022, "0x0022", "0x0022", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0023, "0x0023", "0x0023", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0024, "0x0024", "0x0024", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0025, "0x0025", "0x0025", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0026, "0x0026", "0x0026", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0027, "0x0027", "0x0027", "Unknown", canonCs1IfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCs1Tag)", "(UnknownCanonCs1Tag)", "Unknown Canon Camera Settings 1 tag", canonCs1IfdId, makerTags, invalidTypeId, printValue)
    };

    //! WhiteBalance, tag 0x0007
    extern const TagDetails canonCs2WhiteBalance[] = {
        {  0, "Auto"                        },
        {  1, "Sunny"                       },
        {  2, "Cloudy"                      },
        {  3, "Tungsten"                    },
        {  4, "Fluorescent"                 },
        {  5, "Flash"                       },
        {  6, "Custom"                      },
        {  7, "Black & White"               },
        {  8, "Shade"                       },
        {  9, "Manual Temperature (Kelvin)" },
        { 10, "PC Set 1"                    },
        { 11, "PC Set 2"                    },
        { 12, "PC Set 3"                    },
        { 14, "Daylight Fluorescent"        },
        { 15, "Custom 1"                    },
        { 16, "Custom 2"                    },
        { 17, "Underwater"                  }
    };

    //! AFPointUsed, tag 0x000e
    extern const TagDetailsBitmask canonCs2AFPointUsed[] = {
        { 0x0004, "left"   },
        { 0x0002, "center" },
        { 0x0001, "right"  }
    };

    //! FlashBias, tag 0x000f
    extern const TagDetails canonCs2FlashBias[] = {
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

    // Canon Camera Settings 2 Tag Info
    const TagInfo CanonMakerNote::tagInfoCs2_[] = {
        TagInfo(0x0001, "0x0001", "0x0001", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0002, "ISOSpeed", "ISO Speed Used", "ISO speed used", canonCs2IfdId, makerTags, unsignedShort, printCs20x0002),
        TagInfo(0x0003, "0x0003", "0x0003", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0004, "TargetAperture", "TargetAperture", "Target Aperture", canonCs2IfdId, makerTags, unsignedShort, printCs20x0015),
        TagInfo(0x0005, "TargetShutterSpeed", "TargetShutterSpeed", "Target shutter speed", canonCs2IfdId, makerTags, unsignedShort, printCs20x0016),
        TagInfo(0x0006, "0x0006", "0x0006", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "WhiteBalance", "WhiteBalance", "White balance setting", canonCs2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs2WhiteBalance)),
        TagInfo(0x0008, "0x0008", "0x0008", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0009, "Sequence", "Sequence", "Sequence number (if in a continuous burst)", canonCs2IfdId, makerTags, unsignedShort, printCs20x0009),
        TagInfo(0x000a, "0x000a", "0x000a", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000b, "0x000b", "0x000b", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000c, "0x000c", "0x000c", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000d, "0x000d", "0x000d", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000e, "AFPointUsed", "AFPointUsed", "AF point used", canonCs2IfdId, makerTags, unsignedShort, printCs20x000e),
        TagInfo(0x000f, "FlashBias", "FlashBias", "Flash bias", canonCs2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(canonCs2FlashBias)),
        TagInfo(0x0010, "0x0010", "0x0010", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0011, "0x0011", "0x0011", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0012, "0x0012", "0x0012", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0013, "SubjectDistance", "SubjectDistance", "Subject distance (units are not clear)", canonCs2IfdId, makerTags, unsignedShort, printCs20x0013),
        TagInfo(0x0014, "0x0014", "0x0014", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0015, "ApertureValue", "ApertureValue", "Aperture", canonCs2IfdId, makerTags, unsignedShort, printCs20x0015),
        TagInfo(0x0016, "ShutterSpeedValue", "ShutterSpeedValue", "Shutter speed", canonCs2IfdId, makerTags, unsignedShort, printCs20x0016),
        TagInfo(0x0017, "0x0017", "0x0017", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0018, "0x0018", "0x0018", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0019, "0x0019", "0x0019", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001a, "0x001a", "0x001a", "Unknown", canonCs2IfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCs2Tag)", "(UnknownCanonCs2Tag)", "Unknown Canon Camera Settings 2 tag", canonCs2IfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Custom Function Tag Info
    const TagInfo CanonMakerNote::tagInfoCf_[] = {
        TagInfo(0x0001, "NoiseReduction", "NoiseReduction", "Long exposure noise reduction", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0002, "ShutterAeLock", "ShutterAeLock", "Shutter/AE lock buttons", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0003, "MirrorLockup", "MirrorLockup", "Mirror lockup", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0004, "ExposureLevelIncrements", "ExposureLevelIncrements", "Tv/Av and exposure level", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0005, "AFAssist", "AFAssist", "AF assist light", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0006, "FlashSyncSpeedAv", "FlashSyncSpeedAv", "Shutter speed in Av mode", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "AEBSequence", "AEBSequence", "AEB sequence/auto cancellation", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0008, "ShutterCurtainSync", "ShutterCurtainSync", "Shutter curtain sync", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0009, "LensAFStopButton", "LensAFStopButton", "Lens AF stop button Fn. Switch", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000a, "FillFlashAutoReduction", "FillFlashAutoReduction", "Auto reduction of fill flash", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000b, "MenuButtonReturn", "MenuButtonReturn", "Menu button return position", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000c, "SetButtonFunction", "SetButtonFunction", "SET button func. when shooting", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000d, "SensorCleaning", "SensorCleaning", "Sensor cleaning", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000e, "SuperimposedDisplay", "SuperimposedDisplay", "Superimposed display", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000f, "ShutterReleaseNoCFCard", "ShutterReleaseNoCFCard", "Shutter Release W/O CF Card", canonCfIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCfTag)", "(UnknownCanonCfTag)", "Unknown Canon Custom Function tag", canonCfIfdId, makerTags, invalidTypeId, printValue)
    };

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
                    addCsEntry(canonCs1IfdId, c, cs->offset() + c*2,
                               cs->data() + c*2, 3);
                    c += 2;
                }
                else {
                    addCsEntry(canonCs1IfdId, c, cs->offset() + c*2,
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
                addCsEntry(canonCs2IfdId, c, cs->offset() + c*2,
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
               || entry.ifdId() == canonCs1IfdId
               || entry.ifdId() == canonCs2IfdId
               || entry.ifdId() == canonCfIfdId);
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
        // Collect camera settings 1 entries and add the original Canon tag
        Entry cs1;
        if (assemble(cs1, canonCs1IfdId, 0x0001, byteOrder_)) {
            ifd_.erase(0x0001);
            ifd_.add(cs1);
        }
        // Collect camera settings 2 entries and add the original Canon tag
        Entry cs2;
        if (assemble(cs2, canonCs2IfdId, 0x0004, byteOrder_)) {
            ifd_.erase(0x0004);
            ifd_.add(cs2);
        }
        // Collect custom function entries and add the original Canon tag
        Entry cf;
        if (assemble(cf, canonCfIfdId, 0x000f, byteOrder_)) {
            ifd_.erase(0x000f);
            ifd_.add(cf);
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
        // Collect camera settings 1 entries and add the original Canon tag
        Entry cs1(alloc_);
        if (assemble(cs1, canonCs1IfdId, 0x0001, littleEndian)) {
            ifd.erase(0x0001);
            ifd.add(cs1);
        }
        // Collect camera settings 2 entries and add the original Canon tag
        Entry cs2(alloc_);
        if (assemble(cs2, canonCs2IfdId, 0x0004, littleEndian)) {
            ifd.erase(0x0004);
            ifd.add(cs2);
        }
        // Collect custom function entries and add the original Canon tag
        Entry cf(alloc_);
        if (assemble(cf, canonCfIfdId, 0x000f, littleEndian)) {
            ifd.erase(0x000f);
            ifd.add(cf);
        }

        return headerSize() + ifd.size() + ifd.dataSize();
    } // CanonMakerNote::size

    long CanonMakerNote::assemble(Entry& e,
                                  IfdId ifdId,
                                  uint16_t tag,
                                  ByteOrder byteOrder) const
    {
        DataBuf buf(1024);
        memset(buf.pData_, 0x0, 1024);
        uint16_t len = 0;
        Entries::const_iterator end = entries_.end();
        for (Entries::const_iterator i = entries_.begin(); i != end; ++i) {
            if (i->ifdId() == ifdId) {
                uint16_t pos = i->tag() * 2;
                uint16_t size = pos + static_cast<uint16_t>(i->size());
                assert(size <= 1024);
                memcpy(buf.pData_ + pos, i->data(), i->size());
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

    std::ostream& CanonMakerNote::printCs10x0002(std::ostream& os,
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

    std::ostream& CanonMakerNote::printCs1Lens(std::ostream& os,
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
        os << std::fixed << std::setprecision(1)
           << len2 << " - " << len1 << " mm";
        os.copyfmt(oss);
        return os;
    }

    std::ostream& CanonMakerNote::printCs20x0002(std::ostream& os,
                                                 const Value& value)
    {
        // Ported from Exiftool by Will Stokes
        return os << exp(canonEv(value.toLong()) * log(2.0)) * 100.0 / 32.0;
    }

    std::ostream& CanonMakerNote::printCs20x0009(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        os << l << "";
        // Todo: determine unit
        return os;
    }

    std::ostream& CanonMakerNote::printCs20x000e(std::ostream& os,
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
            EXV_PRINT_TAG_BITMASK(canonCs2AFPointUsed)(os, value);
        }
        os << " used";
        return os;
    }

    std::ostream& CanonMakerNote::printCs20x0013(std::ostream& os,
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

    std::ostream& CanonMakerNote::printCs20x0015(std::ostream& os,
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

    std::ostream& CanonMakerNote::printCs20x0016(std::ostream& os,
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
