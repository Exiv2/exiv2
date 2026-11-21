# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/60"

    filename = "$data_path/POC13"
    commands = ["$exiv2 $filename"]
    stdout = [
        """File name       : $filename
File size       : 60 Bytes
MIME type       : image/x-olympus-orf
Image size      : 0 x 0
"""
    ]
    stderr = [
        """Error: Directory Image, entry 0x0000 has invalid size 65536*1; skipping entry.
Error: Directory Image, entry 0x0117 has invalid size 65536*1; skipping entry.
Error: Directory Image, entry 0x0004 has invalid size 4294967289*2; skipping entry.
Error: Directory Image, entry 0x0100 has invalid size 65536*1; skipping entry.
$filename: No Exif data found in the file
"""
    ]
    # Every Exif value has an impossible count; report that no Exif remains.
    retval = [253]
