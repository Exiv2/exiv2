# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/50"

    filename = "$data_path/POC3"
    commands = ["$exiv2 $filename"]
    stdout = [
        """File name       : $filename
File size       : 28 Bytes
MIME type       : image/tiff
Image size      : 0 x 0
"""
    ]
    stderr = [
        """Error: Directory Image, entry 0x0144 has invalid size 4294967295*8; skipping entry.
$filename: No Exif data found in the file
"""
    ]
    retval = ["$no_exif_data_found_retval"]
