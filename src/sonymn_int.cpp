// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
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
  File:      sonymn.cpp
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Apr-05, ahu: created
 */
// *****************************************************************************
// included header files
#include "types.hpp"
#include "minoltamn_int.hpp"
#include "sonymn_int.hpp"
#include "tags_int.hpp"
#include "tiffcomposite_int.hpp"
#include "value.hpp"
#include "i18n.h"                // NLS support.
#include <math.h>

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    // -- Standard Sony Makernotes tags ---------------------------------------------------------------

    //! Lookup table to translate Sony Auto HDR values to readable labels
    extern const TagDetails sonyHDRMode[] = {
        { 0x00000, N_("Off")   },
        { 0x10001, N_("Auto")  },
        { 0x10010, "1"         },
        { 0x10012, "2"         },
        { 0x10014, "3"         },
        { 0x10016, "4"         },
        { 0x10018, "5"         }
    };

    //! Lookup table to translate Sony model ID values to readable labels
    extern const TagDetails sonyModelId[] = {
        { 2, "DSC-R1"                   },
        { 256, "DSLR-A100"              },
        { 257, "DSLR-A900"              },
        { 258, "DSLR-A700"              },
        { 259, "DSLR-A200"              },
        { 260, "DSLR-A350"              },
        { 261, "DSLR-A300"              },
        { 262, "DSLR-A900 (APS-C mode)" },
        { 263, "DSLR-A380/A390"         },
        { 264, "DSLR-A330"              },
        { 265, "DSLR-A230"              },
        { 266, "DSLR-A290"              },
        { 269, "DSLR-A850"              },
        { 270, "DSLR-A850 (APS-C mode)" },
        { 273, "DSLR-A550"              },
        { 274, "DSLR-A500"              },
        { 275, "DSLR-A450"              },
        { 278, "NEX-5"                  },
        { 279, "NEX-3"                  },
        { 280, "SLT-A33"                },
        { 281, "SLT-A55 / SLT-A55V"     },
        { 282, "DSLR-A560"              },
        { 283, "DSLR-A580"              },
        { 284, "NEX-C3"                 },
        { 285, "SLT-A35"                },
        { 286, "SLT-A65 / SLT-A65V"     },
        { 287, "SLT-A77 / SLT-A77V"     },
        { 288, "NEX-5N"                 },
        { 289, "NEX-7"                  },
        { 290, "NEX-VG20E"              },
        { 291, "SLT-A37"                },
        { 292, "SLT-A57"                },
        { 293, "NEX-F3"                 },
        { 294, "SLT-A99 / SLT-A99V"     },
        { 295, "NEX-6"                  },
        { 296, "NEX-5R"                 },
        { 297, "DSC-RX100"              },
        { 298, "DSC-RX1"                },
        { 299, "NEX-VG900"              },
        { 300, "NEX-VG30E"              },
        { 302, "ILCE-3000 / ILCE-3500"  },
        { 303, "SLT-A58"                },
        { 305, "NEX-3N"                 },
        { 306, "ILCE-7"                 },
        { 307, "NEX-5T"                 },
        { 308, "DSC-RX100M2"            },
        { 309, "DSC-RX10"               },
        { 310, "DSC-RX1R"               },
        { 311, "ILCE-7R"                },
        { 312, "ILCE-6000"              },
        { 313, "ILCE-5000"              },
        { 317, "DSC-RX100M3"            },
        { 318, "ILCE-7S"                },
        { 319, "ILCA-77M2"              },
        { 339, "ILCE-5100"              },
        { 340, "ILCE-7M2"               },
        { 341, "DSC-RX100M4"            },
        { 342, "DSC-RX10M2"             },
        { 344, "DSC-RX1RM2"             },
        { 346, "ILCE-QX1"               },
        { 347, "ILCE-7RM2"              },
        { 350, "ILCE-7SM2"              },
        { 353, "ILCA-68"                },
        { 354, "ILCA-99M2"              },
        { 355, "DSC-RX10M3"             },
        { 356, "DSC-RX100M5"            },
        { 357, "ILCE-6300"              }
    };

    //! Lookup table to translate Sony dynamic range optimizer values to readable labels
    extern const TagDetails print0xb025[] = {
        { 0,  N_("Off")           },
        { 1,  N_("Standard")      },
        { 2,  N_("Advanced Auto") },
        { 3,  N_("Auto")          },
        { 8,  N_("Advanced Lv1")  },
        { 9,  N_("Advanced Lv2")  },
        { 10, N_("Advanced Lv3")  },
        { 11, N_("Advanced Lv4")  },
        { 12, N_("Advanced Lv5")  },
        { 16, "1"                 },
        { 17, "2"                 },
        { 18, "3"                 },
        { 19, "4"                 },
        { 20, "5"                 }
    };

    //! Lookup table to translate Sony exposure mode values to readable labels
    extern const TagDetails sonyExposureMode[] = {
        { 0,     N_("Auto")                     },
        { 1,     N_("Portrait")                 },
        { 2,     N_("Beach")                    },
        { 3,     N_("Sports")                   },
        { 4,     N_("Snow")                     },
        { 5,     N_("Landscape")                },
        { 6,     N_("Program")                  },
        { 7,     N_("Aperture priority")        },
        { 8,     N_("Shutter priority")         },
        { 9,     N_("Night Scene / Twilight")   },
        { 10,    N_("Hi-Speed Shutter")         },
        { 11,    N_("Twilight Portrait")        },
        { 12,    N_("Soft Snap / Portrait")     },
        { 13,    N_("Fireworks")                },
        { 14,    N_("Smile Shutter")            },
        { 15,    N_("Manual")                   },
        { 18,    N_("High Sensitivity")         },
        { 19,    N_("Macro")                    },
        { 20,    N_("Advanced Sports Shooting") },
        { 29,    N_("Underwater")               },
        { 33,    N_("Food")                     },
        { 34,    N_("Sweep Panorama")           },
        { 35,    N_("Handheld Night Shot")      },
        { 36,    N_("Anti Motion Blur")         },
        { 37,    N_("Pet")                      },
        { 38,    N_("Backlight Correction HDR") },
        { 39,    N_("Superior Auto")            },
        { 40,    N_("Background Defocus")       },
        { 41,    N_("Soft Skin")                },
        { 42,    N_("3D Image")                 },
        { 65535, N_("n/a")                      }
    };

    //! Lookup table to translate Sony JPEG Quality values to readable labels
    extern const TagDetails sonyJPEGQuality[] = {
        { 0,     N_("Normal")       },
        { 1,     N_("Fine")         },
        { 2,     N_("Extra Fine")   },
        { 65535, N_("n/a")          }
    };

    //! Lookup table to translate Sony anti-blur values to readable labels
    extern const TagDetails sonyAntiBlur[] = {
        { 0,     N_("Off")             },
        { 1,     N_("On (Continuous)") },
        { 2,     N_("On (Shooting)")   },
        { 65535, N_("n/a")             }
    };

    //! Lookup table to translate Sony dynamic range optimizer values to readable labels
    extern const TagDetails print0xb04f[] = {
        { 0, N_("Off")      },
        { 1, N_("Standard") },
        { 2, N_("Plus")     }
    };

    //! Lookup table to translate Sony Intelligent Auto values to readable labels
    extern const TagDetails sonyIntelligentAuto[] = {
        { 0, N_("Off")      },
        { 1, N_("On")       },
        { 2, N_("Advanced") }
    };

    //! Lookup table to translate Sony WB values to readable labels
    extern const TagDetails sonyWhiteBalance[] = {
        { 0,  N_("Auto")                       },
        { 4,  N_("Manual")                     },
        { 5,  N_("Daylight")                   },
        { 6,  N_("Cloudy")                     },
        { 7,  N_("White Fluorescent")          },
        { 8,  N_("Cool White Fluorescent")     },
        { 9,  N_("Day White Fluorescent")      },
        { 10, N_("Incandescent2")              },
        { 11, N_("Warm White Fluorescent")     },
        { 14, N_("Incandescent")               },
        { 15, N_("Flash")                      },
        { 17, N_("Underwater 1 (Blue Water)")  },
        { 18, N_("Underwater 2 (Green Water)") }
    };

    //! Lookup table to translate Sony AF mode values to readable labels
    extern const TagDetails sonyFocusMode[] = {
        { 1,     "AF-S"             },
        { 2,     "AF-C"             },
        { 4,     N_("Permanent-AF") },
        { 65535, N_("n/a")          }
    };

    //! Lookup table to translate Sony AF mode values to readable labels
    extern const TagDetails sonyAFMode[] = {
        { 0,     N_("Default")          },
        { 1,     N_("Multi AF")         },
        { 2,     N_("Center AF")        },
        { 3,     N_("Spot AF")          },
        { 4,     N_("Flexible Spot AF") },
        { 6,     N_("Touch AF")         },
        { 14,    N_("Manual Focus")     },
        { 15,    N_("Face Detected")    },
        { 65535, N_("n/a")              }
    };

    //! Lookup table to translate Sony AF illuminator values to readable labels
    extern const TagDetails sonyAFIlluminator[] = {
        { 0,     N_("Off") },
        { 1,     N_("Auto")  },
        { 65535, N_("n/a") }
    };

    //! Lookup table to translate Sony macro mode values to readable labels
    extern const TagDetails sonyMacroMode[] = {
        { 0,     N_("Off")         },
        { 1,     N_("On")          },
        { 2,     N_("Close Focus") },
        { 65535, N_("n/a")         }
    };

    //! Lookup table to translate Sony flash level values to readable labels
    extern const TagDetails sonyFlashLevel[] = {
        { -32768, N_("Low")    },
        { -1,     N_("n/a")    },
        { 0,      N_("Normal") },
        { 32767,  N_("High")   }
    };

    //! Lookup table to translate Sony release mode values to readable labels
    extern const TagDetails sonyReleaseMode[] = {
        { 0,     N_("Normal")                   },
        { 2,     N_("Burst")                    },
        { 5,     N_("Exposure Bracketing")      },
        { 6,     N_("White Balance Bracketing") },
        { 65535, N_("n/a")                      }
    };

	//! Lookup table to translate Sony releasemode2 values to readable labels
	extern const TagDetails sonyReleaseMode2[] = {
		{0, N_("Normal") },
		{1, N_("Continuous") },
		{2, N_("Continuous - Exposure Bracketing") },
		{3, N_("DRO or White Balance Bracketing") },
		{5, N_("Continuous - Burst") },
		{6, N_("Single Frame - Capture During Movie") },
		{7, N_("Continuous - Sweep Panorama") },
		{8, N_("Continuous - Anti-Motion Blur, Hand-held Twilight") },
		{9, N_("Continuous - HDR") },
		{10, N_("Continuous - Background defocus") },
		{13, N_("Continuous - 3D Sweep Panorama") },
		{15, N_("Continuous - High Resolution Sweep Panorama") },
		{16, N_("Continuous - 3D Image") },
		{17, N_("Continuous - Burst 2") },
		{19, N_("Continuous - Speed/Advance Priority") },
		{20, N_("Continuous - Multi Frame NR") },
		{23, N_("Single-frame - Exposure Bracketing") },
		{26, N_("Continuous Low") },
		{27, N_("Continuous - High Sensitivity") },
		{28, N_("Smile Shutter") },
		{29, N_("Continuous - Tele-zoom Advance Priority") },
		{146, N_("Single Frame - Movie Capture") }
	};

	//! Lookup table to translate Sony DynamicRangeOptimizer values to readable labels
	extern const TagDetails sonyDynamicRangeOptimizer[] = {
		{0, N_("Off") },
		{1, N_("Auto") },
		{3, N_("Lv1") },
		{4, N_("Lv2") },
		{5, N_("Lv3") },
		{6, N_("Lv4") },
		{7, N_("Lv5") },
		{8, N_("n/a") }
	};

	//! Lookup table to translate Sony release mode3 values to readable labels
	extern const TagDetails sonyReleaseMode3[] = {
		{0, N_("Normal") },
		{1, N_("Continuous") },
		{2, N_("Bracketing") },
		{4, N_("Continuous - Burst") },
		{5, N_("Continuous - Speed/Advance Priority") },
		{6, N_("Normal - Self-timer") },
		{9, N_("Single Burst Shooting") }
	};

	//! Lookup table to translate Sony SelfTimer values to readable labels
	extern const TagDetails sonySelfTimer[] = {
		{0, N_("Off") },
		{1, N_("Self-timer 10 s") },
		{2, N_("Self-timer 2 s") }
	};

	//! Lookup table to translate Sony FlashMode values to readable labels
	extern const TagDetails sonyFlashMode2010[] = {
		{0, N_("Autoflash") },
		{1, N_("Fill-flash") },
		{2, N_("Flash Off") },
		{3, N_("Slow Sync") },
		{4, N_("Rear Sync") },
		{6, N_("Wireless") }
	};

	//! Lookup table to translate Sony HDRSetting values to readable labels
	extern const TagDetails sonyHDRSetting[] = {
		{0, N_("Off") },
		{1, N_("HDR Auto") },
		{3, N_("HDR 1 EV") },
		{5, N_("HDR 2 EV") },
		{7, N_("HDR 3 EV") },
		{9, N_("HDR 4 EV") },
		{11, N_("HDR 5 EV") },
		{13, N_("HDR 6 EV") }
	};

	//! Lookup table to translate Sony PictureProfile values to readable labels
	extern const TagDetails sonyPictureProfile[] = {
		{0, N_("Gamma Still - Standard/Neutral (PP2)") },
		{1, N_("Gamma Still - Portrait") },
		{4, N_("Gamma Still - B&W/Sepia") },
		{5, N_("Gamma Still - Clear") },
		{6, N_("Gamma Still - Deep") },
		{7, N_("Gamma Still - Light") },
		{8, N_("Gamma Still - Vivid") },
		{9, N_("Gamma Still - Real") },
		{10, N_("Gamma Movie (PP1)") },
		{22, N_("Gamma ITU709 (PP3 or PP4)") },
		{24, N_("Gamma Cine1 (PP5)") },
		{25, N_("Gamma Cine2 (PP6)") },
		{26, N_("Gamma Cine3") },
		{27, N_("Gamma Cine4") },
		{28, N_("Gamma S-Log2 (PP7)") },
		{29, N_("Gamma ITU709 (800%)") },
		{31, N_("Gamma S-Log3 (PP8 or PP9)") },
		{33, N_("Gamma HLG2 (PP10)") }
	};

	//! Lookup table to translate Sony PictureEffect2 values to readable labels
	extern const TagDetails sonyPictureEffect2[] = {
		{0, N_("Off") },
		{1, N_("Toy Camera") },
		{2, N_("Pop Color") },
		{3, N_("Posterization") },
		{4, N_("Retro Photo") },
		{5, N_("Soft High Key") },
		{6, N_("Partial Color") },
		{7, N_("High Contrast Monochrome") },
		{8, N_("Soft Focus") },
		{9, N_("HDR Painting") },
		{10, N_("Rich-tone Monochrome") },
		{11, N_("Miniature") },
		{12, N_("Water Color") },
		{13, N_("Illustration") }
	};

	//! Lookup table to translate Sony Quality2 values to readable labels
	extern const TagDetails sonyQuality2[] = {
		{0, N_("JPEG") },
		{1, N_("RAW") },
		{2, N_("RAW + JPEG") }
	};

	//! Lookup table to translate Sony MeteringMode values to readable labels
	extern const TagDetails sonyMeteringMode2010[] = {
		{0, N_("Multi-segment") },
		{2, N_("Center-weighted average") },
		{3, N_("Spot") },
		{4, N_("Average") },
		{5, N_("Highlight") }
	};

	//! Lookup table to translate Sony ExposureProgram values to readable labels
	extern const TagDetails sonyExposureProgram2010[] = {
		{0, N_("Program AE") },
		{1, N_("Aperture-priority AE") },
		{2, N_("Shutter speed priority AE") },
		{3, N_("Manual") },
		{4, N_("Auto") },
		{5, N_("iAuto") },
		{6, N_("Superior Auto") },
		{7, N_("iAuto+") },
		{8, N_("Portrait") },
		{9, N_("Landscape") },
		{10, N_("Twilight") },
		{11, N_("Twilight Portrait") },
		{12, N_("Sunset") },
		{14, N_("Action (High speed)") },
		{16, N_("Sports") },
		{17, N_("Handheld Night Shot") },
		{18, N_("Anti Motion Blur") },
		{19, N_("High Sensitivity") },
		{21, N_("Beach") },
		{22, N_("Snow") },
		{23, N_("Fireworks") },
		{26, N_("Underwater") },
		{27, N_("Gourmet") },
		{28, N_("Pet") },
		{29, N_("Macro") },
		{30, N_("Backlight Correction HDR") },
		{33, N_("Sweep Panorama") },
		{36, N_("Background Defocus") },
		{37, N_("Soft Skin") },
		{42, N_("3D Image") },
		{43, N_("Cont. Priority AE") },
		{45, N_("Document") },
		{46, N_("Party") }
	};

	//! Lookup table to translate Sony LensFormat values to readable labels
	extern const TagDetails sonyLensFormat[] = {
		{0, N_("Unknown") },
		{1, N_("APS-C") },
		{2, N_("Full-frame") }
	};

	//! Lookup table to translate Sony LensMount values to readable labels
	extern const TagDetails sonyLensMount[] = {
		{0, N_("Unknown") },
		{1, N_("A-mount") },
		{2, N_("E-mount") }
	};

	extern const TagDetails sonyLensType2[] = {
			{    0, "Unknown E-mount lens or other lens"                        },
			{    0, "Sigma 19mm F2.8 [EX] DN"                                   }, // 1
			{    0, "Sigma 30mm F2.8 [EX] DN"                                   }, // 2
			{    0, "Sigma 60mm F2.8 DN"                                        }, // 3
			{    0, "Sony E 18-200mm F3.5-6.3 OSS LE"                           }, // 4
			{    0, "Tamron 18-200mm F3.5-6.3 Di III VC"                        }, // 5
			{    0, "Tokina FiRIN 20mm F2 FE AF"                                }, // 6
			{    0, "Tokina FiRIN 20mm F2 FE MF"                                }, // 7
			{    0, "Zeiss Touit 12mm F2.8"                                     }, // 8
			{    0, "Zeiss Touit 32mm F1.8"                                     }, // 9
			{    0, "Zeiss Touit 50mm F2.8 Macro"                               }, // 10
			{    0, "Zeiss Loxia 50mm F2"                                       }, // 11
			{    0, "Zeiss Loxia 35mm F2"                                       }, // 12
			{    1, "Sony LA-EA1 or Sigma MC-11 Adapter"                        },
			{    2, "Sony LA-EA2 Adapter"                                       },
			{    3, "Sony LA-EA3 Adapter"                                       },
			{    6, "Sony LA-EA4 Adapter"                                       },
			{   44, "Metabones Canon EF Smart Adapter"                          },
			{   78, "Metabones Canon EF Smart Adapter Mark III or Other Adapter"},
			{  184, "Metabones Canon EF Speed Booster Ultra"                    },
			{  234, "Metabones Canon EF Smart Adapter Mark IV"                  },
			{  239, "Metabones Canon EF Speed Booster"                          },
			{32784, "Sony E 16mm F2.8"                                          },
			{32785, "Sony E 18-55mm F3.5-5.6 OSS"                               },
			{32786, "Sony E 55-210mm F4.5-6.3 OSS"                              },
			{32787, "Sony E 18-200mm F3.5-6.3 OSS"                              },
			{32788, "Sony E 30mm F3.5 Macro"                                    },
			{32789, "Sony E 24mm F1.8 ZA or Samyang AF 50mm F1.4"               },
			{32789, "Samyang AF 50mm F1.4"                                      }, // 1
			{32790, "Sony E 50mm F1.8 OSS or Samyang AF 14mm F2.8"              },
			{32790, "Samyang AF 14mm F2.8"                                      }, // 1
			{32791, "Sony E 16-70mm F4 ZA OSS"                                  },
			{32792, "Sony E 10-18mm F4 OSS"                                     },
			{32793, "Sony E PZ 16-50mm F3.5-5.6 OSS"                            },
			{32794, "Sony FE 35mm F2.8 ZA or Samyang Lens"                      },
			{32794, "Samyang AF 24mm F2.8"                                      }, // 1
			{32794, "Samyang AF 35mm F2.8"                                      }, // 2
			{32795, "Sony FE 24-70mm F4 ZA OSS"                                 },
			{32796, "Sony FE 85mm F1.8 or Viltrox PFU RBMH 85mm F1.8"           },
			{32796, "Viltrox PFU RBMH 85mm F1.8"                                }, // 1
			{32797, "Sony E 18-200mm F3.5-6.3 OSS LE"                           },
			{32798, "Sony E 20mm F2.8"                                          },
			{32799, "Sony E 35mm F1.8 OSS"                                      },
			{32800, "Sony E PZ 18-105mm F4 G OSS"                               },
			{32801, "Sony FE 12-24mm F4 G"                                      },
			{32802, "Sony FE 90mm F2.8 Macro G OSS"                             },
			{32803, "Sony E 18-50mm F4-5.6"                                     },
			{32804, "Sony FE 24mm F1.4 GM"                                      },
			{32805, "Sony FE 24-105mm F4 G OSS"                                 },
			{32807, "Sony E PZ 18-200mm F3.5-6.3 OSS"                           },
			{32808, "Sony FE 55mm F1.8 ZA"                                      },
			{32810, "Sony FE 70-200mm F4 G OSS"                                 },
			{32811, "Sony FE 16-35mm F4 ZA OSS"                                 },
			{32812, "Sony FE 50mm F2.8 Macro"                                   },
			{32813, "Sony FE 28-70mm F3.5-5.6 OSS"                              },
			{32814, "Sony FE 35mm F1.4 ZA"                                      },
			{32815, "Sony FE 24-240mm F3.5-6.3 OSS"                             },
			{32816, "Sony FE 28mm F2"                                           },
			{32817, "Sony FE PZ 28-135mm F4 G OSS"                              },
			{32819, "Sony FE 100mm F2.8 STF GM OSS"                             },
			{32820, "Sony E PZ 18-110mm F4 G OSS"                               },
			{32821, "Sony FE 24-70mm F2.8 GM"                                   },
			{32822, "Sony FE 50mm F1.4 ZA"                                      },
			{32823, "Sony FE 85mm F1.4 GM or Samyang AF 85mm F1.4"              },
			{32823, "Samyang AF 85mm F1.4"                                      }, // 1
			{32824, "Sony FE 50mm F1.8"                                         },
			{32826, "Sony FE 21mm F2.8 (SEL28F20 + SEL075UWC)"                  },
			{32827, "Sony FE 16mm F3.5 Fisheye (SEL28F20 + SEL057FEC)"          },
			{32828, "Sony FE 70-300mm F4.5-5.6 G OSS"                           },
			{32829, "Sony FE 100-400mm F4.5-5.6 GM OSS"                         },
			{32830, "Sony FE 70-200mm F2.8 GM OSS"                              },
			{32831, "Sony FE 16-35mm F2.8 GM"                                   },
			{32848, "Sony FE 400mm F2.8 GM OSS"                                 },
			{32849, "Sony E 18-135mm F3.5-5.6 OSS"                              },
			{32850, "Sony FE 135mm F1.8 GM"                                     },
			{32851, "Sony FE 200-600mm F5.6-6.3 G OSS"                          },
			{32852, "Sony FE 600mm F4 GM OSS"                                   },
			{32853, "Sony E 16-55mm F2.8 G"                                     },
			{32854, "Sony E 70-350mm F4.5-6.3 G OSS"                            },
			{32858, "Sony FE 35mm F1.8"                                         },
			{33072, "Sony FE 70-200mm F2.8 GM OSS + 1.4X Teleconverter"         },
			{33073, "Sony FE 70-200mm F2.8 GM OSS + 2X Teleconverter"           },
			{33076, "Sony FE 100mm F2.8 STF GM OSS (macro mode)"                },
			{33077, "Sony FE 100-400mm F4.5-5.6 GM OSS + 1.4X Teleconverter"    },
			{33078, "Sony FE 100-400mm F4.5-5.6 GM OSS + 2X Teleconverter"      },
			{33079, "Sony FE 400mm F2.8 GM OSS + 1.4X Teleconverter"            },
			{33080, "Sony FE 400mm F2.8 GM OSS + 2X Teleconverter"              },
			{33081, "Sony FE 200-600mm F5.6-6.3 G OSS + 1.4X Teleconverter"     },
			{33082, "Sony FE 200-600mm F5.6-6.3 G OSS + 2X Teleconverter"       },
			{33083, "Sony FE 600mm F4 GM OSS + 1.4X Teleconverter"              },
			{33084, "Sony FE 600mm F4 GM OSS + 2X Teleconverter"                },
			{49201, "Zeiss Touit 12mm F2.8"                                     },
			{49202, "Zeiss Touit 32mm F1.8"                                     },
			{49203, "Zeiss Touit 50mm F2.8 Macro"                               },
			{49216, "Zeiss Batis 25mm F2"                                       },
			{49217, "Zeiss Batis 85mm F1.8"                                     },
			{49218, "Zeiss Batis 18mm F2.8"                                     },
			{49219, "Zeiss Batis 135mm F2.8"                                    },
			{49220, "Zeiss Batis 40mm F2 CF"                                    },
			{49232, "Zeiss Loxia 50mm F2"                                       },
			{49233, "Zeiss Loxia 35mm F2"                                       },
			{49234, "Zeiss Loxia 21mm F2.8"                                     },
			{49235, "Zeiss Loxia 85mm F2.4"                                     },
			{49236, "Zeiss Loxia 25mm F2.4"                                     },
			{49457, "Tamron 28-75mm F2.8 Di III RXD"                            },
			{49458, "Tamron 17-28mm F2.8 Di III RXD"                            },
			{49712, "Tokina FiRIN 20mm F2 FE AF"                                },
			{49713, "Tokina FiRIN 100mm F2.8 FE MACRO"                          },
			{50480, "Sigma 30mm F1.4 DC DN | C"                                 },
			{50481, "Sigma 50mm F1.4 DG HSM | A"                                },
			{50482, "Sigma 18-300mm F3.5-6.3 DC MACRO OS HSM | C + MC-11"       },
			{50483, "Sigma 18-35mm F1.8 DC HSM | A + MC-11"                     },
			{50484, "Sigma 24-35mm F2 DG HSM | A + MC-11"                       },
			{50485, "Sigma 24mm F1.4 DG HSM | A + MC-11"                        },
			{50486, "Sigma 150-600mm F5-6.3 DG OS HSM | C + MC-11"              },
			{50487, "Sigma 20mm F1.4 DG HSM | A + MC-11"                        },
			{50488, "Sigma 35mm F1.4 DG HSM | A"                                },
			{50489, "Sigma 150-600mm F5-6.3 DG OS HSM | S + MC-11"              },
			{50490, "Sigma 120-300mm F2.8 DG OS HSM | S + MC-11"                },
			{50492, "Sigma 24-105mm F4 DG OS HSM | A + MC-11"                   },
			{50493, "Sigma 17-70mm F2.8-4 DC MACRO OS HSM | C + MC-11"          },
			{50495, "Sigma 50-100mm F1.8 DC HSM | A + MC-11"                    },
			{50499, "Sigma 85mm F1.4 DG HSM | A"                                },
			{50501, "Sigma 100-400mm F5-6.3 DG OS HSM | C + MC-11"              },
			{50503, "Sigma 16mm F1.4 DC DN | C"                                 },
			{50507, "Sigma 105mm F1.4 DG HSM | A"                               },
			{50508, "Sigma 56mm F1.4 DC DN | C"                                 },
			{50512, "Sigma 70-200mm F2.8 DG OS HSM | S + MC-11"                 },
			{50513, "Sigma 70mm F2.8 DG MACRO | A"                              },
			{50514, "Sigma 45mm F2.8 DG DN | C"                                 },
			{50515, "Sigma 35mm F1.2 DG DN | A"                                 },
			{50516, "Sigma 14-24mm F2.8 DG DN | A"                              },
			{50992, "Voigtlander SUPER WIDE-HELIAR 15mm F4.5 III"               },
			{50993, "Voigtlander HELIAR-HYPER WIDE 10mm F5.6"                   },
			{50994, "Voigtlander ULTRA WIDE-HELIAR 12mm F5.6 III"               },
			{50995, "Voigtlander MACRO APO-LANTHAR 65mm F2 Aspherical"          },
			{50996, "Voigtlander NOKTON 40mm F1.2 Aspherical"                   },
			{50997, "Voigtlander NOKTON classic 35mm F1.4"                      },
			{50998, "Voigtlander MACRO APO-LANTHAR 110mm F2.5"                  },
			{50999, "Voigtlander COLOR-SKOPAR 21mm F3.5 Aspherical"             },
			{51000, "Voigtlander NOKTON 50mm F1.2 Aspherical"                   },
			{51001, "Voigtlander NOKTON 21mm F1.4 Aspherical"                   },
			{51504, "Samyang AF 50mm F1.4"                                      },
			{51505, "Samyang AF 14mm F2.8 or Samyang AF 35mm F2.8"              },
			{51505, "Samyang AF 35mm F2.8"                                      }, // 1
			{51507, "Samyang AF 35mm F1.4"                                      }
	};

	//! Lookup table to translate Sony DistortionCorrParamsPresent values to readable labels
	extern const TagDetails sonyDistortionCorrParamsPresent[] = {
		{0, N_("No") },
		{ 1, N_("Yes") }
	};

	//! Lookup table to translate Sony sequence number values to readable labels
    extern const TagDetails sonySequenceNumber[] = {
        { 0,     N_("Single")                    },
        { 65535, N_("n/a")                       }
    };

    //! Lookup table to translate Sony long exposure noise reduction values to readable labels
    extern const TagDetails sonyLongExposureNoiseReduction[] = {
        { 0,     N_("Off") },
        { 1,     N_("On")  },
        { 65535, N_("n/a") }
    };

    std::ostream& SonyMakerNote::print0xb000(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 4)
        {
            os << "(" << value << ")";
        }
        else
        {
            std::string val = value.toString(0) + value.toString(1) + value.toString(2) + value.toString(3);
            if      (val == "0002") os << "JPEG";
            else if (val == "1000") os << "SR2";
            else if (val == "2000") os << "ARW 1.0";
            else if (val == "3000") os << "ARW 2.0";
            else if (val == "3100") os << "ARW 2.1";
            else if (val == "3200") os << "ARW 2.2";
            else if (val == "3300") os << "ARW 2.3";
            else if (val == "3310") os << "ARW 2.3.1";
            else if (val == "3320") os << "ARW 2.3.2";
            else                    os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& SonyMakerNote::printImageSize(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() == 2)
            os << value.toString(0) << " x " << value.toString(1);
        else
            os << "(" << value << ")";

        return os;
    }

    // Sony MakerNote Tag Info
    const TagInfo SonyMakerNote::tagInfo_[] = {
        TagInfo(0x0102, "Quality", N_("Image Quality"),
                N_("Image quality"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyImageQuality),
        TagInfo(0x0104, "FlashExposureComp", N_("Flash Exposure Compensation"),
                N_("Flash exposure compensation in EV"),
                sony1Id, makerTags, signedRational, -1, print0x9204),
        TagInfo(0x0105, "Teleconverter", N_("Teleconverter Model"),
                N_("Teleconverter Model"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyTeleconverterModel),
        TagInfo(0x0112, "WhiteBalanceFineTune", N_("White Balance Fine Tune"),
                N_("White Balance Fine Tune Value"),
                sony1Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x0114, "CameraSettings", N_("Camera Settings"),
                N_("Camera Settings"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0x0115, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyWhiteBalanceStd),
        TagInfo(0x0116, "0x0116", "0x0116",
                N_("Unknown"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0x0E00, "PrintIM", N_("Print IM"),
                N_("PrintIM information"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0x1000, "MultiBurstMode", N_("Multi Burst Mode"),
                N_("Multi Burst Mode"),
                sony1Id, makerTags, undefined, -1, printMinoltaSonyBoolValue),
        TagInfo(0x1001, "MultiBurstImageWidth", N_("Multi Burst Image Width"),
                N_("Multi Burst Image Width"),
                sony1Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x1002, "MultiBurstImageHeight", N_("Multi Burst Image Height"),
                N_("Multi Burst Image Height"),
                sony1Id, makerTags, unsignedShort, -1, printValue),
        // TODO : Implement Panorama tags decoding.
        TagInfo(0x1003, "Panorama", N_("Panorama"),
                N_("Panorama"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0x2000, "0x2000", "0x2000",
                N_("Unknown"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0x2001, "PreviewImage", N_("Preview Image"),
                N_("JPEG preview image"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0x2002, "0x2002", "0x2002",
                N_("Unknown"),
                sony1Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x2003, "0x2003", "0x2003",
                N_("Unknown"),
                sony1Id, makerTags, asciiString, -1, printValue),
        TagInfo(0x2004, "Contrast", "Contrast",
                N_("Contrast"),
                sony1Id, makerTags, signedLong, -1, printValue),
        TagInfo(0x2005, "Saturation", "Saturation",
                N_("Saturation"),
                sony1Id, makerTags, signedLong, -1, printValue),
        TagInfo(0x2006, "0x2006", "0x2006",
                N_("Unknown"),
                sony1Id, makerTags, signedLong, -1, printValue),
        TagInfo(0x2007, "0x2007", "0x2007",
                N_("Unknown"),
                sony1Id, makerTags, signedLong, -1, printValue),
        TagInfo(0x2008, "0x2008", "0x2008",
                N_("Unknown"),
                sony1Id, makerTags, signedLong, -1, printValue),
        TagInfo(0x2009, "0x2009", "0x2009",
                N_("Unknown"),
                sony1Id, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x200A, "AutoHDR", N_("Auto HDR"),
                N_("High Definition Range Mode"),
                sony1Id, makerTags, unsignedLong, -1, EXV_PRINT_TAG(sonyHDRMode)),
        // TODO : Implement Shot Info tags decoding.
        TagInfo(0x3000, "ShotInfo", N_("Shot Info"),
                N_("Shot Information"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0xB000, "FileFormat", N_("File Format"),
                N_("File Format"),
                sony1Id, makerTags, unsignedByte, -1, print0xb000),
        TagInfo(0xB001, "SonyModelID", N_("Sony Model ID"),
                N_("Sony Model ID"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyModelId)),
        TagInfo(0xB020, "ColorReproduction", N_("Color Reproduction"),
                N_("Color Reproduction"),
                sony1Id, makerTags, asciiString, -1, printValue),
        TagInfo(0xb021, "ColorTemperature", N_("Color Temperature"),
                N_("Color Temperature"),
                sony1Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xB022, "ColorCompensationFilter", N_("Color Compensation Filter"),
                N_("Color Compensation Filter: negative is green, positive is magenta"),
                sony1Id, makerTags, unsignedLong, -1, printValue),
        TagInfo(0xB023, "SceneMode", N_("Scene Mode"),
                N_("Scene Mode"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonySceneMode),
        TagInfo(0xB024, "ZoneMatching", N_("Zone Matching"),
                N_("Zone Matching"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyZoneMatching),
        TagInfo(0xB025, "DynamicRangeOptimizer", N_("Dynamic Range Optimizer"),
                N_("Dynamic Range Optimizer"),
                sony1Id, makerTags, unsignedLong, -1, EXV_PRINT_TAG(print0xb025)),
        TagInfo(0xB026, "ImageStabilization", N_("Image Stabilization"),
                N_("Image stabilization"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyBoolValue),
        TagInfo(0xB027, "LensID", N_("Lens ID"),
                N_("Lens identifier"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyLensID),
        TagInfo(0xB028, "MinoltaMakerNote", N_("Minolta MakerNote"),
                N_("Minolta MakerNote"),
                sony1Id, makerTags, undefined, -1, printValue),
        TagInfo(0xB029, "ColorMode", N_("Color Mode"),
                N_("Color Mode"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyColorMode),
        TagInfo(0xB02B, "FullImageSize", N_("Full Image Size"),
                N_("Full Image Size"),
                sony1Id, makerTags, unsignedLong, -1, printImageSize),
        TagInfo(0xB02C, "PreviewImageSize", N_("Preview Image Size"),
                N_("Preview image size"),
                sony1Id, makerTags, unsignedLong, -1, printImageSize),
        TagInfo(0xB040, "Macro", N_("Macro"),
                N_("Macro"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyMacroMode)),
        TagInfo(0xB041, "ExposureMode", N_("Exposure Mode"),
                N_("Exposure Mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyExposureMode)),
        TagInfo(0xB042, "FocusMode", N_("Focus Mode"),
                N_("Focus Mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyFocusMode)),
        TagInfo(0xB043, "AFMode", N_("AF Mode"),
                N_("AF Mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyAFMode)),
        TagInfo(0xB044, "AFIlluminator", N_("AF Illuminator"),
                N_("AF Illuminator"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyAFIlluminator)),
        TagInfo(0xB047, "JPEGQuality", N_("JPEG Quality"),
                N_("JPEG Quality"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyJPEGQuality)),
        TagInfo(0xB048, "FlashLevel", N_("Flash Level"),
                N_("Flash Level"),
                sony1Id, makerTags, signedShort, -1, EXV_PRINT_TAG(sonyFlashLevel)),
        TagInfo(0xB049, "ReleaseMode", N_("Release Mode"),
                N_("Release Mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyReleaseMode)),
        TagInfo(0xB04A, "SequenceNumber", N_("Sequence Number"),
                N_("Shot number in continuous burst mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonySequenceNumber)),
        TagInfo(0xB04B, "AntiBlur", N_("Anti-Blur"),
                N_("Anti-Blur"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyAntiBlur)),
        TagInfo(0xB04E, "LongExposureNoiseReduction", N_("Long Exposure Noise Reduction"),
                N_("Long Exposure Noise Reduction"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyLongExposureNoiseReduction)),
        TagInfo(0xB04F, "DynamicRangeOptimizer", N_("Dynamic Range Optimizer"),
                N_("Dynamic Range Optimizer"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(print0xb04f)),
        TagInfo(0xB052, "IntelligentAuto", N_("Intelligent Auto"),
                N_("Intelligent Auto"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyIntelligentAuto)),
        TagInfo(0xB054, "WhiteBalance2", N_("White Balance 2"),
                N_("White balance 2"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyWhiteBalance)),
        // End of list marker
        TagInfo(0xffff, "(UnknownSony1MakerNoteTag)", "(UnknownSony1MakerNoteTag)",
                N_("Unknown Sony1MakerNote tag"),
                sony1Id, makerTags, asciiString, -1, printValue)
    };

    const TagInfo* SonyMakerNote::tagList()
    {
        return tagInfo_;
    }

    // -- Sony camera settings ---------------------------------------------------------------

    //! Lookup table to translate Sony camera settings drive mode values to readable labels
    extern const TagDetails sonyDriveModeStd[] = {
        { 0x01, N_("Single Frame")                      },
        { 0x02, N_("Continuous High")                   },
        { 0x04, N_("Self-timer 10 sec")                 },
        { 0x05, N_("Self-timer 2 sec, Mirror Lock-up")  },
        { 0x06, N_("Single-frame Bracketing")           },
        { 0x07, N_("Continuous Bracketing")             },
        { 0x0a, N_("Remote Commander")                  },
        { 0x0b, N_("Mirror Lock-up")                    },
        { 0x12, N_("Continuous Low")                    },
        { 0x18, N_("White Balance Bracketing Low")      },
        { 0x19, N_("D-Range Optimizer Bracketing Low")  },
        { 0x28, N_("White Balance Bracketing High")     },
        { 0x29, N_("D-Range Optimizer Bracketing High") },
        { 0x29, N_("D-Range Optimizer Bracketing High") } // To silence compiler warning
    };

    //! Lookup table to translate Sony camera settings focus mode values to readable labels
    extern const TagDetails sonyCSFocusMode[] = {
        { 0, N_("Manual") },
        { 1, "AF-S"       },
        { 2, "AF-C"       },
        { 3, "AF-A"       }
    };

    //! Lookup table to translate Sony camera settings metering mode values to readable labels
    extern const TagDetails sonyMeteringMode[] = {
        { 1, N_("Multi-segment")           },
        { 2, N_("Center weighted average") },
        { 4, N_("Spot")                    }
    };

    //! Lookup table to translate Sony camera settings creative style values to readable labels
    extern const TagDetails sonyCreativeStyle[] = {
        { 1,    N_("Standard")            },
        { 2,    N_("Vivid")               },
        { 3,    N_("Portrait")            },
        { 4,    N_("Landscape")           },
        { 5,    N_("Sunset")              },
        { 6,    N_("Night View/Portrait") },
        { 8,    N_("Black & White")       },
        { 9,    N_("Adobe RGB")           },
        { 11,   N_("Neutral")             },
        { 12,   N_("Clear")               },
        { 13,   N_("Deep")                },
        { 14,   N_("Light")               },
        { 15,   N_("Autumn")              },
        { 16,   N_("Sepia")               }
    };

    //! Lookup table to translate Sony camera settings flash mode values to readable labels
    extern const TagDetails sonyFlashMode[] = {
        { 0, N_("ADI") },
        { 1, N_("TTL") },
    };

    //! Lookup table to translate Sony AF illuminator values to readable labels
    extern const TagDetails sonyAFIlluminatorCS[] = {
        { 0, N_("Auto") },
        { 1, N_("Off")  }
    };

    //! Lookup table to translate Sony camera settings image style values to readable labels
    extern const TagDetails sonyImageStyle[] = {
        { 1,    N_("Standard")            },
        { 2,    N_("Vivid")               },
        { 3,    N_("Portrait")            },
        { 4,    N_("Landscape")           },
        { 5,    N_("Sunset")              },
        { 7,    N_("Night View/Portrait") },
        { 8,    N_("B&W")                 },
        { 9,    N_("Adobe RGB")           },
        { 11,   N_("Neutral")             },
        { 129,  N_("StyleBox1")           },
        { 130,  N_("StyleBox2")           },
        { 131,  N_("StyleBox3")           },
        { 132,  N_("StyleBox4")           },
        { 133,  N_("StyleBox5")           },
        { 134,  N_("StyleBox6")           }
    };

    //! Lookup table to translate Sony camera settings exposure program values to readable labels
    extern const TagDetails sonyExposureProgram[] = {
        { 0,    N_("Auto")                      },
        { 1,    N_("Manual")                    },
        { 2,    N_("Program AE")                },
        { 3,    N_("Aperture-priority AE")      },
        { 4,    N_("Shutter speed priority AE") },
        { 8,    N_("Program Shift A")           },
        { 9,    N_("Program Shift S")           },
        { 16,   N_("Portrait")                  },
        { 17,   N_("Sports")                    },
        { 18,   N_("Sunset")                    },
        { 19,   N_("Night Portrait")            },
        { 20,   N_("Landscape")                 },
        { 21,   N_("Macro")                     },
        { 35,   N_("Auto No Flash")             }
    };

    //! Lookup table to translate Sony camera settings image size values to readable labels
    extern const TagDetails sonyImageSize[] = {
        { 1, N_("Large")  },
        { 2, N_("Medium") },
        { 3, N_("Small")  }
    };

    //! Lookup table to translate Sony aspect ratio values to readable labels
    extern const TagDetails sonyAspectRatio[] = {
        { 1, "3:2"   },
        { 2, "16:9"  }
    };

    //! Lookup table to translate Sony exposure level increments values to readable labels
    extern const TagDetails sonyExposureLevelIncrements[] = {
        { 33, "1/3 EV" },
        { 50, "1/2 EV" }
    };

    // Sony Camera Settings Tag Info
    // NOTE: all are for A200, A230, A300, A350, A700, A850 and A900 Sony model excepted
    // some entries which are only relevant with A700.

    // Warnings: Exiftool database give a list of tags shorted in decimal mode, not hexadecimal.

    const TagInfo SonyMakerNote::tagInfoCs_[] = {
        // NOTE: A700 only
        TagInfo(0x0004, "DriveMode", N_("Drive Mode"),
                N_("Drive Mode"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyDriveModeStd)),
        // NOTE: A700 only
        TagInfo(0x0006, "WhiteBalanceFineTune", N_("White Balance Fine Tune"),
                N_("White Balance Fine Tune"),
                sony1CsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0010, "FocusMode", N_("Focus Mode"),
                N_("Focus Mode"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCSFocusMode)),
        TagInfo(0x0011, "AFAreaMode", N_("AF Area Mode"),
                N_("AF Area Mode"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyAFAreaMode),
        TagInfo(0x0012, "LocalAFAreaPoint", N_("Local AF Area Point"),
                N_("Local AF Area Point"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyLocalAFAreaPoint),
        TagInfo(0x0015, "MeteringMode", N_("Metering Mode"),
                N_("Metering Mode"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyMeteringMode)),
        TagInfo(0x0016, "ISOSetting", N_("ISO Setting"),
                N_("ISO Setting"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0018, "DynamicRangeOptimizerMode", N_("Dynamic Range Optimizer Mode"),
                N_("Dynamic Range Optimizer Mode"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyDynamicRangeOptimizerMode),
        TagInfo(0x0019, "DynamicRangeOptimizerLevel", N_("Dynamic Range Optimizer Level"),
                N_("Dynamic Range Optimizer Level"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001A, "CreativeStyle", N_("Creative Style"),
                N_("Creative Style"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCreativeStyle)),
        TagInfo(0x001C, "Sharpness", N_("Sharpness"),
                N_("Sharpness"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001D, "Contrast", N_("Contrast"),
                N_("Contrast"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001E, "Saturation", N_("Saturation"),
                N_("Saturation"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001F, "ZoneMatchingValue", N_("Zone Matching Value"),
                N_("Zone Matching Value"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0022, "Brightness", N_("Brightness"),
                N_("Brightness"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0023, "FlashMode", N_("FlashMode"),
                N_("FlashMode"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFlashMode)),
        // NOTE: A700 only
        TagInfo(0x0028, "PrioritySetupShutterRelease", N_("Priority Setup Shutter Release"),
                N_("Priority Setup Shutter Release"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyPrioritySetupShutterRelease),
        // NOTE: A700 only
        TagInfo(0x0029, "AFIlluminator", N_("AF Illuminator"),
                N_("AF Illuminator"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyAFIlluminatorCS)),
        // NOTE: A700 only
        TagInfo(0x002A, "AFWithShutter", N_("AF With Shutter"),
                N_("AF With Shutter"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyBoolInverseValue),
        // NOTE: A700 only
        TagInfo(0x002B, "LongExposureNoiseReduction", N_("Long Exposure Noise Reduction"),
                N_("Long Exposure Noise Reduction"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue),
        // NOTE: A700 only
        TagInfo(0x002C, "HighISONoiseReduction", N_("High ISO NoiseReduction"),
                N_("High ISO NoiseReduction"),
                sony1CsId, makerTags, unsignedShort, 1, printValue),
        // NOTE: A700 only
        TagInfo(0x002D, "ImageStyle", N_("Image Style"),
                N_("Image Style"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyImageStyle)),
        TagInfo(0x003C, "ExposureProgram", N_("Exposure Program"),
                N_("Exposure Program"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureProgram)),
        TagInfo(0x003D, "ImageStabilization", N_("Image Stabilization"),
                N_("Image Stabilization"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue),
        TagInfo(0x003F, "Rotation", N_("Rotation"),
                N_("Rotation"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyRotation),
        TagInfo(0x0054, "SonyImageSize", N_("Sony Image Size"),
                N_("Sony Image Size"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyImageSize)),
        TagInfo(0x0055, "AspectRatio", N_("Aspect Ratio"),
                N_("Aspect Ratio"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyAspectRatio)),
        TagInfo(0x0056, "Quality", N_("Quality"),
                N_("Quality"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyQualityCs),
        TagInfo(0x0058, "ExposureLevelIncrements", N_("Exposure Level Increments"),
                N_("Exposure Level Increments"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureLevelIncrements)),
        // End of list marker
        TagInfo(0xffff, "(UnknownSony1CsTag)", "(UnknownSony1CsTag)",
                N_("Unknown Sony1 Camera Settings tag"),
                sony1CsId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* SonyMakerNote::tagListCs()
    {
        return tagInfoCs_;
    }

    // -- Sony camera settings 2 ---------------------------------------------------------------

    // Sony Camera Settings Tag Version 2 Info
    // NOTE: for A330, A380, A450, A500, A550 Sony model

    // Warnings: Exiftool database give a list of tags shorted in decimal mode, not hexadecimal.

    const TagInfo SonyMakerNote::tagInfoCs2_[] = {
        TagInfo(0x0010, "FocusMode", N_("Focus Mode"),
                N_("Focus Mode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCSFocusMode)),
        TagInfo(0x0011, "AFAreaMode", N_("AF Area Mode"),
                N_("AF Area Mode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printMinoltaSonyAFAreaMode),
        TagInfo(0x0012, "LocalAFAreaPoint", N_("Local AF Area Point"),
                N_("Local AF Area Point"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printMinoltaSonyLocalAFAreaPoint),
        TagInfo(0x0013, "MeteringMode", N_("Metering Mode"),
                N_("Metering Mode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyMeteringMode)),
        TagInfo(0x0014, "ISOSetting", N_("ISO Setting"),
                N_("ISO Setting"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0016, "DynamicRangeOptimizerMode", N_("Dynamic Range Optimizer Mode"),
                N_("Dynamic Range Optimizer Mode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printMinoltaSonyDynamicRangeOptimizerMode),
        TagInfo(0x0017, "DynamicRangeOptimizerLevel", N_("Dynamic Range Optimizer Level"),
                N_("Dynamic Range Optimizer Level"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0018, "CreativeStyle", N_("Creative Style"),
                N_("Creative Style"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCreativeStyle)),
        TagInfo(0x0019, "Sharpness", N_("Sharpness"),
                N_("Sharpness"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001A, "Contrast", N_("Contrast"),
                N_("Contrast"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001B, "Saturation", N_("Saturation"),
                N_("Saturation"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0023, "FlashMode", N_("FlashMode"),
                N_("FlashMode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFlashMode)),
        TagInfo(0x003C, "ExposureProgram", N_("Exposure Program"),
                N_("Exposure Program"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureProgram)),
        TagInfo(0x003F, "Rotation", N_("Rotation"),
                N_("Rotation"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printMinoltaSonyRotation),
        TagInfo(0x0054, "SonyImageSize", N_("Sony Image Size"),
                N_("Sony Image Size"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyImageSize)),
        // End of list marker
        TagInfo(0xffff, "(UnknownSony1Cs2Tag)", "(UnknownSony1Cs2Tag)",
                N_("Unknown Sony1 Camera Settings 2 tag"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* SonyMakerNote::tagListCs2()
    {
        return tagInfoCs2_;
    }

    //! Sony Tag 9402 Sony2Fp (FocusPosition)
    const TagInfo SonyMakerNote::tagInfoFp_[] = {
        TagInfo(  0x04, "AmbientTemperature", N_("Ambient Temperature"), N_("Ambient Temperature"), sony2FpId, makerTags,   signedByte, 1, printValue),
        TagInfo(  0x16, "FocusMode"         , N_("Focus Mode")         , N_("Focus Mode")         , sony2FpId, makerTags, unsignedByte, 1, printValue),
        TagInfo(  0x17, "AFAreaMode"        , N_("AF Area Mode")       , N_("AF Area Mode")       , sony2FpId, makerTags, unsignedByte, 1, printValue),
        TagInfo(  0x2d, "FocusPosition2"    , N_("Focus Position 2")   , N_("Focus Position 2")   , sony2FpId, makerTags, unsignedByte, 1, printValue),
        // End of list marker
        TagInfo(0xffff, "(Unknownsony2FpTag)", "(Unknownsony2FpTag)"   , "(Unknownsony2FpTag)"    , sony2FpId, makerTags, unsignedByte, 1, printValue)
    };

    const TagInfo* SonyMakerNote::tagListFp()
    {
        return tagInfoFp_;
    }

	std::ostream& printValuePlusOne(std::ostream& os, const Value& value, const ExifData*)
	{
		return os << (value.toLong() + 1);
	}

	std::ostream& printValueBy16(std::ostream& os, const Value& value, const ExifData*)
	{
		return os << (value.toFloat()/16);
	}

	std::ostream& printStopsAboveBaseISO(std::ostream& os, const Value& value, const ExifData*)
	{
		return os << static_cast<long>(16 - ((value.toFloat()/256)));
	}

	std::ostream& printBrightness(std::ostream& os, const Value& value, const ExifData*)
	{
		return os << static_cast<long>((value.toFloat() / 256) - 56.6);
	}

	std::ostream& printExposureCompensation(std::ostream& os, const Value& value, const ExifData*)
	{
		return os << static_cast<long>(-(value.toFloat() / 256));
	}

	std::ostream& printSonyISO(std::ostream& os, const Value& value, const ExifData*)
	{
		return os << static_cast<long>(100 * pow((16 - (value.toFloat() / 256)), 2));
	}

	std::ostream& printValueBy10(std::ostream& os, const Value& value, const ExifData*)
	{
		return os << static_cast<long>(value.toFloat() / 10);
	}

	//! Sony Tag 2010e Sony2010e (Miscellaneous)
	const TagInfo SonyMakerNote::tagInfo2010e_[] = {
		TagInfo(0, "SequenceImageNumber", N_("Sequence Image Number"), N_("Sequence Image Number"), sony2010eId, makerTags, unsignedLong, 1, printValuePlusOne),
		TagInfo(4, "SequenceFileNumber", N_("SequenceFileNumber"), N_("SequenceFileNumber"), sony2010eId, makerTags, unsignedLong, 1, printValuePlusOne),
		TagInfo(8, "ReleaseMode2", N_("ReleaseMode2"), N_("ReleaseMode2"), sony2010eId, makerTags, unsignedLong, 1, EXV_PRINT_TAG(sonyReleaseMode2)),
		TagInfo(540, "DigitalZoomRatio", N_("DigitalZoomRatio"), N_("DigitalZoomRatio"), sony2010eId, makerTags, unsignedByte, 1, printValueBy16),
		TagInfo(556, "SonyDateTime", N_("SonyDateTime"), N_("SonyDateTime"), sony2010eId, makerTags, undefined, 1, printValue),
		TagInfo(808, "DynamicRangeOptimizer", N_("DynamicRangeOptimizer"), N_("DynamicRangeOptimizer"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyDynamicRangeOptimizer)),
		TagInfo(1208, "MeterInfo", N_("MeterInfo"), N_("MeterInfo"), sony2010eId, makerTags, undefined, 1, printValue),
		TagInfo(4444, "ReleaseMode3", N_("ReleaseMode3"), N_("ReleaseMode3"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyReleaseMode3)),
		TagInfo(4448, "ReleaseMode2", N_("ReleaseMode2"), N_("ReleaseMode2"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyReleaseMode2)),
		TagInfo(4456, "SelfTimer", N_("SelfTimer"), N_("SelfTimer"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonySelfTimer)),
		TagInfo(4460, "FlashMode", N_("FlashMode"), N_("FlashMode"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyFlashMode2010)),
		TagInfo(4466, "StopsAboveBaseISO", N_("StopsAboveBaseISO"), N_("StopsAboveBaseISO"), sony2010eId, makerTags, unsignedShort, 1, printStopsAboveBaseISO),
		TagInfo(4468, "BrightnessValue", N_("BrightnessValue"), N_("BrightnessValue"), sony2010eId, makerTags, unsignedShort, 1, printBrightness),
		TagInfo(4472, "DynamicRangeOptimizer", N_("DynamicRangeOptimizer"), N_("DynamicRangeOptimizer"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyDynamicRangeOptimizer)),
		TagInfo(4476, "HDRSetting", N_("HDRSetting"), N_("HDRSetting"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyHDRSetting)),
		TagInfo(4480, "ExposureCompensation", N_("ExposureCompensation"), N_("ExposureCompensation"), sony2010eId, makerTags, signedShort, 1, printExposureCompensation),
		TagInfo(4502, "PictureProfile", N_("PictureProfile"), N_("PictureProfile"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyPictureProfile)),
		TagInfo(4503, "PictureProfile2", N_("PictureProfile2"), N_("PictureProfile2"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyPictureProfile)),
		TagInfo(4507, "PictureEffect2", N_("PictureEffect2"), N_("PictureEffect2"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyPictureEffect2)),
		TagInfo(4520, "Quality2", N_("Quality2"), N_("Quality2"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyQuality2)),
		TagInfo(4524, "MeteringMode", N_("MeteringMode"), N_("MeteringMode"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyMeteringMode2010)),
		TagInfo(4525, "ExposureProgram", N_("ExposureProgram"), N_("ExposureProgram"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyExposureProgram2010)),
		TagInfo(4532, "WB_RGBLevels", N_("WB_RGBLevels"), N_("WB_RGBLevels"), sony2010eId, makerTags, unsignedShort, 3, printValue),
		TagInfo(4692, "SonyISO", N_("SonyISO"), N_("SonyISO"), sony2010eId, makerTags, unsignedShort, 1, printSonyISO),
		TagInfo(4696, "SonyISO2", N_("SonyISO2"), N_("SonyISO2"), sony2010eId, makerTags, unsignedShort, 1, printSonyISO),
		TagInfo(4728, "FocalLength", N_("FocalLength"), N_("FocalLength"), sony2010eId, makerTags, unsignedShort, 1, printValueBy10),
		TagInfo(4730, "MinFocalLength", N_("MinFocalLength"), N_("MinFocalLength"), sony2010eId, makerTags, unsignedShort, 1, printValueBy10),
		TagInfo(4732, "MaxFocalLength", N_("MaxFocalLength"), N_("MaxFocalLength"), sony2010eId, makerTags, unsignedShort, 1, printValueBy10),
		TagInfo(4736, "SonyISO3", N_("SonyISO3"), N_("SonyISO3"), sony2010eId, makerTags, unsignedShort, 1, printSonyISO),
		TagInfo(6256, "DistortionCorrParams", N_("DistortionCorrParams"), N_("DistortionCorrParams"), sony2010eId, makerTags, signedShort, 16, printValue),
		TagInfo(6289, "LensFormat", N_("LensFormat"), N_("LensFormat"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyLensFormat)),
		TagInfo(6290, "LensMount", N_("LensMount"), N_("LensMount"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyLensMount)),
		TagInfo(6291, "LensType2", N_("LensType2"), N_("LensType2"), sony2010eId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyLensType2)),
		TagInfo(6294, "LensType", N_("LensType"), N_("LensType"), sony2010eId, makerTags, unsignedShort, 1, printMinoltaSonyLensID),
		TagInfo(6296, "DistortionCorrParamsPresent", N_("DistortionCorrParamsPresent"), N_("DistortionCorrParamsPresent"), sony2010eId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sonyDistortionCorrParamsPresent)),
		TagInfo(6297, "DistortionCorrParamsNumber", N_("DistortionCorrParamsNumber"), N_("DistortionCorrParamsNumber"), sony2010eId, makerTags, unsignedByte, 1, printValue),
		// End of list marker
		TagInfo(0xffff, "(UnknownSony2010eTag)", "(UnknownSony2010eTag)"   , "(UnknownSony2010eTag)"    , sony2010eId, makerTags, unsignedByte, 1, printValue)
	};

	const TagInfo* SonyMakerNote::tagList2010e()
	{
		return tagInfo2010e_;
	}

	// https://github.com/Exiv2/exiv2/pull/906#issuecomment-504338797
    static DataBuf sonyTagCipher(uint16_t /* tag */, const byte* bytes, uint32_t size, TiffComponent* const /*object*/, bool bDecipher)
    {
        DataBuf b(bytes,size); // copy the data

        // initialize the code table
        byte  code[256];
        for ( uint32_t i = 0 ; i < 249 ; i++ ) {
            if ( bDecipher ) {
                code[(i * i * i) % 249] = i ;
            } else {
                code[i] = (i * i * i) % 249 ;
            }
        }
        for ( uint32_t i = 249 ; i < 256 ; i++ ) {
            code[i] = i;
        }

        // code byte-by-byte
        for ( uint32_t i = 0 ; i < size ; i++ ) {
            b.pData_[i] = code[bytes[i]];
        }

        return b;
    }

    DataBuf sonyTagDecipher(uint16_t tag, const byte* bytes, uint32_t size, TiffComponent* const object)
    {
        return sonyTagCipher(tag,bytes,size,object,true);
    }
    DataBuf sonyTagEncipher(uint16_t tag, const byte* bytes, uint32_t size, TiffComponent* const object)
    {
        return sonyTagCipher(tag,bytes,size,object,false);
    }

}}                                      // namespace Internal, Exiv2
