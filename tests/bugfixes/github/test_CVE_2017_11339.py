# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/52"

    filename = "$data_path/POC5"
    commands = ["$exiv2 $filename"]
    stdout = ["""File name       : $filename
File size       : 60 Bytes
MIME type       : image/x-olympus-orf
Image size      : 0 x 0
Camera make     : 
Camera model    : 
Image timestamp : 
Image number    : 
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
Exif Resolution : 
White balance   : 
Thumbnail       : None
Copyright       : 
Exif comment    : 

"""]
    stderr = [
        """Error: Directory Image, entry 0xcf45 has invalid size 4294967292*4; skipping entry.
Error: Offset of directory Image, entry 0x8000 is out of bounds: Offset = 0x0012ff00; truncating the entry
"""
    ]
    retval = [0]
