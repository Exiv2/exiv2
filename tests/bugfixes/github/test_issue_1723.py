# -*- coding: utf-8 -*-

import system_tests

class Olympus_MZuiko_12_45mm_f4_Pro(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/1723"
    
    filename = "$data_path/olympus-m.zuiko-12-45mm-f4-pro.exv"
    commands  = ["$exiv2 -pa -K Exif.OlympusEq.LensType $filename"]
    stderr = [""]
    stdout = ["""Exif.OlympusEq.LensType                      Byte        6  Olympus M.Zuiko Digital ED 12-45mm F4.0 Pro
"""]
    retval = [0]
