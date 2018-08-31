# -*- coding: utf-8 -*-

import system_tests

class CheckLensZoomNikkor(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1026"

    filename = system_tests.path("$data_path/exiv2-bug1026.jpg")
    commands = [ "$exiv2 -q -pa -g Lens $filename" ]

    stdout = [ """Exif.Nikon3.LensType                         Byte        1  D G VR
Exif.Nikon3.Lens                             Rational    4  18-200mm F3.5-5.6
Exif.Nikon3.LensFStops                       Undefined   4  5.33333
Exif.NikonLd3.LensIDNumber                   Byte        1  Nikon AF-S DX VR Zoom-Nikkor 18-200mm f/3.5-5.6G IF-ED II
Exif.NikonLd3.LensFStops                     Byte        1  F5.3
"""
    ]
    stderr = [""]
    retval = [0]
