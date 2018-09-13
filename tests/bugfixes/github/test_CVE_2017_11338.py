# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/51"

    filename = "$data_path/POC4"
    commands = ["$exiv2 $filename"]
    stdout = [
        """File name       : $filename
File size       : 35 Bytes
MIME type       : image/x-panasonic-rw2
Image size      : 0 x 0
"""
    ]
    stderr = [
        """Warning: Directory PanasonicRaw has an unexpected next pointer; ignored.
Error: Directory PanasonicRaw, entry 0x002e has invalid size 4294967295*1; skipping entry.
$filename: No Exif data found in the file
"""
    ]
    retval = ["$no_exif_data_found_retval"]
