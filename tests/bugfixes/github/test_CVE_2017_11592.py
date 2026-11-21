# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/56"

    filename = "$data_path/POC9"
    commands = ["$exiv2 $filename"]
    stdout = [
        """File name       : $filename
File size       : 523 Bytes
MIME type       : image/tiff
Image size      : 0 x 0
"""
    ]
    stderr = [
        """Error: Directory Image, entry 0x02bc has invalid size 2926*2; skipping entry.
Error: Directory Thumbnail: Next pointer is out of bounds; ignored.
Error: Directory Thumbnail, entry 0x02bc has invalid size 4294967295*2; skipping entry.
$filename: No Exif data found in the file
"""
    ]
    # Every Exif value has an impossible count; report that no Exif remains.
    retval = [253]
