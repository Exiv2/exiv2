# -*- coding: utf-8 -*-

import system_tests


class NikonLens(metaclass=system_tests.CaseMeta):

    filenames = ["$data_path/exiv2-bug1014.exv", "$data_path/exiv2-bug1014_2.exv"]
    commands = ["$exiv2 -pa --grep lens/i %s" % filenames[0], "$exiv2 -pa --grep lens/i %s" % filenames[1]]

    stdout = ["""Exif.Nikon3.LensType                         Byte        1  D G VR
Exif.Nikon3.Lens                             Rational    4  24-70mm F2.8
Exif.Nikon3.LensFStops                       Undefined   4  6
Exif.NikonLd3.LensIDNumber                   Byte        1  Nikon AF-S Nikkor 24-70mm f/2.8E ED VR
Exif.NikonLd3.LensFStops                     Byte        1  F6.0
""",
              """Exif.Nikon3.LensType                         Byte        1  D G VR
Exif.Nikon3.Lens                             Rational    4  70-200mm F2.8
Exif.Nikon3.LensFStops                       Undefined   4  6
Exif.NikonLd3.LensIDNumber                   Byte        1  Nikon AF-S Nikkor 70-200mm f/2.8E FL ED VR
Exif.NikonLd3.LensFStops                     Byte        1  F6.0
"""
              ]
    stderr = ["", ""]
    retval = [0, 0]
