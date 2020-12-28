# -*- coding: utf-8 -*-

import system_tests

class Nikon_LensData8(metaclass=system_tests.CaseMeta):
    url      = "https://github.com/Exiv2/exiv2/pull/1437"
    filename = "$data_path/CH0_0174.exv"
    commands = ["$exiv2 -g lens/i -g aperture/i $filename"]
    stderr   = [""]
    stdout   = ["""Exif.Nikon3.LensType                         Byte        1  D G VR
Exif.Nikon3.Lens                             Rational    4  70-200mm F2.8
Exif.Nikon3.LensFStops                       Undefined   4  6
Exif.NikonLd4.AFAperture                     Byte        1  F2.8
Exif.NikonLd4.LensIDNumber                   Byte        1  Nikon AF-S VR Zoom-Nikkor 70-200mm f/2.8G IF-ED
Exif.NikonLd4.LensFStops                     Byte        1  F6.0
Exif.NikonLd4.MaxApertureAtMinFocal          Byte        1  F2.8
Exif.NikonLd4.MaxApertureAtMaxFocal          Byte        1  F2.8
Exif.NikonLd4.EffectiveMaxAperture           Byte        1  F2.8
Exif.NikonLd4.LensID                         Short       1  0
Exif.NikonLd4.MaxAperture                    Short       1  F0.5
Exif.Photo.LensSpecification                 Rational    4  700/10 2000/10 280/100 280/100
Exif.Photo.LensMake                          Ascii       6  
Exif.Photo.LensModel                         Ascii      65  
Exif.Photo.LensSerialNumber                  Ascii      11  
"""]
    retval = [0]
