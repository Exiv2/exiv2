# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class Fix646_NikonAF22(metaclass=CaseMeta):

    url      =   "http://dev.exiv2.org/issues/646"
    filename =   "$data_path/_DSC8437.exv"
    commands = [
                  "$exiv2 -pa --grep NikonAf $filename"
]
    stdout   = [
        """Exif.NikonAf22.Version                       Undefined   4  1.01
Exif.NikonAf22.ContrastDetectAF              Byte        1  On
Exif.NikonAf22.AFAreaMode                    Byte        1  1
Exif.NikonAf22.PhaseDetectAF                 Byte        1  Off
Exif.NikonAf22.PrimaryAFPoint                Byte        1  0
Exif.NikonAf22.AFPointsUsed                  Byte        7  0 0 0 0 0 0 0
Exif.NikonAf22.AFImageWidth                  Short       1  8256
Exif.NikonAf22.AFImageHeight                 Short       1  5504
Exif.NikonAf22.AFAreaXPosition               Short       1  3950
Exif.NikonAf22.AFAreaYPosition               Short       1  2871
Exif.NikonAf22.AFAreaWidth                   Short       1  435
Exif.NikonAf22.AFAreaHeight                  Short       1  360
Exif.NikonAf22.ContrastDetectAFInFocus       Byte        2  1 0
"""
    ]
    stderr = [ "" ] * len(commands)
    retval = [ 0  ] * len(commands)
