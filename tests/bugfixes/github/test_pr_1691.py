# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class Tamron_SP_24_70mm_F28_Di_VC_USD_G2_0E(metaclass=CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1691"
    
    filename = path("$data_path/Tamron_SP_24-70mm_F2.8_Di_VC_USD_G2_0E.exv")
    commands = ["$exiv2 -pa --grep lens/i $filename"]

    stderr = [""]
    stdout = ["""Exif.Nikon3.LensType                         Byte        1  D G VR
Exif.Nikon3.Lens                             Rational    4  24-70mm F2.8
Exif.Nikon3.LensFStops                       Undefined   4  5.91667
Exif.NikonLd3.LensIDNumber                   Byte        1  Tamron SP 24-70mm F/2.8 Di VC USD G2
Exif.NikonLd3.LensFStops                     Byte        1  F5.9
"""
    ]
    stderr = [""]
    retval = [0]
