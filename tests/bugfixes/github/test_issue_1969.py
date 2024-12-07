# -*- coding: utf-8 -*-

import system_tests


class TestPrintPlainWithSet(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/1969"

    filename = "$data_path/smiley1.jpg"
    commands = ["$exiv2 -K Exif.Image.ImageDescription -PV $filename"]
    stdout = [
        """set Exif.Image.ImageDescription                  Ascii      Well it is a smiley that happens to be green
"""
    ]
    retval = [0]
