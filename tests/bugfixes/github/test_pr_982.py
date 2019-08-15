# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class CanonAfTest(metaclass=CaseMeta):

    filename = path("$data_path/exiv2-pr982.exv")
    commands = ["$exiv2 -pa --grep CanonAf $filename"]

    stdout = ["""Exif.CanonAf2.InfoSize                       Short       1  96
Exif.CanonAf2.Mode                           Short       1  Single-point AF
Exif.CanonAf2.NumPoints                      Short       1  9
Exif.CanonAf2.NumValid                       Short       1  9
Exif.CanonAf2.ImageWidth                     Short       1  4752
Exif.CanonAf2.ImageHeight                    Short       1  3168
Exif.CanonAf2.Width                          Short       1  4272
Exif.CanonAf2.Height                         Short       1  2848
Exif.CanonAf2.AreaWidths                     SShort      9  115 115 115 162 200 162 115 115 115
Exif.CanonAf2.AreaHeights                    SShort      9  153 153 153 105 199 105 153 153 153
Exif.CanonAf2.AreaXPositions                 SShort      9  -1127 -674 -674 0 0 0 674 674 1127
Exif.CanonAf2.AreaYPositions                 SShort      9  0 321 -321 603 0 -603 321 -321 0
Exif.CanonAf2.PointsInFocus                  SShort      1  16
Exif.CanonAf2.PointsSelected                 Short       1  256
Exif.CanonAf2.PrimaryPoint                   Short       1  0
"""
    ]
    stderr = [""]
    retval = [0]
