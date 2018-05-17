# -*- coding: utf-8 -*-

import system_tests


@system_tests.CopyFiles("$data_path/exiv2-empty.jpg")
class NonIntrusiveWriteUpdateIFD(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/452"

    filename = "$data_path/exiv2-empty_copy.jpg"
    commands = [
        """$exiv2 -u -v -M"set Exif.GPSInfo.GPSLatitude SRational -1/3 -2/3 -3/3" $filename""",
        "$exiv2 -u -pv $filename",
        """$exiv2 -u -v -M"set Exif.GPSInfo.GPSLatitude Rational 1/3 2/3 3/3" $filename""",
        "$exiv2 -u -pv $filename",

    ]
    stdout = [
        """File 1/1: $filename
Set Exif.GPSInfo.GPSLatitude "-1/3 -2/3 -3/3" (SRational)
""",
        """0x8825 Image        GPSTag                      Long        1  26
0x0002 GPSInfo      GPSLatitude                 SRational   3  -1/3 -2/3 -3/3
""",
        """File 1/1: $filename
Set Exif.GPSInfo.GPSLatitude "1/3 2/3 3/3" (Rational)
""",
        """0x8825 Image        GPSTag                      Long        1  26
0x0002 GPSInfo      GPSLatitude                 Rational    3  1/3 2/3 3/3
"""]
    stderr = [""] * 4
    retval = [0] * 4
