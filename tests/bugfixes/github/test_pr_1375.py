# -*- coding: utf-8 -*-

import system_tests

class Olympus_MZuiko_17mm_f12_Pro(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1375"
    
    filename = "$data_path/olympus-m.zuiko-17mm-f1.2-pro.exv"
    commands  = ["$exiv2 -pa -K Exif.OlympusEq.LensType $filename"]
    stderr = ["""Warning: Directory OlympusCs, entry 0x0101: Strip 0 is outside of the data area; ignored.
"""]
    stdout = ["""Exif.OlympusEq.LensType                      Byte        6  Olympus M.Zuiko Digital ED 17mm F1.2 Pro
"""]
    retval = [0]
