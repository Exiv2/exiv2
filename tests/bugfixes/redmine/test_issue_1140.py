# -*- coding: utf-8 -*-

import system_tests

class CheckCanonLenses(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1140"

    filename = system_tests.path("$data_path/exiv2-bug1140.exv")
    commands = [ "$exiv2 -pa -g Lens $filename" ]

    stdout = [ """Exif.CanonCs.LensType                        Short       1  Canon EF-S 24mm f/2.8 STM
Exif.CanonCs.Lens                            Short       3  24.0 mm
Exif.CanonCf.LensAFStopButton                Short       1  2304
Exif.Canon.LensModel                         Ascii      64  
"""
    ]
    stderr = [""]
    retval = [0]
