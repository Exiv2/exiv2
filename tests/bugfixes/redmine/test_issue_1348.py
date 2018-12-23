# -*- coding: utf-8 -*-

import system_tests


class NikonLensId227(metaclass=system_tests.CaseMeta):

    filename = "$data_path/exiv2-bug1348.exv"
    commands = ["$exiv2 -pa --grep lensidnumber/i $filename"]

    stdout = ["""Exif.NikonLd3.LensIDNumber                   Byte        1  Tamron SP AF 150-600mm F/5-6.3 Di VC USD G2
"""
	]
    stderr = [""]
    retval = [0]
