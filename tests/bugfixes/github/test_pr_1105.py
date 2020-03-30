# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class Tamron_17_35_Di_OSD(metaclass=CaseMeta):

    filename = path("$data_path/exiv2-pr1105.exv")
    commands = ["$exiv2 -pa --grep lens/i $filename"]

    stdout = ["""Exif.Nikon3.LensType                         Byte        1  D G 
Exif.Nikon3.Lens                             Rational    4  17-35mm F2.8-4
Exif.Nikon3.LensFStops                       Undefined   4  5
Exif.NikonLd3.LensIDNumber                   Byte        1  Tamron 17-35mm F/2.8-4 Di OSD
Exif.NikonLd3.LensFStops                     Byte        1  F5.0
"""
    ]
    stderr = [""]
    retval = [0]
