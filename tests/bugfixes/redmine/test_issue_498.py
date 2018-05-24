# -*- coding: utf-8 -*-

import system_tests


@system_tests.CopyFiles("$data_path/exiv2-empty.jpg")
class ReadDataAfterTagDelete(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/498"

    filename = "$data_path/exiv2-empty_copy.jpg"

    commands = [
        """$exiv2 -u -v -M"set Exif.GPSInfo.GPSLatitude 0/1 1/1 2/1" $filename""",
        "$exiv2 -u -v -pv $filename",
        """$exiv2 -u -v -M"del Exif.GPSInfo.GPSLatitude" $filename""",
        "$exiv2 -u -v -pv $filename"
    ]

    stdout = [
        """File 1/1: $filename
Set Exif.GPSInfo.GPSLatitude "0/1 1/1 2/1" (Rational)
""",
        """File 1/1: $filename
0x8825 Image        GPSTag                      Long        1  26
0x0002 GPSInfo      GPSLatitude                 Rational    3  0/1 1/1 2/1
""",
        """File 1/1: $filename
Del Exif.GPSInfo.GPSLatitude
""",
        """File 1/1: $filename
"""
    ]

    stderr = [""] * 3 + ["""$filename: No Exif data found in the file
"""]
    retval = [0] * 4
