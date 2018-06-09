# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/53"

    filename = "$data_path/POC6"
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
        """Error: Directory Image, entry 0x0000 has invalid size 4294967295*1; skipping entry.
Error: Offset of directory Image, entry 0x0000 is out of bounds: Offset = 0x7e000000; truncating the entry
Error: Offset of directory Image, entry 0x0111 is out of bounds: Offset = 0x7e000000; truncating the entry
"""
    ]
    retval = [0]
