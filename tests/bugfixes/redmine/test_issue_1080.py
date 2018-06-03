# -*- coding: utf-8 -*-

import system_tests

class CheckDivisionByZero(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1080"

    filename = system_tests.path("$data_path/exiv2-bug1080.jpg")
    commands = [ "$exiv2 -pa $filename" ]

    stdout = [ """Exif.Image.ExifTag                           Long        1  26
Exif.Photo.ExposureBiasValue                 SRational   1  0 EV
"""
    ]
    stderr = [""]
    retval = [0]
