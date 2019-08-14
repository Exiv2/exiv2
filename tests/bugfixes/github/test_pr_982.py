# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class CanonAfTest(metaclass=CaseMeta):

    filename = path("$data_path/exiv2-pr982.exv")
    commands = ["$exiv2 -pa --grep CanonAf $filename"]

    stdout = ["""Exif.CanonAf2.AFInfoSize                     Short       1  96
Exif.CanonAf2.AFMode                         Short       1  Single-point AF
Exif.CanonAf2.AFNumPoints                    Short       1  9
Exif.CanonAf2.AFNumValid                     Short       1  9
Exif.CanonAf2.AFImageWidth                   Short       1  4752
Exif.CanonAf2.AFImageHeight                  Short       1  3168
Exif.CanonAf2.AFWidth                        Short       1  4272
Exif.CanonAf2.AFHeight                       Short       1  2848
"""
    ]
    stderr = [""]
    retval = [0]
