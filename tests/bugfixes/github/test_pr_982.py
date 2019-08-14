# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class CanonAfTest(metaclass=CaseMeta):

    filename = path("$data_path/exiv2-pr982.exv")
    commands = ["$exiv2 -pa --grep CanonAf $filename"]

    stdout = ["""Exif.CanonAf.AFInfoSize                      Short       1  96
Exif.CanonAf.AFMode                          Short       1  Single-point AF
Exif.CanonAf.AFNumPoints                     Short       1  9
Exif.CanonAf.AFNumValid                      Short       1  9
Exif.CanonAf.AFImageWidth                    Short       1  4752
Exif.CanonAf.AFImageHeight                   Short       1  3168
Exif.CanonAf.AFWidth                         Short       1  4272
Exif.CanonAf.AFHeight                        Short       1  2848
"""
    ]
    stderr = [""]
    retval = [0]
