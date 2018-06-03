# -*- coding: utf-8 -*-

import system_tests

class CheckTiffPageNumber(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1044"

    filename = system_tests.path("$data_path/exiv2-bug1044.tif")
    commands = [ "$exiv2 -q -pa -g PageNumber $filename" ]

    stdout = [ """Exif.Image.PageNumber                        Short       2  0 1
"""
    ]
    stderr = [""]
    retval = [0]
