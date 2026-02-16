// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
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
// easyaccess-test.cpp
// Sample program using high-level metadata access functions

#include <exiv2/exiv2.hpp>
#include <iostream>
#include <iomanip>
#include <cassert>

typedef Exiv2::ExifData::const_iterator (*EasyAccessFct)(const Exiv2::ExifData& ed);

struct EasyAccess {
    const char*   label_;
    EasyAccessFct findFct_;
};

static const EasyAccess easyAccess[] = {
    { "Orientation",          Exiv2::orientation      },
    { "ISO speed",            Exiv2::isoSpeed         },
    { "Date & time original", Exiv2::dateTimeOriginal },
    { "Flash bias",           Exiv2::flashBias        },
    { "Exposure mode",        Exiv2::exposureMode     },
    { "Scene mode",           Exiv2::sceneMode        },
    { "Macro mode",           Exiv2::macroMode        },
    { "Image quality",        Exiv2::imageQuality     },
    { "White balance",        Exiv2::whiteBalance     },
    { "Lens name",            Exiv2::lensName         },
    { "Saturation",           Exiv2::saturation       },
    { "Sharpness",            Exiv2::sharpness        },
    { "Contrast",             Exiv2::contrast         },
    { "Scene capture type",   Exiv2::sceneCaptureType },
    { "Metering mode",        Exiv2::meteringMode     },
    { "Camera make",          Exiv2::make             },
    { "Camera model",         Exiv2::model            },
    { "Exposure time",        Exiv2::exposureTime     },
    { "FNumber",              Exiv2::fNumber          },
    { "Shutter speed value",  Exiv2::shutterSpeedValue },
    { "Aperture value",       Exiv2::apertureValue    },
    { "Brightness value",     Exiv2::brightnessValue  },
    { "Exposure bias",        Exiv2::exposureBiasValue },
    { "Max aperture value",   Exiv2::maxApertureValue },
    { "Subject distance",     Exiv2::subjectDistance  },
    { "Light source",         Exiv2::lightSource      },
    { "Flash",                Exiv2::flash            },
    { "Camera serial number", Exiv2::serialNumber     },
    { "Focal length",         Exiv2::focalLength      },
    { "Subject location/area", Exiv2::subjectArea     },
    { "Flash energy",         Exiv2::flashEnergy      },
    { "Exposure index",       Exiv2::exposureIndex    },
    { "Sensing method",       Exiv2::sensingMethod    },
    { "AF point",             Exiv2::afPoint          }
};

int main(int argc, char **argv)
try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[1]);
    assert (image.get() != 0);
    image->readMetadata();
    Exiv2::ExifData& ed = image->exifData();

    for (unsigned int i = 0; i < EXV_COUNTOF(easyAccess); ++i) {
        Exiv2::ExifData::const_iterator pos = easyAccess[i].findFct_(ed);
        std::cout << std::setw(21) << std::left << easyAccess[i].label_;
        if (pos != ed.end()) {
            std::cout << " (" << std::setw(35) << pos->key() << ") : "
                      << pos->print(&ed) << "\n";
        }
        else {
            std::cout << " (" << std::setw(35) << " " << ") : \n";
        }
    }

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
