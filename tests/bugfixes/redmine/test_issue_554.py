# -*- coding: utf-8 -*-

import system_tests


@system_tests.CopyFiles("$data_path/exiv2-empty.jpg")
class SettingExifImageDateTimeDateGivesValueTooLarge(
        metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/554"

    filename = "$data_path/exiv2-empty_copy.jpg"
    commands = [
        """$exiv2 -u -v -M"set Exif.Image.DateTime Date 2007-05-27" $filename""",
        "$exiv2 -u -pt $filename"
    ]
    stdout = [
        """File 1/1: $filename
Set Exif.Image.DateTime "2007-05-27" (Date)
""",
        """Exif.Image.DateTime                          Undefined   8  50 48 48 55 48 53 50 55
"""
    ]
    stderr = [
        """Error: 'Date' is not a valid Exif (TIFF) type; using type 'Undefined'.
""",
        ""
    ]
    retval = [0] * 2
