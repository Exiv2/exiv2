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
Thumbnail       : None
Camera make     : 
Camera model    : 
Image timestamp : 
File number     : 
Exposure time   : 
Aperture        : 
Exposure bias   : 
Flash           : 
Flash bias      : 
Focal length    : 
Subject distance: 
ISO speed       : 
Exposure mode   : 
Metering mode   : 
Macro mode      : 
Image quality   : 
White balance   : 
Copyright       : 
Exif comment    : 

"""
    ]
    stderr = [
        """Error: Offset of directory Image, entry 0x0000 is out of bounds: Offset = 0x7e000000; truncating the entry
Error: Offset of directory Image, entry 0x0117 is out of bounds: Offset = 0x4f524900; truncating the entry
Error: Directory Image, entry 0x0004 has invalid size 4294967289*2; skipping entry.
Error: Offset of directory Image, entry 0x0100 is out of bounds: Offset = 0x0012ff00; truncating the entry
"""
    ]
    retval = [0]
