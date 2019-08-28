# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class test_issue_990Test(metaclass=CaseMeta):

    filename = path("$data_path/test_issue_990.exv")
    commands  = ["$exiv2 -pa --grep lens/i $filename"
                ]
    stdout = ["""Exif.Nikon3.LensType                         Byte        1  D G VR
Exif.Nikon3.Lens                             Rational    4  70-200mm F2.8
Exif.Nikon3.LensFStops                       Undefined   4  7
Exif.NikonLd3.LensIDNumber                   Byte        1  Tamron SP 70-200mm F/2.8 Di VC USD
Exif.NikonLd3.LensFStops                     Byte        1  F7.0
"""
             ]
    stderr = [""]
    retval = [ 0]
