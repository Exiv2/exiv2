# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/49"

    filename = "$data_path/POC2"
    commands = ["$exiv2 $filename"]
    retval = [0]
    stdout = ["""File name       : $filename
File size       : 60 Bytes
MIME type       : image/tiff
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
        """Error: Directory Image, entry 0x0000 has invalid size 4294967295*8; skipping entry.
Error: Directory Image, entry 0x0000 has invalid size 1229524224*4; skipping entry.
Error: Directory Image, entry 0x0000 has invalid size 1229520896*8; skipping entry.
"""]
