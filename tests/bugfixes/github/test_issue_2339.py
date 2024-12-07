# -*- coding: utf-8 -*-

from system_tests import CaseMeta


class issue_2339_printDegrees_integer_overflow(metaclass=CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/2339"
    filename = "$data_path/issue_2339_poc.tiff"
    commands = ["$exiv2 -q -pa $filename"]
    retval = [0]
    stderr = [""]
    stdout = [
        """Exif.Image.ExifTag                           SLong       7  2 10 538976288 -2023161847 7 4 -822083584
Exif.GPSInfo.GPSDestLatitude                 Float       3  (9.80908925027372e-45 5.60519385729927e-45 -2147483648)
"""
    ]
