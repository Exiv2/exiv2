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
        """Error: Upper boundary of data for directory Image, entry 0x02bc is out of bounds: Offset = 0x00000001, size = 5852, exceeds buffer size by 5330 Bytes; truncating the entry
Error: Directory Thumbnail: Next pointer is out of bounds; ignored.
Error: Directory Thumbnail, entry 0x02bc has invalid size 4294967295*2; skipping entry.
"""
    ]
    retval = [0]
