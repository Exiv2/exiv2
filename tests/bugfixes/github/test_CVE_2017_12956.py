# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/59"

    filename = "$data_path/POC12"
    commands = ["$exiv2 $filename"]
    stdout = [
        """File name       : $filename
File size       : 68 Bytes
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

"""
    ]
    stderr = [
        """Error: Offset of directory Image, entry 0x0111 is out of bounds: Offset = 0x00001b15; truncating the entry
Warning: Directory Image, entry 0x0111: Size and data offset entries have different number of components, ignoring them.
Error: Directory Image, entry 0x010f has invalid size 2147483647*2; skipping entry.
"""
    ]
    retval = [0]
