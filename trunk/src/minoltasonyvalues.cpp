// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2010 Andreas Huggel <ahuggel@gmx.net>
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
  File:      minoltasonyvalues.cpp
  Version:   $Rev$
  Author(s): Gilles Caulier (cgilles) <caulier dot gilles at gmail dot com>
  History:   14-Apr-10, gc: submitted
  Credits:   See header file.
 */

// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "minoltasonyvalues.hpp"
#include "value.hpp"
#include "i18n.h"                // NLS support.

#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>

// *****************************************************************************
// member definitions
namespace Exiv2 {

        //! Lookup table to translate Minolta/Sony Lens id values to readable labels
    /* NOTE:
       - duplicate tags value are:
       0/25520, 4/25920, 13/25610, 19/25910, 22/26050/26070,
       25500/25501/26130, 25540/25541/25850, 25580/25581, 2564025641,
       25720/25721, 25790/25791, 25960/25961, 25980/25981, 26150/26151
       - No need to i18n these string.
    */
    extern TagDetails const minoltaSonyLensID[] = {
        { 0,     "Minolta AF 28-85mm F3.5-4.5 New" },
        { 1,     "Minolta AF 80-200mm F2.8 HS-APO G" },
        { 2,     "Minolta AF 28-70mm F2.8 G" },
        { 3,     "Minolta AF 28-80mm F4-5.6" },
        { 4,     "Minolta AF 85mm F1.4G" },
        { 5,     "Minolta AF 35-70mm F3.5-4.5 [II]" },
        { 6,     "Minolta AF 24-85mm F3.5-4.5 [New]" },
        { 7,     "Minolta AF 100-300mm F4.5-5.6(D) APO [New] | "
                 "Minolta AF 100-400mm F4.5-6.7(D) | "
                 "Sigma AF 100-300mm F4 EX DG IF" },
        { 8,     "Minolta AF 70-210mm F4.5-5.6 [II]" },
        { 9,     "Minolta AF 50mm F3.5 Macro" },
        { 10,    "Minolta AF 28-105mm F3.5-4.5 [New]" },
        { 11,    "Minolta AF 300mm F4 HS-APO G" },
        { 12,    "Minolta AF 100mm F2.8 Soft Focus" },
        { 13,    "Minolta AF 75-300mm F4.5-5.6 (New or II)" },
        { 14,    "Minolta AF 100-400mm F4.5-6.7 APO" },
        { 15,    "Minolta AF 400mm F4.5 HS-APO G" },
        { 16,    "Minolta AF 17-35mm F3.5 G" },
        { 17,    "Minolta AF 20-35mm F3.5-4.5" },
        { 18,    "Minolta AF 28-80mm F3.5-5.6 II" },
        { 19,    "Minolta AF 35mm F1.4 G" },
        { 20,    "Minolta/Sony 135mm F2.8 [T4.5] STF" },
        { 22,    "Minolta AF 35-80mm F4-5.6 II" },
        { 23,    "Minolta AF 200mm F4 Macro APO G" },
        { 24,    "Minolta/Sony AF 24-105mm F3.5-4.5 (D) | "
                 "Sigma 18-50mm F2.8 | "
                 "Sigma 17-70mm F2.8-4.5 (D) | "
                 "Sigma 20-40mm F2.8 EX DG Aspherical IF | "
                 "Sigma 18-200mm F3.5-6.3 DC | "
                 "Sigma 20-40mm F2.8 EX DG Aspherical IF | "
                 "Sigma DC 18-125mm F4-5,6 D | "
                 "Tamron SP AF 28-75mm F2.8 XR Di (IF) Macro" },
        { 25,    "Minolta AF 100-300mm F4.5-5.6 APO (D) | "
                 "Sigma 100-300mm F4 EX (APO (D) or D IF) | "
                 "Sigma 70mm F2.8 EX DG Macro | "
                 "Sigma 20mm F1.8 EX DG Aspherical RF | "
                 "Sigma 30mm F1.4 DG EX" },
        { 27,    "Minolta AF 85mm F1.4 G (D)" },
        { 28,    "Minolta/Sony AF 100mm F2.8 Macro (D) | "
                 "Tamron SP AF 90mm F2.8 Di Macro" },
        { 29,    "Minolta/Sony AF 75-300mm F4.5-5.6 (D) " },
        { 30,    "Minolta AF 28-80mm F3.5-5.6 (D) | "
                 "Sigma AF 10-20mm F4-5.6 EX DC | "
                 "Sigma AF 12-24mm F4.5-5.6 EX DG | "
                 "Sigma 28-70mm EX DG F2.8 | "
                 "Sigma 55-200mm F4-5.6 DC" },
        { 31,    "Minolta AF 50mm F2.8 Macro(D) | "
                 "Minolta AF 50mm F3.5 Macro" },
        { 32,    "Minolta AF 100-400mm F4.5-6.7(D) | "
                 "Minolta AF 300mm F2.8G APO(D) SSM" },
        { 33,    "Minolta/Sony AF 70-200mm F2.8 G" },
        { 35,    "Minolta AF 85mm F1.4 G (D) Limited" },
        { 36,    "Minolta AF 28-100mm F3.5-5.6 (D)" },
        { 38,    "Minolta AF 17-35mm F2.8-4 (D)" },
        { 39,    "Minolta AF 28-75mm F2.8 (D)" },
        { 40,    "Minolta/Sony AF DT 18-70mm F3.5-5.6 (D) | "
                 "Sony AF DT 18-200mm F3.5-6.3" },
        { 41,    "Minolta/Sony AF DT 11-18mm F4.5-5.6 (D) | "
                 "Tamron SP AF 11-18mm F4.5-5.6 Di II LD Aspherical IF" },
        { 42,    "Minolta/Sony AF DT 18-200mm F3.5-6.3 (D)" },
        { 43,    "Sony 35mm F1.4 G (SAL-35F14G)" },
        { 44,    "Sony 50mm F1.4 (SAL-50F14)" },
        { 45,    "Carl Zeiss Planar T* 85mm F1.4 ZA" },
        { 46,    "Carl Zeiss Vario-Sonnar T* DT 16-80mm F3.5-4.5 ZA" },
        { 47,    "Carl Zeiss Sonnar T* 135mm F1.8 ZA" },
        { 48,    "Carl Zeiss Vario-Sonnar T* 24-70mm F2.8 ZA SSM (SAL-2470Z)" },
        { 49,    "Sony AF DT 55-200mm F4-5.6" },
        { 50,    "Sony AF DT 18-250mm F3.5-6.3" },
        { 51,    "Sony AF DT 16-105mm F3.5-5.6 | "
                 "Sony AF DT 55-200mm F4-5.5" },
        { 52,    "Sony 70-300mm F4.5-5.6 G SSM" },
        { 53,    "Sony AF 70-400mm F4.5-5.6 G SSM (SAL-70400G)" },
        { 54,    "Carl Zeiss Vario-Sonnar T* 16-35mm F2.8 ZA SSM (SAL-1635Z)" },
        { 55,    "Sony DT 18-55mm F3.5-5.6 SAM (SAL-1855)" },
        { 56,    "Sony AF DT 55-200mm F4-5.6 SAM" },
        { 57,    "Sony AF DT 50mm F1.8 SAM" },
        { 58,    "Sony AF DT 30mm F2.8 SAM Macro" },
        { 128,   "Sigma 70-200mm F2.8 APO EX DG MACRO | TAMRON 18-200, 28-300 | 80-300mm F3.5-6.3"      },
        { 193,   "AF 1.4x APO II"                                                                       },
        { 25500, "AF50mm F1.7"                                                                          },
        { 25501, "AF50mm F1.7"                                                                          },
        { 25510, "AF35-70mm F1.4"                                                                       },
        { 25511, "AF35-70mm F4"                                                                         },
        { 25520, "AF28-85mm F3.5-4.5"                                                                   },
        { 25521, "TOKINA 19-35mm F3.5-4.5 | TOKINA 28-70mm F2.8 AT-X"                                   },
        { 25530, "AF28-135mm F4-4.5"                                                                    },
        { 25531, "AF28-135mm F4-4.5"                                                                    },
        { 25540, "AF35-105mm F3.5-4.5"                                                                  },
        { 25541, "AF35-105mm F3.5-4.5"                                                                  },
        { 25550, "AF70-210mm F4"                                                                        },
        { 25551, "AF70-210mm F4 Macro | Sigma 70-210mm F4-5.6 APO"                                      },
        { 25560, "AF135mm F2.8"                                                                         },
        { 25561, "AF135mm F2.8"                                                                         },
        { 25570, "AF28mm F2.8"                                                                          },
        { 25571, "AF28mm F2.8"                                                                          },
        { 25580, "AF24-50mm F4"                                                                         },
        { 25581, "AF24-50mm F4"                                                                         },
        { 25600, "AF100-200mm F4.5"                                                                     },
        { 25601, "AF100-200mm F4.5"                                                                     },
        { 25610, "AF75-300mm F4.5-5.6"                                                                  },
        { 25611, "Sigma 70-300mm F4-5.6 | Sigma 300mm F4 APO Macro"                                     },
        { 25620, "AF50mm F1.4"                                                                          },
        { 25621, "AF50mm F1.4 NEW"                                                                      },
        { 25630, "AF300mm F2.8G APO"                                                                    },
        { 25631, "AF300mm F2.8G"                                                                        },
        { 25640, "AF50mm F2.8 Macro"                                                                    },
        { 25641, "AF50mm F2.8 Macro"                                                                    },
        { 25650, "AF600mm F4 APO"                                                                       },
        { 25660, "AF24mm F2.8"                                                                          },
        { 25661, "AF24mm F2.8 | Sigma 17-35mm F2.8-4.0 EX-D"                                            },
        { 25720, "AF500mm F8 Reflex"                                                                    },
        { 25721, "AF500mm F8 Reflex"                                                                    },
        { 25780, "AF16mm F2.8 Fisheye"                                                                  },
        { 25781, "AF16mm F2.8 Fisheye | Sigma 8mm F4 Fisheye"                                           },
        { 25790, "AF20mm F2.8"                                                                          },
        { 25791, "AF20mm F2.8"                                                                          },
        { 25810, "AF100mm F2.8 Macro"                                                                   },
        { 25811, "AF100mm F2.8 Macro(D) | TAMRON 90mm F2.8 Macro | Sigma 180mm F5.6 Macro"              },
        { 25850, "AF35-105mm F3.5-4.5"                                                                  },
        { 25858, "TAMRON 24-135mm F3.5-5.6"                                                             },
        { 25880, "AF70-210mm F3.5-4.5"                                                                  },
        { 25890, "AF80-200mm F2.8 APO"                                                                  },
        { 25891, "TOKINA 80-200mm F2.8"                                                                 },
        { 25910, "AF35mm F1.4G"                                                                         },
        { 25911, "AF35mm F1.4G"                                                                         },
        { 25920, "AF85mm F1.4G"                                                                         },
        { 25921, "AF85mm F1.4G(D)"                                                                      },
        { 25930, "AF200mm F2.8 APO"                                                                     },
        { 25931, "AF200mm F2.8G"                                                                        },
        { 25940, "AF 3X-1X F1.7-2.8 Macro"                                                              },
        { 25960, "AF28mm F2"                                                                            },
        { 25961, "AF28mm F2"                                                                            },
        { 25970, "AF35mm F2"                                                                            },
        { 25971, "AF35mm F2"                                                                            },
        { 25980, "AF100mm F2"                                                                           },
        { 25981, "AF100mm F2"                                                                           },
        { 26040, "AF80-200mm F4.5-5.6"                                                                  },
        { 26050, "AF35-80mm F4-5.6"                                                                     },
        { 26060, "AF100-300mm F4.5-5.6"                                                                 },
        { 26061, "AF100-300mm F4.5-5.6(D) | Sigma 105mm F2.8 Macro EX-DG"                               },
        { 26070, "AF35-80mm F4-5.6"                                                                     },
        { 26071, "AF35-80mm F4-5.6"                                                                     },
        { 26080, "AF300mm F2.8G APO High Speed"                                                         },
        { 26081, "AF300mm F2.8G"                                                                        },
        { 26090, "AF600mm F4G APO High Speed"                                                           },
        { 26120, "AF200mm F2.8G APO High Speed"                                                         },
        { 26121, "AF200mm F2.8G(D)"                                                                     },
        { 26130, "AF50mm F1.7"                                                                          },
        { 26131, "AF50mm F1.7"                                                                          },
        { 26150, "AF28-105mm F3.5-4.5 Xi"                                                               },
        { 26151, "AF28-105mm F3.5-4.5 Xi"                                                               },
        { 26160, "AF35-200mm F4.5-5.6 Xi"                                                               },
        { 26161, "AF35-200mm F4.5-5.6 Xi"                                                               },
        { 26180, "AF28-80mm F4-5.6 Xi"                                                                  },
        { 26190, "AF80-200mm F4.5-5.6 Xi"                                                               },
        { 26210, "AF100-300mm F4.5-5.6 Xi"                                                              },
        { 26240, "AF35-80mm F4-5.6 Power"                                                               },
        { 26241, "AF35-80mm F4-5.6"                                                                     },
        { 45741, "AF200mm F2.8G x2 | TOKINA 300mm F2.8 x2"                                              }
    };

    std::ostream& printMinoltaSonyLensID(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyLensID)(os, value, metadata);
    }

    // TODO: Added shared methods here.

}                                       // namespace Exiv2
