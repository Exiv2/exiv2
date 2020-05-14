# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/54"

    filename = "$data_path/POC7"
    commands = ["$exiv2 -b $filename"]
    stdout = [
        """File name       : $filename
File size       : 632 Bytes
MIME type       : image/tiff
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

"""
    ]
    stderr = [
        """Error: Directory Image, entry 0x0000 has invalid size 1229520896*8; skipping entry.
Error: Directory Image, entry 0x0000 has invalid size 1447624704*8; skipping entry.
Error: Directory Image, entry 0x0111 has invalid size 4294967295*2; skipping entry.
Warning: Directory Image, entry 0x0111: Size or data offset value not set, ignoring them.
"""
    ]
    retval = [0]
