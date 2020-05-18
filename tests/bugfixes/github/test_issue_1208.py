# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class Sigma_Art_85mm_F1_4Nikon_F_mount(metaclass=CaseMeta):

    filename = path("$data_path/exiv2-issue1208.exv")
    commands = ["$exiv2 -pa --grep lens/i $filename"]

    stdout = ["""Exif.Nikon3.LensType                         Byte        1  D G 
Exif.Nikon3.Lens                             Rational    4  85mm F1.4
Exif.Nikon3.LensFStops                       Undefined   4  7
Exif.NikonLd3.LensIDNumber                   Byte        1  Sigma 85mm F1.4 DG HSM | A
Exif.NikonLd3.LensFStops                     Byte        1  F7.0
"""
    ]
    stderr = [""]
    retval = [0]
