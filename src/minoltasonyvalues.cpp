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
    extern TagDetails minoltaSonyLensID[] = {
        { 0,     "AF28-85mm F3.5-4.5"                                                                   },
        { 1,     "AF80-200mm F2.8G"                                                                     },
        { 2,     "AF28-70mm F2.8G"                                                                      },
        { 3,     "AF28-80mm F4-5.6"                                                                     },
        { 4,     "AF85mm F1.4G"                                                                         },
        { 5,     "AF35-70mm F3.5-4.5"                                                                   },
        { 6,     "AF24-85mm F3.5-4.5"                                                                   },
        { 7,     "AF100-300mm F4.5-5.6(D) APO | AF100-400mm F4.5-6.7(D)"                                },
        { 8,     "AF70-210mm F4.5-5.6"                                                                  },
        { 9,     "AF50mm F3.5 Macro"                                                                    },
        { 10,    "AF28-105mm F3.5-4.5"                                                                  },
        { 11,    "AF300mm F4G APO"                                                                      },
        { 12,    "AF100mm F2.8 Soft Focus"                                                              },
        { 13,    "AF75-300mm F4.5-5.6"                                                                  },
        { 14,    "AF100-400mm F4.5-6.7 APO"                                                             },
        { 15,    "AF400mm F4.5G APO"                                                                    },
        { 16,    "AF17-35mm F3.5G"                                                                      },
        { 17,    "AF20-35mm F3.5-4.5"                                                                   },
        { 18,    "AF28-80mm F3.5-5.6"                                                                   },
        { 19,    "AF35mm F1.4G"                                                                         },
        { 20,    "STF135mm F2.8[T4.5]"                                                                  },
        { 22,    "AF35-80mm F4-5.6"                                                                     },
        { 23,    "AF200mm F4G APO Macro"                                                                },
        { 24,    "AF24-105mm F3.5-4.5(D) | SIGMA 18-50mm F2.8 | Sigma DC 18-125mm F4-5,6 D"             },
        { 25,    "AF100-300mm F4.5-5.6 APO(D) | SIGMA 30mm F1.4"                                        },
        { 27,    "AF85mm F1.4G(D)"                                                                      },
        { 28,    "AF100mm F2.8 Macro(D)"                                                                },
        { 29,    "AF75-300mm F4.5-5.6(D)"                                                               },
        { 30,    "AF28-80mm F3.5-5.6(D)"                                                                },
        { 31,    "AF50mm F2.8 Macro(D) | AF50mm F3.5 Macro"                                             },
        { 32,    "AF100-400mm F4.5-6.7(D) x1.5 | AF300mm F2.8G APO(D) SSM"                              },
        { 33,    "AF70-200mm F2.8G APO(D) SSM"                                                          },
        { 35,    "AF85mm F1.4G(D) Limited"                                                              },
        { 36,    "AF28-100mm F3.5-5.6(D)"                                                               },
        { 38,    "AF17-35mm F2.8-4(D)"                                                                  },
        { 39,    "AF28-75mm F2.8(D)"                                                                    },
        { 40,    "AFDT18-70mm F3.5-5.6(D)"                                                              },
        { 41,    "AFDT11-18mm F4.5-5.6(D)"                                                              },
        { 42,    "AFDT18-200mm F3.5-6.3(D)"                                                             },
        { 45,    "CZ Planar T* AF85mm F1.4"                                                             },
        { 46,    "CZ Vario-Sonnar T* AFDT16-80mm F3.5-4.5"                                              },
        { 47,    "CZ Sonnar T* 135mm F1.8"                                                              },
        { 128,   "SIGMA 70-200mm F2.8 APO EX DG MACRO | TAMRON 18-200, 28-300 | 80-300mm F3.5-6.3"      },
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
        { 25551, "AF70-210mm F4 Macro | SIGMA 70-210mm F4-5.6 APO"                                      },
        { 25560, "AF135mm F2.8"                                                                         },
        { 25561, "AF135mm F2.8"                                                                         },
        { 25570, "AF28mm F2.8"                                                                          },
        { 25571, "AF28mm F2.8"                                                                          },
        { 25580, "AF24-50mm F4"                                                                         },
        { 25581, "AF24-50mm F4"                                                                         },
        { 25600, "AF100-200mm F4.5"                                                                     },
        { 25601, "AF100-200mm F4.5"                                                                     },
        { 25610, "AF75-300mm F4.5-5.6"                                                                  },
        { 25611, "SIGMA 70-300mm F4-5.6 | SIGMA 300mm F4 APO Macro"                                     },
        { 25620, "AF50mm F1.4"                                                                          },
        { 25621, "AF50mm F1.4 NEW"                                                                      },
        { 25630, "AF300mm F2.8G APO"                                                                    },
        { 25631, "AF300mm F2.8G"                                                                        },
        { 25640, "AF50mm F2.8 Macro"                                                                    },
        { 25641, "AF50mm F2.8 Macro"                                                                    },
        { 25650, "AF600mm F4 APO"                                                                       },
        { 25660, "AF24mm F2.8"                                                                          },
        { 25661, "AF24mm F2.8 | SIGMA 17-35mm F2.8-4.0 EX-D"                                            },
        { 25720, "AF500mm F8 Reflex"                                                                    },
        { 25721, "AF500mm F8 Reflex"                                                                    },
        { 25780, "AF16mm F2.8 Fisheye"                                                                  },
        { 25781, "AF16mm F2.8 Fisheye | SIGMA 8mm F4 Fisheye"                                           },
        { 25790, "AF20mm F2.8"                                                                          },
        { 25791, "AF20mm F2.8"                                                                          },
        { 25810, "AF100mm F2.8 Macro"                                                                   },
        { 25811, "AF100mm F2.8 Macro(D) | TAMRON 90mm F2.8 Macro | SIGMA 180mm F5.6 Macro"              },
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
        { 26061, "AF100-300mm F4.5-5.6(D) | SIGMA 105mm F2.8 Macro EX-DG"                               },
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

}                                       // namespace Exiv2
