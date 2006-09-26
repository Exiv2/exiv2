// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006 Andreas Huggel <ahuggel@gmx.net>
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
  File:      minoltamn.cpp
  Version:   $Rev$
  Author(s): Gilles Caulier (gc) <caulier.gilles@kdemail.net>
             Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   06-May-06, gc: submitted
  Credits:   See header file.
 */

// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "minoltamn.hpp"
#include "makernote.hpp"
#include "value.hpp"
#include "tags.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    // -- Standard Minolta Makernotes tags ---------------------------------------------------------------

    //! Lookup table to translate Minolta color mode values to readable labels
    extern const TagDetails minoltaColorMode[] = {
        { 0,  "Natural Color"  },
        { 1,  "Black & White"  },
        { 2,  "Vivid Color"    },
        { 3,  "Solarization"   },
        { 4,  "AdobeRGB"       },
        { 5,  "Sepia"          },
        { 9,  "Natural"        },
        { 12, "Portrait"       },
        { 13, "Natural sRGB"   },
        { 14, "Natural+ sRGB"  },
        { 15, "Landscape"      },
        { 16, "Evening"        },	
        { 17, "Night Scene"    },    
        { 18, "Night Portrait" }    
    };

    //! Lookup table to translate Minolta image quality values to readable labels
    extern const TagDetails minoltaImageQuality[] = {
        { 0, "Raw"        },
        { 1, "Super Fine" },
        { 2, "Fine"       },
        { 3, "Standard"   },
        { 4, "Economy"    },
        { 5, "Extra Fine" }
    };

    //! Lookup table to translate Minolta zone matching values
    extern const TagDetails minoltaZoneMatching[] = {
        { 0, "ISO Setting Used" },
        { 1, "High Key"         },
        { 2, "Low Key"          }
    };

    //! Lookup table to translate Minolta image stabilization values
    extern const TagDetails minoltaImageStabilization[] = {
        { 1, "Off" },
        { 5, "On"  }
    };

    //! Lookup table to translate Minolta Lens id values to readable labels
    /* NOTE: duplicate tags value : 0/25520, 4/25920, 13/25610, 19/25910 22/26050/26070 */
    extern const TagDetails minoltaLensID[] = {
        { 0,     "AF28-85mm F3.5-4.5"                                                         },
        { 1,     "AF80-200mm F2.8G"                                                           },
        { 2,     "AF28-70mm F2.8G"                                                            },
        { 3,     "AF28-80mm F4-5.6"                                                           },
        { 4,     "AF85mm F1.4G"                                                               },
        { 5,     "AF35-70mm F3.5-4.5"                                                         },
        { 6,     "AF24-85mm F3.5-4.5"                                                         },
        { 7,     "AF100-300mm F4.5-5.6(D) APO or AF100-400mm F4.5-6.7(D)"                     },
        { 8,     "AF70-210mm F4.5-5.6"                                                        },
        { 9,     "AF50mm F3.5 Macro"                                                          },
        { 10,    "AF28-105mm F3.5-4.5"                                                        },
        { 11,    "AF300mm F4G APO"                                                            },
        { 12,    "AF100mm F2.8 Soft Focus"                                                    },
        { 13,    "AF75-300mm F4.5-5.6"                                                        },
        { 14,    "AF100-400mm F4.5-6.7 APO"                                                   },
        { 15,    "AF400mm F4.5G APO"                                                          },
        { 16,    "AF17-35mm F3.5G"                                                            },
        { 17,    "AF20-35mm F3.5-4.5"                                                         },
        { 18,    "AF28-80mm F3.5-5.6"                                                         },
        { 19,    "AF35mm F1.4G"                                                               },
        { 20,    "STF135mm F2.8[T4.5]"                                                        },
        { 22,    "AF35-80mm F4-5.6"                                                           },
        { 23,    "AF200mm F4G Macro"                                                          },
        { 24,    "AF24-105mm F3.5-4.5(D) or SIGMA 18-50mm F2.8 or Sigma DC 18-125mm F4-5,6 D" },
        { 25,    "AF100-300mm F4.5-5.6 APO(D)"                                                },
        { 27,    "AF85mm F1.4G(D)"                                                            },
        { 28,    "AF100mm F2.8 Macro(D)"                                                      },
        { 29,    "AF75-300mm F4.5-5.6(D)"                                                     },
        { 30,    "AF28-80mm F3.5-5.6(D)"                                                      },
        { 31,    "AF50mm F2.8 Macro(D) or AF50mm F3.5 Macro"                                  },
        { 32,    "AF100-400mm F4.5-6.7(D) x1.5 or AF300mm F2.8G(D) SSM"                       },
        { 33,    "AF70-200mm F2.8G SSM"                                                       },
        { 35,    "AF85mm F1.4G(D) Limited"                                                    },
        { 38,    "AF17-35mm F2.8-4(D)"                                                        },
        { 39,    "AF28-75mm F2.8(D)"                                                          },
        { 40,    "AFDT18-70mm F3.5-5.6(D)"                                                    },
        { 41,    "TAMRON Di II LD 11-18mm F4-5.6"                                             },
        { 128,   "TAMRON 18-200, 28-300 or 80-300mm F3.5-6.3"                                 },
        { 25501, "AF50mm F1.7"                                                                },
        { 25520, "AF28-85mm F3.5-4.5"                                                         },
        { 25521, "TOKINA 19-35mm F3.5-4.5 or TOKINA 28-70mm F2.8 AT-X"                        },
        { 25541, "AF35-105mm F3.5-4.5"                                                        },
        { 25551, "AF70-210mm F4 Macro or SIGMA 70-210mm F4-5.6 APO"                           },
        { 25581, "AF24-50mm F4"                                                               },
        { 25610, "AF75-300mm F4.5-5.6"                                                        },
        { 25611, "SIGMA 70-300mm F4-5.6 or SIGMA 300mm F4 APO Macro"                          },
        { 25621, "AF50mm F1.4 NEW"                                                            },
        { 25631, "AF300mm F2.8G"                                                              },
        { 25641, "AF50mm F2.8 Macro"                                                          },
        { 25661, "AF24mm F2.8 or SIGMA 17-35mm F2.8-4.0 EX-D"                                 },
        { 25721, "AF500mm F8 Reflex"                                                          },
        { 25781, "AF16mm F2.8 Fisheye or SIGMA 8mm F4 Fisheye"                                },
        { 25791, "AF20mm F2.8"                                                                },
        { 25811, "AF100mm F2.8 Macro(D) or TAMRON 90mm F2.8 Macro or SIGMA 180mm F5.6 Macro"  },
        { 25858, "TAMRON 24-135mm F3.5-5.6"                                                   },
        { 25891, "TOKINA 80-200mm F2.8"                                                       },
        { 25910, "AF35mm F1.4G"                                                               },
        { 25920, "AF85mm F1.4G"                                                               },
        { 25921, "AF85mm F1.4G(D)"                                                            },
        { 25931, "AF200mm F2.8G"                                                              },
        { 25961, "AF28mm F2"                                                                  },
        { 25981, "AF100mm F2"                                                                 },
        { 26050, "AF35-80mm F4-5.6"                                                           },
        { 26061, "AF100-300mm F4.5-5.6(D) or SIGMA 105mm F2.8 Macro EX-DG"                    },
        { 26070, "AF35-80mm F4-5.6"                                                           },
        { 26081, "AF300mm F2.8G"                                                              },
        { 26121, "AF200mm F2.8G(D)"                                                           },
        { 26131, "AF50mm F1.7"                                                                },
        { 26151, "AF28-105mm F3.5-4.5 Xi"                                                     },
        { 26241, "AF35-80mm F4-5.6"                                                           },
        { 45741, "AF200mm F2.8G x2 or TOKINA 300mm F2.8 x2"                                   }
    };

    // Minolta Tag Info
    const TagInfo MinoltaMakerNote::tagInfo_[] = {
        TagInfo(0x0000, "Version", "Makernote Version", "String 'MLT0' (not null terminated)", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0001, "CameraSettingsStdOld", "Camera Settings (Std Old)", "Standard Camera settings (Old Camera models like D5, D7, S304, and S404)", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0003, "CameraSettingsStdNew", "Camera Settings (Std New)", "Standard Camera settings (New Camera Models like D7u, D7i, and D7hi)", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0004, "CameraSettings7D", "Camera Settings (7D)", "Camera Settings (for Dynax 7D model)", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0018, "ImageStabilizationData", "Image Stabilization data", "Image stabilization data", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0040, "CompressedImageSize", "Compressed Image Size", "Compressed image size", minoltaIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0081, "Thumbnail", "Thumbnail", "Jpeg thumbnail 640x480 pixels", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0088, "ThumbnailOffset", "Thumbnail Offset", "Offset of the thumbnail", minoltaIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0089, "ThumbnailLength", "Thumbnail Length", "Size of the thumbnail", minoltaIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0101, "ColorMode", "Color Mode", "Color mode", minoltaIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaColorMode)),
        TagInfo(0x0102, "Quality", "Image Quality", "Image quality", minoltaIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaImageQuality)),

        // TODO: Tag 0x0103 : quality or image size (see ExifTool doc).

        TagInfo(0x0107, "ImageStabilization", "Image Stabilization", "Image stabilization", minoltaIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaImageStabilization)),
        TagInfo(0x010a, "ZoneMatching", "Zone Matching", "Zone matching", minoltaIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaZoneMatching)),
        TagInfo(0x010b, "ColorTemperature", "Color Temperature", "Color temperature", minoltaIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x010c, "LensID", "Lens ID", "Lens ID", minoltaIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaLensID)),
        TagInfo(0x0114, "CameraSettings5D", "Camera Settings (5D)", "Camera Settings (for Dynax 5D model)", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0e00, "PIM_IFD", "PIM IFD", "PIM information", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0f00, "CameraSettingsZ1", "Camera Settings (Z1)", "Camera Settings (for Z1, DImage X, and F100 models)", minoltaIfdId, makerTags, undefined, printValue),
        // End of list marker

        TagInfo(0xffff, "(UnknownMinoltaMakerNoteTag)", "(UnknownMinoltaMakerNoteTag)", "Unknown MinoltaMakerNote tag", minoltaIfdId, makerTags, invalidTypeId, printValue)
    };

    // -- Standard Minolta camera settings ---------------------------------------------------------------

    //! Lookup table to translate Minolta Std camera settings exposure mode values to readable labels
    extern const TagDetails minoltaExposureModeStd[] = {
        { 0, "Program"           },
        { 1, "Aperture Priority" },
        { 2, "Shutter Priority"  },
        { 3, "Manual"            }
    };

    //! Lookup table to translate Minolta Std camera settings exposure mode values to readable labels
    extern const TagDetails minoltaFlashModeStd[] = {
        { 0, "Fill flash"        },
        { 1, "Red-eye reduction" },
        { 2, "Rear flash sync"   },
        { 3, "Wireless"          }
    };

    //! Lookup table to translate Minolta Std camera settings white balance values to readable labels
    extern const TagDetails minoltaWhiteBalanceStd[] = {
        { 0,  "Auto"          },
        { 1,  "Daylight"      },
        { 2,  "Cloudy"        },
        { 3,  "Tungsten"      },
        { 5,  "Custom"        },
        { 7,  "Fluorescent"   },
        { 8,  "Fluorescent 2" },
        { 11, "Custom 2"      },
        { 12, "Custom 3"      }
    };

    //! Lookup table to translate Minolta Std camera settings image size values to readable labels
    extern const TagDetails minoltaImageSizeStd[] = {
        { 0, "Full"      },
        { 1, "1600x1200" },
        { 2, "1280x960"  },
        { 3, "640x480"   },
        { 6, "2080x1560" },
        { 7, "2560x1920" },
        { 8, "3264x2176" }
    };

    //! Lookup table to translate Minolta Std camera settings image quality values to readable labels
    extern const TagDetails minoltaImageQualityStd[] = {
        { 0, "Raw"        },
        { 1, "Super Fine"  },
        { 2, "Fine"       },
        { 3, "Standard"   },
        { 4, "Economy"    },
        { 5, "Extra Fine" }
    };

    //! Lookup table to translate Minolta Std camera settings drive mode values to readable labels
    extern const TagDetails minoltaDriveModeStd[] = {
        { 0, "Single"         },
        { 1, "Continuous"     },
        { 2, "Self-timer"     },
        { 4, "Bracketing"     },
        { 5, "Interval"       },
        { 6, "UHS continuous" },
        { 7, "HS continuous"  }
    };

    //! Lookup table to translate Minolta Std camera settings metering mode values to readable labels
    extern const TagDetails minoltaMeteringModeStd[] = {
        { 0, "Multi-segment"   },
        { 1, "Center weighted" },
        { 2, "Spot"            }
    };

    //! Lookup table to translate Minolta Std camera settings macro mode values to readable labels
    extern const TagDetails minoltaMacroModeStd[] = {
        { 0, "Off" },
        { 1, "On"  }
    };

    //! Lookup table to translate Minolta Std camera settings digital zoom values to readable labels
    extern const TagDetails minoltaDigitalZoomStd[] = {
        { 0, "Off"                      },
        { 1, "Electronic magnification" },
        { 2, "2x"                       }
    };

    //! Lookup table to translate Minolta Std camera bracket step mode values to readable labels
    extern const TagDetails minoltaBracketStepStd[] = {
        { 0, "1/3 EV" },
        { 1, "2/3 EV" },
        { 2, "1 EV"   }
    };

    //! Lookup table to translate Minolta Std camera settings AF points values to readable labels
    extern const TagDetails minoltaAFPointsStd[] = {
        { 0, "Center"        },
        { 1, "Top"           },
        { 2, "Top-Right"     },
        { 3, "Right"         },
        { 4, "Bottom-Right " },
        { 5, "Bottom"        },
        { 6, "Bottom-Left"   },
        { 7, "Left"          },
        { 8, "Top-Left"      }
    };

    //! Lookup table to translate Minolta Std camera settings white balance values to readable labels
    extern const TagDetails minoltaFlashStd[] = {
        { 0, "Did not fire" },
        { 1, "Fired"        }
    };

    //! Lookup table to translate Minolta Std camera settings file number memory values to readable labels
    extern const TagDetails minoltaFileNumberMemoryStd[] = {
        { 0, "Off" },
        { 1, "On"  }
    };

    //! Lookup table to translate Minolta Std camera settings sharpness values to readable labels
    extern const TagDetails minoltaSharpnessStd[] = {
        { 0, "Hard"   },
        { 1, "Normal" },
        { 2, "Soft"   }
    };

    //! Lookup table to translate Minolta Std camera settings subject program values to readable labels
    extern const TagDetails minoltaSubjectProgramStd[] = {
        { 0, "None"           },
        { 1, "Portrait"       },
        { 2, "Text"           },
        { 3, "Night Portrait" },
        { 4, "Sunset"         },
        { 5, "Sports Action"  }
    };

    //! Lookup table to translate Minolta Std camera settings ISO settings values to readable labels
    extern const TagDetails minoltaISOSettingStd[] = {
        { 0, "100"  },
        { 1, "200"  },
        { 2, "400"  },
        { 3, "800"  },
        { 4, "Auto" },
        { 5, "64"   }
    };

    //! Lookup table to translate Minolta Std camera settings model values to readable labels
    extern const TagDetails minoltaModelStd[] = {
        { 0, "DiMAGE 7, X1, X21, or X31" },
        { 1, "DiMAGE 5"                  },
        { 2, "DiMAGE S304"               },
        { 3, "DiMAGE S404"               },
        { 4, "DiMAGE 7i"                 },
        { 5, "DiMAGE 7Hi"                },
        { 6, "DiMAGE A1"                 },
        { 7, "DiMAGE A2 or S414"         }
    };

    //! Lookup table to translate Minolta Std camera settings interval mode values to readable labels
    extern const TagDetails minoltaIntervalModeStd[] = {
        { 0, "Still Image"      },
        { 1, "Time-lapse Movie" }
    };

    //! Lookup table to translate Minolta Std camera settings folder name values to readable labels
    extern const TagDetails minoltaFolderNameStd[] = {
        { 0, "Standard Form" },
        { 1, "Data Form"     }
    };

    //! Lookup table to translate Minolta Std camera settings color mode values to readable labels
    extern const TagDetails minoltaColorModeStd[] = {
        { 0, "Natural color" },
        { 1, "Black & White" },
        { 2, "Vivid color"   },
        { 3, "Solarization"  },
        { 4, "Adobe RGB"     }
    };

    //! Lookup table to translate Minolta Std camera settings internal flash values to readable labels
    extern const TagDetails minoltaInternalFlashStd[] = {
        { 0, "Did not fire" },
        { 1, "Fired"        }
    };

    //! Lookup table to translate Minolta Std camera settings wide focus zone values to readable labels
    extern const TagDetails minoltaWideFocusZoneStd[] = {
        { 0, "No zone"                              },
        { 1, "Center zone (horizontal orientation)" },
        { 1, "Center zone (vertical orientation)"   },
        { 1, "Left zone "                           },
        { 4, "Right zone"                           }
    };

    //! Lookup table to translate Minolta Std camera settings focus mode values to readable labels
    extern const TagDetails minoltaFocusModeStd[] = {
        { 0, "Auto focus"   },
        { 1, "Manual focus" }
    };

    //! Lookup table to translate Minolta Std camera settings focus area values to readable labels
    extern const TagDetails minoltaFocusAreaStd[] = {
        { 0, "Wide Focus (normal)" },
        { 1, "Spot Focus"          }
    };

    //! Lookup table to translate Minolta Std camera settings DEC switch position values to readable labels
    extern const TagDetails minoltaDECPositionStd[] = {
        { 0, "Exposure "  },
        { 1, "Contrast"   },
        { 2, "Saturation" },
        { 3, "Filter"     }
    };

    //! Lookup table to translate Minolta Std camera settings color profile values to readable labels
    extern const TagDetails minoltaColorProfileStd[] = {
        { 0, "Not Embedded" },
        { 1, "Embedded"     }
    };

    //! Lookup table to translate Minolta Std camera settings data Imprint values to readable labels
    extern const TagDetails minoltaDataImprintStd[] = {
        { 0, "None"        },
        { 1, "YYYY/MM/DD"  },
        { 2, "MM/DD/HH:MM" },
        { 3, "Text"        },
        { 4, "Text + ID#"  }
    };

    //! Lookup table to translate Minolta Std camera settings flash metering values to readable labels
    extern const TagDetails minoltaFlashMeteringStd[] = {
        { 0, "ADI (Advanced Distance Integration)" },
        { 1, "Pre-flash TTl"                       },
        { 2, "Manual Flash Control"                }
    };

    std::ostream& MinoltaMakerNote::printMinoltaExposureSpeedStd(std::ostream& os, const Value& value)
    {
        // From the PHP JPEG Metadata Toolkit
        os << (value.toLong()/8)-1;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaExposureTimeStd(std::ostream& os, const Value& value)
    {
        // From the PHP JPEG Metadata Toolkit
        os << (value.toLong()/8)-6;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaFNumberStd(std::ostream& os, const Value& value)
    {
        // From the PHP JPEG Metadata Toolkit
        os << (value.toLong()/8)-1;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaExposureCompensationStd(std::ostream& os, const Value& value)
    {
        // From the PHP JPEG Metadata Toolkit
        os << value.toLong()/256;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaFocalLengthStd(std::ostream& os, const Value& value)
    {
        // From the PHP JPEG Metadata Toolkit
        os << (value.toLong()/3)-2;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaDateStd(std::ostream& os, const Value& value)
    {
        // From the PHP JPEG Metadata Toolkit
        os << value.toLong() / 65536 << ":" << std::right << std::setw(2) << std::setfill('0')
           << (value.toLong() - value.toLong() / 65536 * 65536) / 256 << ":"
           << std::right << std::setw(2) << std::setfill('0') << value.toLong() % 256;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaTimeStd(std::ostream& os, const Value& value)
    {
        // From the PHP JPEG Metadata Toolkit
        os << std::right << std::setw(2) << std::setfill('0') << value.toLong() / 65536
           << ":" << std::right << std::setw(2) << std::setfill('0')
           << (value.toLong() - value.toLong() / 65536 * 65536) / 256 << ":"
           << std::right << std::setw(2) << std::setfill('0') << value.toLong() % 256;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaFlashExposureCompStd(std::ostream& os, const Value& value)
    {
        // From the PHP JPEG Metadata Toolkit
        os << (value.toLong()-6)/3;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaWhiteBalanceStd(std::ostream& os, const Value& value)
    {
        // From the PHP JPEG Metadata Toolkit
        os << value.toLong()/256;
        return os;
    }

    std::ostream& MinoltaMakerNote::printMinoltaBrightnessStd(std::ostream& os, const Value& value)
    {
        // From the PHP JPEG Metadata Toolkit
        os << (value.toLong()/8)-6;
        return os;
    }

    // Minolta Standard Camera Settings Tag Info (Old and New)
    const TagInfo MinoltaMakerNote::tagInfoCsStd_[] = {
        TagInfo(0x0001, "ExposureMode", "Exposure Mode", "Exposure mode", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaExposureModeStd)),
        TagInfo(0x0002, "FlashMode", "Flash Mode", "Flash mode", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaFlashModeStd)),
        TagInfo(0x0003, "WhiteBalance", "White Balance", "White balance", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaWhiteBalanceStd)),
        TagInfo(0x0004, "ImageSize", "Image Size", "Image size", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaImageSizeStd)),
        TagInfo(0x0005, "Quality", "Image Quality", "Image quality", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaImageQualityStd)),
        TagInfo(0x0006, "DriveMode", "Drive Mode", "Drive mode", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaDriveModeStd)),
        TagInfo(0x0007, "MeteringMode", "Metering Mode", "Metering mode", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaMeteringModeStd)),
        TagInfo(0x0008, "ExposureSpeed", "Exposure Speed", "Exposure speed", minoltaCsNewIfdId, makerTags, unsignedLong, printMinoltaExposureSpeedStd),
        TagInfo(0x0009, "ExposureTime", "Exposure Time", "Exposure time", minoltaCsNewIfdId, makerTags, unsignedLong, printMinoltaExposureTimeStd),
        TagInfo(0x000A, "FNumber", "FNumber", "FNumber", minoltaCsNewIfdId, makerTags, unsignedLong, printMinoltaFNumberStd),
        TagInfo(0x000B, "MacroMode", "Macro Mode", "Macro mode", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaMacroModeStd)),
        TagInfo(0x000C, "DigitalZoom", "Digital Zoom", "Digital zoom", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaDigitalZoomStd)),
        TagInfo(0x000D, "ExposureCompensation", "Exposure Compensation", "Exposure compensation", minoltaCsNewIfdId, makerTags, unsignedLong, printMinoltaExposureCompensationStd),
        TagInfo(0x000E, "BracketStep", "Bracket Step", "Bracket step", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaBracketStepStd)),
        TagInfo(0x0010, "IntervalLength", "Interval Length", "Interval length", minoltaCsNewIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0011, "IntervalNumber", "Interval Number", "Interval number", minoltaCsNewIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0012, "FocalLength", "Focal Length", "Focal length", minoltaCsNewIfdId, makerTags, unsignedLong, printMinoltaFocalLengthStd),
        TagInfo(0x0013, "FocusDistance", "Focus Distance", "Focus distance", minoltaCsNewIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0014, "Flash", "Flash", "Flash", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaFlashStd)),
        TagInfo(0x0015, "MinoltaDate", "Minolta Date", "Minolta date", minoltaCsNewIfdId, makerTags, unsignedLong, printMinoltaDateStd),
        TagInfo(0x0016, "MinoltaTime", "Minolta Time", "Minolta time", minoltaCsNewIfdId, makerTags, unsignedLong, printMinoltaTimeStd),
        TagInfo(0x0017, "MaxAperture", "Max Aperture", "Max aperture", minoltaCsNewIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x001A, "FileNumberMemory", "File Number Memory", "File number memory", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaFileNumberMemoryStd)),
        TagInfo(0x001B, "ImageNumber", "Image Number", "Image number", minoltaCsNewIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x001C, "ColorBalanceRed", "Color Balance Red", "Color balance red", minoltaCsNewIfdId, makerTags, unsignedLong, printMinoltaWhiteBalanceStd),
        TagInfo(0x001D, "ColorBalanceGreen", "Color Balance Green", "Color balance green", minoltaCsNewIfdId, makerTags, unsignedLong, printMinoltaWhiteBalanceStd),
        TagInfo(0x001E, "ColorBalanceBlue", "Color Balance Blue", "Color balance blue", minoltaCsNewIfdId, makerTags, unsignedLong, printMinoltaWhiteBalanceStd),
        TagInfo(0x001F, "Saturation", "Saturation", "Saturation", minoltaCsNewIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0020, "Contrast", "Contrast", "Contrast", minoltaCsNewIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0021, "Sharpness", "Sharpness", "Sharpness", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaSharpnessStd)),
        TagInfo(0x0022, "SubjectProgram", "Subject Program", "Subject program", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaSubjectProgramStd)),
        TagInfo(0x0023, "FlashExposureComp", "Flash Exposure Compensation", "Flash exposure compensation in EV", minoltaCsNewIfdId, makerTags, unsignedLong, printMinoltaFlashExposureCompStd),
        TagInfo(0x0024, "ISOSpeed", "ISO Speed Mode", "ISO speed setting", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaISOSettingStd)),
        TagInfo(0x0025, "MinoltaModel", "Minolta Model", "Minolta model", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaModelStd)),
        TagInfo(0x0026, "IntervalMode", "Interval Mode", "Interval mode", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaIntervalModeStd)),
        TagInfo(0x0027, "FolderName", "Folder Name", "Folder name", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaFolderNameStd)),
        TagInfo(0x0028, "ColorMode", "ColorMode", "ColorMode", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaColorModeStd)),
        TagInfo(0x0029, "ColorFilter", "Color Filter", "Color filter", minoltaCsNewIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x002A, "BWFilter", "Black White Filter", "Black and white filter", minoltaCsNewIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x002B, "InternalFlash", "Internal Flash", "Internal flash", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaInternalFlashStd)),
        TagInfo(0x002C, "Brightness", "Brightness", "Brightness", minoltaCsNewIfdId, makerTags, unsignedLong, printMinoltaBrightnessStd),
        TagInfo(0x002D, "SpotFocusPointX", "Spot Focus Point X", "Spot focus point X", minoltaCsNewIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x002E, "SpotFocusPointY", "Spot Focus Point Y", "Spot focus point Y", minoltaCsNewIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x002F, "WideFocusZone", "Wide Focus Zone", "Wide focus zone", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaWideFocusZoneStd)),
        TagInfo(0x0030, "FocusMode", "Focus Mode", "Focus mode", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaFocusModeStd)),
        TagInfo(0x0031, "FocusArea", "Focus area", "Focus area", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaFocusAreaStd)),
        TagInfo(0x0032, "DECPosition", "DEC Switch Position", "DEC switch position", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaDECPositionStd)),
        TagInfo(0x0033, "ColorProfile", "Color Profile", "Color profile", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaColorProfileStd)),
        TagInfo(0x0034, "DataImprint", "Data Imprint", "Data imprint", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaDataImprintStd)),
        TagInfo(0x003F, "FlashMetering", "Flash Metering", "Flash metering", minoltaCsNewIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(minoltaFlashMeteringStd)),

        // End of list marker
        TagInfo(0xffff, "(UnknownMinoltaCsTag)", "(UnknownMinoltaCsTag)", "Unknown Minolta Camera Settings tag", minoltaCsNewIfdId, makerTags, invalidTypeId, printValue)
    };

    // -- Minolta Dynax 7D camera settings ---------------------------------------------------------------

    //! Lookup table to translate Minolta Dynax 7D camera settings exposure mode values to readable labels
    extern const TagDetails minoltaExposureMode7D[] = {
        { 0, "Program"           },
        { 1, "Aperture Priority" },
        { 2, "Shutter Priority"  },
        { 3, "Manual"            },
        { 4, "Auto"              },
        { 5, "Program-shift A"   },
        { 6, "Program-shift S"   }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings image size values to readable labels
    extern const TagDetails minoltaImageSize7D[] = {
        { 0, "Large"  },
        { 1, "Medium" },
        { 2, "Small"  }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings image quality values to readable labels
    extern const TagDetails minoltaImageQuality7D[] = {
        { 0,  "Raw"      },
        { 16, "Fine"     },
        { 32, "Normal"   },
        { 34, "Raw+Jpeg" },
        { 48, "Economy"  }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings white balance values to readable labels
    extern const TagDetails minoltaWhiteBalance7D[] = {
        { 0,   "Auto"        },
        { 1,   "Daylight"    },
        { 2,   "Shade"       },
        { 3,   "Cloudy"      },
        { 4,   "Tungsten"    },
        { 5,   "Fluorescent" },
        { 256, "Kelvin"      },
        { 512, "Manual"      }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings focus mode values to readable labels
    extern const TagDetails minoltaFocusMode7D[] = {
        { 0, "Single-shot AF" },
        { 1, "Continuous AF"  },
        { 3, "Automatic AF"   },
        { 4, "Manual"         }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings AF points values to readable labels
    extern const TagDetails minoltaAFPoints7D[] = {
        { 1,   "Center"        },
        { 2,   "Top"           },
        { 4,   "Top-Right"     },
        { 8,   "Right"         },
        { 16,  "Bottom-Right " },
        { 32,  "Bottom"        },
        { 64,  "Bottom-Left"   },
        { 128, "Left"          },
        { 256, "Top-Left"      }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings white balance values to readable labels
    extern const TagDetails minoltaFlash7D[] = {
        { 0, "Did not fire" },
        { 1, "Fired"        }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings ISO settings values to readable labels
    extern const TagDetails minoltaISOSetting7D[] = {
        { 0, "Auto"  },
        { 1, "100"  },
        { 3, "200"  },
        { 4, "400"  },
        { 5, "800"  },
        { 6, "1600" },
        { 7, "3200" }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings color space values to readable labels
    extern const TagDetails minoltaColorSpace7D[] = {
        { 0, "sRGB (Natural)"  },
        { 1, "sRGB (Natural+)" },
        { 4, "Adobe RGB"       }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings rotation values to readable labels
    extern const TagDetails minoltaRotation7D[] = {
        { 72, "Horizontal (normal)" },
        { 76, "Rotate 90 CW"        },
        { 82, "Rotate 270 CW"       }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings noise reduction values to readable labels
    extern const TagDetails minoltaNoiseReduction7D[] = {
        { 0, "Off" },
        { 1, "On"  }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings image stabilization values to readable labels
    extern const TagDetails minoltaImageStabilization7D[] = {
        { 0, "Off" },
        { 1, "On"  }
    };

    //! Lookup table to translate Minolta Dynax 7D camera settings zone matching on values to readable labels
    extern const TagDetails minoltaZoneMatchingOn7D[] = {
        { 0, "Off" },
        { 1, "On"  }
    };

    // Minolta Dynax 7D Camera Settings Tag Info
    const TagInfo MinoltaMakerNote::tagInfoCs7D_[] = {
        TagInfo(0x0000, "ExposureMode", "Exposure Mode", "Exposure mode", minoltaCs7DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaExposureMode7D)),
        TagInfo(0x0002, "ImageSize", "Image Size", "Image size", minoltaCs7DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaImageSize7D)),
        TagInfo(0x0003, "Quality", "Image Quality", "Image quality", minoltaCs7DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaImageQuality7D)),
        TagInfo(0x0004, "WhiteBalance", "White Balance", "White balance", minoltaCs7DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaWhiteBalance7D)),
        TagInfo(0x000E, "FocusMode", "Focus Mode", "Focus mode", minoltaCs7DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaFocusMode7D)),
        TagInfo(0x0010, "AFPoints", "AF Points", "AF points", minoltaCs7DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaAFPoints7D)),
        TagInfo(0x0015, "Flash", "Flash", "Flash", minoltaCs7DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaFlash7D)),
        TagInfo(0x0016, "FlashMode", "Flash Mode", "Flash mode", minoltaCs7DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001C, "ISOSpeed", "ISO Speed Mode", "ISO speed setting", minoltaCs7DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaISOSetting7D)),
        TagInfo(0x001E, "ExposureCompensation", "Exposure Compensation", "Exposure compensation", minoltaCs7DIfdId, makerTags, signedShort, printValue),
        TagInfo(0x0025, "ColorSpace", "Color Space", "Color space", minoltaCs7DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaColorSpace7D)),
        TagInfo(0x0026, "Sharpness", "Sharpness", "Sharpness", minoltaCs7DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0027, "Contrast", "Contrast", "Contrast", minoltaCs7DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0028, "Saturation", "Saturation", "Saturation", minoltaCs7DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x002D, "FreeMemoryCardImages", "Free Memory Card Images", "Free memory card images", minoltaCs7DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x003F, "ColorTemperature", "Color Temperature", "Color temperature", minoltaCs7DIfdId, makerTags, signedShort, printValue),
        TagInfo(0x0040, "Hue", "Hue", "Hue", minoltaCs7DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0046, "Rotation", "Rotation", "Rotation", minoltaCs7DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaRotation7D)),
        TagInfo(0x0047, "FNumber", "FNumber", "FNumber", minoltaCs7DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0048, "ExposureTime", "Exposure Time", "Exposure time", minoltaCs7DIfdId, makerTags, unsignedShort, printValue),
        // 0x004A is a dupplicate than 0x002D.
        TagInfo(0x004A, "FreeMemoryCardImages", "Free Memory Card Images", "Free memory card images", minoltaCs7DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x005E, "ImageNumber", "Image Number", "Image number", minoltaCs7DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0060, "NoiseReduction", "Noise Reduction", "Noise reduction", minoltaCs7DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaNoiseReduction7D)),
        // 0x0062 is a dupplicate than 0x005E.
        TagInfo(0x0062, "ImageNumber", "Image Number", "Image number", minoltaCs7DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0071, "ImageStabilization", "Image Stabilization", "Image stabilization", minoltaCs7DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaImageStabilization7D)),
        TagInfo(0x0075, "ZoneMatchingOn", "Zone Matching On", "Zone matching on", minoltaCs7DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaZoneMatchingOn7D)),

        // End of list marker
        TagInfo(0xffff, "(UnknownMinoltaCs7DTag)", "(UnknownMinoltaCs7DTag)", "Unknown Minolta Camera Settings 7D tag", minoltaCs7DIfdId, makerTags, invalidTypeId, printValue)
    };

    // -- Minolta Dynax 5D camera settings ---------------------------------------------------------------

    //! Lookup table to translate Minolta Dynax 5D camera settings exposure mode values to readable labels
    extern const TagDetails minoltaExposureMode5D[] = {
        { 0,    "Program"           },
        { 1,    "Aperture Priority" },
        { 2,    "Shutter Priority"  },
        { 3,    "Manual"            },
        { 4,    "Auto"              },
        { 4131, "Connected Copying" }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings image size values to readable labels
    extern const TagDetails minoltaImageSize5D[] = {
        { 0, "Large"  },
        { 1, "Medium" },
        { 2, "Small"  }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings image quality values to readable labels
    extern const TagDetails minoltaImageQuality5D[] = {
        { 0,  "Raw"      },
        { 16, "Fine"     },
        { 32, "Normal"   },
        { 34, "Raw+Jpeg" },
        { 48, "Economy"  }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings white balance values to readable labels
    extern const TagDetails minoltaWhiteBalance5D[] = {
        { 0,   "Auto"        },
        { 1,   "Daylight"    },
        { 2,   "Cloudy"      },
        { 3,   "Shade"       },
        { 4,   "Tungsten"    },
        { 5,   "Fluorescent" },
        { 6,   "Flash"       },
        { 256, "Kelvin"      },
        { 512, "Manual"      }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings flash labels
    extern const TagDetails minoltaFlash5D[] = {
        { 0, "Did not fire" },
        { 1, "Fired"        }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings metering mode values to readable labels
    extern const TagDetails minoltaMeteringMode5D[] = {
        { 0, "Multi-segment"   },
        { 1, "Center weighted" },
        { 2, "Spot"            }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings ISO settings values to readable labels
    extern const TagDetails minoltaISOSetting5D[] = {
        { 0,  "Auto"                     },
        { 1,  "100"                      },
        { 3,  "200"                      },
        { 4,  "400"                      },
        { 5,  "800"                      },
        { 6,  "1600"                     },
        { 7,  "3200"                     },
        { 8,  "200 (Zone Matching High)" },
        { 10, "80 (Zone Matching Low)"   }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings rotation values to readable labels
    extern const TagDetails minoltaRotation5D[] = {
        { 72, "Horizontal (normal)" },
        { 76, "Rotate 90 CW"        },
        { 82, "Rotate 270 CW"       }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings noise reduction values to readable labels
    extern const TagDetails minoltaNoiseReduction5D[] = {
        { 0, "Off" },
        { 1, "On"  }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings image stabilization values to readable labels
    extern const TagDetails minoltaImageStabilization5D[] = {
        { 0, "Off" },
        { 1, "On"  }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings focus position values to readable labels
    extern const TagDetails minoltaFocusPosition5D[] = {
        { 0, "Wide"       },
        { 1, "Central"    },
        { 2, "Up"         },
        { 3, "Up Right"   },
        { 4, "Right"      },
        { 5, "Down Right" },
        { 6, "Down"       },
        { 7, "Down Left"  },
        { 8, "Left"       },
        { 9, "Up Left"    }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings focus area values to readable labels
    extern const TagDetails minoltaFocusArea5D[] = {
        { 0, "Wide"      },
        { 1, "Selection" },
        { 2, "Spot"      }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings focus mode values to readable labels
    extern const TagDetails minoltaAFMode5D[] = {
        { 0, "AF-A" },
        { 1, "AF-S" },
        { 2, "AF-D" },
        { 3, "DMF"  }
    };

    //! Lookup table to translate Minolta Dynax 5D camera settings focus mode values to readable labels
    extern const TagDetails minoltaFocusMode5D[] = {
        { 0, "AF" },
        { 1, "MF" }
    };

    //! Method to convert Minolta Dynax 5D exposure manual bias values.
    std::ostream& MinoltaMakerNote::printMinoltaExposureManualBias5D(std::ostream& os, const Value& value)
    {
        // From Xavier Raynaud: the value is converted from 0:256 to -5.33:5.33

        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(2)
           << (float (value.toLong()-128)/24);
        os.copyfmt(oss);
        return os;
    }

    //! Method to convert Minolta Dynax 5D exposure compensation values.
    std::ostream& MinoltaMakerNote::printMinoltaExposureCompensation5D(std::ostream& os, const Value& value)
    {
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(2)
           << (float (value.toLong()-300)/100);
        os.copyfmt(oss);
        return os;
    }

    // Minolta Dynax 5D Camera Settings Tag Info
    const TagInfo MinoltaMakerNote::tagInfoCs5D_[] = {
        TagInfo(0x000A, "ExposureMode", "Exposure Mode", "Exposure mode", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaExposureMode5D)),
        TagInfo(0x000C, "ImageSize", "Image Size", "Image size", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaImageSize5D)),
        TagInfo(0x000D, "Quality", "Image Quality", "Image quality", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaImageQuality5D)),
        TagInfo(0x000E, "WhiteBalance", "White Balance", "White balance", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaWhiteBalance5D)),
        TagInfo(0x001a, "FocusPosition", "Focus Position", "Focus position", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaFocusPosition5D)),
        TagInfo(0x001b, "FocusArea", "Focus Area", "Focus area", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaFocusArea5D)),
        TagInfo(0x001F, "Flash", "Flash", "Flash", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaFlash5D)),
        TagInfo(0x0025, "MeteringMode", "Metering Mode", "Metering mode", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaMeteringMode5D)),
        TagInfo(0x0026, "ISOSpeed", "ISO Speed Mode", "ISO speed setting", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaISOSetting5D)),
        TagInfo(0x0030, "Sharpness", "Sharpness", "Sharpness", minoltaCs5DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0031, "Contrast", "Contrast", "Contrast", minoltaCs5DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0032, "Saturation", "Saturation", "Saturation", minoltaCs5DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0035, "ExposureTime", "Exposure Time", "Exposure time", minoltaCs5DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0036, "FNumber", "FNumber", "FNumber", minoltaCs5DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0037, "FreeMemoryCardImages", "Free Memory Card Images", "Free memory card images", minoltaCs5DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0038, "ExposureRevision", "Exposure Revision", "Exposure revision", minoltaCs5DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0048, "FocusMode", "Focus Mode", "Focus mode", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaFocusMode5D)),
        TagInfo(0x0049, "ColorTemperature", "Color Temperature", "Color temperature", minoltaCs5DIfdId, makerTags, signedShort, printValue),
        TagInfo(0x0050, "Rotation", "Rotation", "Rotation", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaRotation5D)),
        TagInfo(0x0053, "ExposureCompensation", "Exposure Compensation", "Exposure compensation", minoltaCs5DIfdId, makerTags, unsignedShort, printMinoltaExposureCompensation5D),
        TagInfo(0x0054, "FreeMemoryCardImages", "Free Memory Card Images", "Free memory card images", minoltaCs5DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0091, "ExposureManualBias", "Exposure Manual Bias", "Exposure manual bias", minoltaCs5DIfdId, makerTags, unsignedShort, printMinoltaExposureManualBias5D),
        TagInfo(0x009e, "AFMode", "AF Mode", "AF mode", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaAFMode5D)),
        TagInfo(0x00AE, "ImageNumber", "Image Number", "Image number", minoltaCs5DIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00B0, "NoiseReduction", "Noise Reduction", "Noise reduction", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaNoiseReduction5D)),
        TagInfo(0x00BD, "ImageStabilization", "Image Stabilization", "Image stabilization", minoltaCs5DIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(minoltaImageStabilization5D)),

        // From Xavier Raynaud: some notes on missing tags.
        // 0x0051 seems to be identical to FNumber (0x0036). An approx. relation between Tag value
        // and Fstop is exp(-0.335+value*0.043)
        // 0x0052 seems to be identical to ExposureTime (0x0035). An approx. relation between Tag
        // value and Exposure time is exp(-4+value*0.085)

        // End of list marker
        TagInfo(0xffff, "(UnknownMinoltaCs5DTag)", "(UnknownMinoltaCs5DTag)", "Unknown Minolta Camera Settings 5D tag", minoltaCs5DIfdId, makerTags, invalidTypeId, printValue)
    };

    // TODO : Add camera settings tags info "New2"...

    //! @cond IGNORE
    MinoltaMakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("KONICA MINOLTA*", "*", createMinoltaMakerNote);
        MakerNoteFactory::registerMakerNote("Minolta*", "*", createMinoltaMakerNote);

        MakerNoteFactory::registerMakerNote(minoltaIfdId,      MakerNote::AutoPtr(new MinoltaMakerNote));
        MakerNoteFactory::registerMakerNote(minoltaCs5DIfdId,  MakerNote::AutoPtr(new MinoltaMakerNote));
        MakerNoteFactory::registerMakerNote(minoltaCs7DIfdId,  MakerNote::AutoPtr(new MinoltaMakerNote));
        MakerNoteFactory::registerMakerNote(minoltaCsOldIfdId, MakerNote::AutoPtr(new MinoltaMakerNote));
        MakerNoteFactory::registerMakerNote(minoltaCsNewIfdId, MakerNote::AutoPtr(new MinoltaMakerNote));

        ExifTags::registerMakerTagInfo(minoltaIfdId,      tagInfo_);
        ExifTags::registerMakerTagInfo(minoltaCs5DIfdId,  tagInfoCs5D_);
        ExifTags::registerMakerTagInfo(minoltaCs7DIfdId,  tagInfoCs7D_);
        ExifTags::registerMakerTagInfo(minoltaCsOldIfdId, tagInfoCsStd_);
        ExifTags::registerMakerTagInfo(minoltaCsNewIfdId, tagInfoCsStd_);
    }
    //! @endcond

    int MinoltaMakerNote::read(const byte* buf, long len, long start, ByteOrder byteOrder, long shift)
    {
        int rc = IfdMakerNote::read(buf, len, start, byteOrder, shift);
        if (rc) return rc;

        // Decode Dynax 5D camera settings and add settings as additional entries
        Entries::iterator cs5D = ifd_.findTag(0x0114);

        if (cs5D != ifd_.end() && cs5D->type() == undefined) {
            for (uint16_t c = 0; cs5D->count()/2 > c; ++c)  {
                addCsEntry(minoltaCs5DIfdId, c, cs5D->offset() + c*2, cs5D->data() + c*2, 1);
            }
            // Discard the original entry
            ifd_.erase(cs5D);
        }

        // Decode Dynax 7D camera settings and add settings as additional entries
        Entries::iterator cs7D = ifd_.findTag(0x0004);

        if (cs7D != ifd_.end() && cs7D->type() == undefined) {
            for (uint16_t c = 0; cs7D->count()/2 > c; ++c)  {
                addCsEntry(minoltaCs7DIfdId, c, cs7D->offset() + c*2, cs7D->data() + c*2, 1);
            }
            // Discard the original entry
            ifd_.erase(cs7D);
        }

        // Decode Old Std camera settings and add settings as additional entries
        Entries::iterator csOldStd = ifd_.findTag(0x0001);

        if (csOldStd != ifd_.end() && csOldStd->type() == undefined) {
            for (uint16_t c = 0; csOldStd->count()/4 > c; ++c)  {
                addCsStdEntry(minoltaCsOldIfdId, c, csOldStd->offset() + c*4, csOldStd->data() + c*4, 1);
            }
            // Discard the original entry
            ifd_.erase(csOldStd);
        }

        // Decode New Std camera settings and add settings as additional entries
        Entries::iterator csNewStd = ifd_.findTag(0x0003);

        if (csNewStd != ifd_.end() && csNewStd->type() == undefined) {
            for (uint16_t c = 0; csNewStd->count()/4 > c; ++c)  {
                addCsStdEntry(minoltaCsNewIfdId, c, csNewStd->offset() + c*4, csNewStd->data() + c*4, 1);
            }
            // Discard the original entry
            ifd_.erase(csNewStd);
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

    void MinoltaMakerNote::addCsEntry(IfdId ifdId, uint16_t tag, long offset, const byte* data, int count)
    {
        Entry e(false);
        e.setIfdId(ifdId);
        e.setTag(tag);
        e.setOffset(offset);
        e.setValue(unsignedShort, count, data, 2*count, bigEndian);
        add(e);
    }

    void MinoltaMakerNote::addCsStdEntry(IfdId ifdId, uint32_t tag, long offset, const byte* data, int count)
    {
        Entry e(false);
        e.setIfdId(ifdId);
        e.setTag(tag);
        e.setOffset(offset);
        e.setValue(unsignedLong, count, data, 4*count, bigEndian);
        add(e);
    }

    void MinoltaMakerNote::add(const Entry& entry)
    {
        assert(alloc_ == entry.alloc());
        assert(   entry.ifdId() == minoltaIfdId
               || entry.ifdId() == minoltaCs5DIfdId
               || entry.ifdId() == minoltaCs7DIfdId
               || entry.ifdId() == minoltaCsOldIfdId
               || entry.ifdId() == minoltaCsNewIfdId);
        // allow duplicates
        entries_.push_back(entry);
    }

    long MinoltaMakerNote::copy(byte* buf, ByteOrder byteOrder, long offset)
    {
        if (byteOrder_ == invalidByteOrder) byteOrder_ = byteOrder;

        assert(ifd_.alloc());
        ifd_.clear();

        // Add all standard Minolta entries to the IFD
        Entries::const_iterator end = entries_.end();
        for (Entries::const_iterator i = entries_.begin(); i != end; ++i) {
            if (i->ifdId() == minoltaIfdId) {
                ifd_.add(*i);
            }
        }

        // Collect Dynax 5D camera settings entries and add the original Minolta tag
        Entry cs5D;
        if (assemble(cs5D, minoltaCs5DIfdId, 0x0114, bigEndian)) {
            ifd_.erase(0x0114);
            ifd_.add(cs5D);
        }

        // Collect Dynax 7D camera settings entries and add the original Minolta tag
        Entry cs7D;
        if (assemble(cs7D, minoltaCs7DIfdId, 0x0004, bigEndian)) {
            ifd_.erase(0x0004);
            ifd_.add(cs7D);
        }

        // Collect Old Std camera settings entries and add the original Minolta tag
        Entry csOldStd;
        if (assembleStd(csOldStd, minoltaCsOldIfdId, 0x0001, bigEndian)) {
            ifd_.erase(0x0001);
            ifd_.add(csOldStd);
        }

        // Collect New Std camera settings entries and add the original Minolta tag
        Entry csNewStd;
        if (assembleStd(csNewStd, minoltaCsNewIfdId, 0x0003, bigEndian)) {
            ifd_.erase(0x0003);
            ifd_.add(csNewStd);
        }

        return IfdMakerNote::copy(buf, byteOrder_, offset);
    } // MinoltaMakerNote::copy

    void MinoltaMakerNote::updateBase(byte* pNewBase)
    {
        byte* pBase = ifd_.updateBase(pNewBase);
        if (absShift_ && !alloc_) {
            Entries::iterator end = entries_.end();
            for (Entries::iterator pos = entries_.begin(); pos != end; ++pos) {
                pos->updateBase(pBase, pNewBase);
            }
        }
    } // MinoltaMakerNote::updateBase

    long MinoltaMakerNote::size() const
    {
        Ifd ifd(minoltaIfdId, 0, alloc_); // offset doesn't matter

        // Add all standard Minolta entries to the IFD
        Entries::const_iterator end = entries_.end();
        for (Entries::const_iterator i = entries_.begin(); i != end; ++i) {
            if (i->ifdId() == minoltaIfdId) {
                ifd.add(*i);
            }
        }
        // Collect Dynax 5D camera settings entries and add the original Minolta tag
        Entry cs5D(alloc_);
        if (assemble(cs5D, minoltaCs5DIfdId, 0x0114, bigEndian)) {
            ifd.erase(0x0114);
            ifd.add(cs5D);
        }

        // Collect Dynax 7D camera settings entries and add the original Minolta tag
        Entry cs7D(alloc_);
        if (assemble(cs7D, minoltaCs7DIfdId, 0x0004, bigEndian)) {
            ifd.erase(0x0004);
            ifd.add(cs7D);
        }

        // Collect Old Std camera settings entries and add the original Minolta tag
        Entry csOldStd(alloc_);
        if (assembleStd(csOldStd, minoltaCsOldIfdId, 0x0001, bigEndian)) {
            ifd.erase(0x0001);
            ifd.add(csOldStd);
        }

        // Collect New Std camera settings entries and add the original Minolta tag
        Entry csNewStd(alloc_);
        if (assembleStd(csNewStd, minoltaCsNewIfdId, 0x0003, bigEndian)) {
            ifd.erase(0x0003);
            ifd.add(csNewStd);
        }

        return headerSize() + ifd.size() + ifd.dataSize();
    } // MinoltaMakerNote::size

    long MinoltaMakerNote::assemble(Entry&    e,
                                    IfdId     ifdId, 
                                    uint16_t  tag, 
                                    ByteOrder /*byteOrder*/) const
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
            e.setIfdId(minoltaIfdId);
            e.setIdx(0); // don't care
            e.setTag(tag);
            e.setOffset(0); // will be calculated when the IFD is written
            e.setValue(undefined, len, buf.pData_, len * 2);
        }
        return len;
    } // MinoltaMakerNote::assemble

    long MinoltaMakerNote::assembleStd(Entry&    e,
                                       IfdId     ifdId,
                                       uint32_t  tag,
                                       ByteOrder /*byteOrder*/) const
    {
        DataBuf buf(1024);
        memset(buf.pData_, 0x0, 1024);
        uint32_t len = 0;
        Entries::const_iterator end = entries_.end();
        for (Entries::const_iterator i = entries_.begin(); i != end; ++i) {
            if (i->ifdId() == ifdId) {
                uint32_t pos = i->tag() * 4;
                uint32_t size = pos + static_cast<uint32_t>(i->size());
                assert(size <= 1024);
                memcpy(buf.pData_ + pos, i->data(), i->size());
                if (len < size) len = size;
            }
        }
        if (len > 0) {
            e.setIfdId(minoltaIfdId);
            e.setIdx(0); // don't care
            e.setTag(tag);
            e.setOffset(0); // will be calculated when the IFD is written
            e.setValue(undefined, len, buf.pData_, len * 4);
        }
        return len;
    } // MinoltaMakerNote::assembleStd

    Entries::const_iterator MinoltaMakerNote::findIdx(int idx) const
    {
        return std::find_if(entries_.begin(), entries_.end(), FindEntryByIdx(idx));
    }

    MinoltaMakerNote::MinoltaMakerNote(bool alloc)
        : IfdMakerNote(minoltaIfdId, alloc)
    {
    }

    MinoltaMakerNote::MinoltaMakerNote(const MinoltaMakerNote& rhs)
        : IfdMakerNote(rhs)
    {
        entries_ = rhs.entries_;
    }

    MinoltaMakerNote::AutoPtr MinoltaMakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    MinoltaMakerNote* MinoltaMakerNote::create_(bool alloc) const
    {
        return new MinoltaMakerNote(alloc);
    }

    MinoltaMakerNote::AutoPtr MinoltaMakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    MinoltaMakerNote* MinoltaMakerNote::clone_() const
    {
        return new MinoltaMakerNote(*this);
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createMinoltaMakerNote(bool alloc, const byte* /*buf*/, long /*len*/,
                                              ByteOrder /*byteOrder*/, long /*offset*/)
    {
        return MakerNote::AutoPtr(new MinoltaMakerNote(alloc));
    }

}                                       // namespace Exiv2
