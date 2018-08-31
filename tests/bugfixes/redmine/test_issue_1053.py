# -*- coding: utf-8 -*-

import system_tests

class CheckOptionK(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1053"

    filename = system_tests.path("$data_path/exiv2-bug884c.jpg")
    commands = [ "$exiv2 -PE -g ImageWidth            $filename",
        "$exiv2 -PE -K ImageWidth            $filename",
        "$exiv2 -PE -K Exif.Image.ImageWidth $filename" ]

    stdout = [ """Exif.Image.ImageWidth                        Long        1  3040
""",
"",
"""Exif.Image.ImageWidth                        Long        1  3040
""",
    ]
    stderr = [""] * 3
    retval = [0, 1, 0]
