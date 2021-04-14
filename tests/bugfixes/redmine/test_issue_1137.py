# -*- coding: utf-8 -*-

import system_tests


@system_tests.CopyFiles("$data_path/exiv2-empty.jpg")
class MetadataPiping(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1137"

    filename = system_tests.path("$data_path/exiv2-empty_copy.jpg")
    Stonehenge = system_tests.path("$data_path/Stonehenge.exv")

    commands = [
        """$exiv2 -pa                   $filename""",
        """$exiv2 -PkV --grep GPSL      $Stonehenge""",
        """$exiv2 -m- $filename""",
        """$exiv2 -pa  --grep GPSL      $filename"""
    ]

    output_grep_GPSL = """set Exif.GPSInfo.GPSLatitudeRef                   N
set Exif.GPSInfo.GPSLatitude                      51/1 106969/10000 0/1
set Exif.GPSInfo.GPSLongitudeRef                  W
set Exif.GPSInfo.GPSLongitude                     1/1 495984/10000 0/1
"""

    stdin = [
        None,
        None,
        output_grep_GPSL,
        None
    ]

    stdout = [
        "",
        output_grep_GPSL,
        "",
        """Exif.GPSInfo.GPSLatitudeRef                  Ascii       2  North
Exif.GPSInfo.GPSLatitude                     Rational    3  51 deg 10' 41.81"
Exif.GPSInfo.GPSLongitudeRef                 Ascii       2  West
Exif.GPSInfo.GPSLongitude                    Rational    3  1 deg 49' 35.90"
"""
    ]
