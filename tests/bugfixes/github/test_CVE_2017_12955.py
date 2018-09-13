# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/58"

    filename = "$data_path/POC11"
    commands = ["$exiv2 $filename"]
    stdout = [
        """File name       : $filename
File size       : 100 Bytes
MIME type       : image/pgf
Image size      : 131345 x 65536
"""
    ]
    stderr = ["""Error: Directory Image: Next pointer is out of bounds; ignored.
Error: Directory Image, entry 0x0000 has invalid size 3402235904*1; skipping entry.
Error: Directory Image, entry 0x014a has invalid size 4294967295*1; skipping entry.
Warning: Directory Image, entry 0x014a doesn't look like a sub-IFD.
Warning: Directory Image, entry 0x4720 has unknown Exif (TIFF) type 60362; setting type size 1.
Error: Directory Image, entry 0x4720 has invalid size 1330792777*1; skipping entry.
Warning: Directory Image, entry 0x0001 has unknown Exif (TIFF) type 0; setting type size 1.
Error: Directory Image, entry 0x0001 has invalid size 3401632458*1; skipping entry.
$filename: No Exif data found in the file
"""]
    retval = ["$no_exif_data_found_retval"]
