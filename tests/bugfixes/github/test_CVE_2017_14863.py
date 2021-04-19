# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/132"

    filename = "$data_path/01-Null-exiv2-poc"
    commands = ["$exiv2 $filename"]
    stdout = [
        """File name       : $filename
File size       : 24550 Bytes
MIME type       : image/tiff
Image size      : 12336 x 12336
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
        """Warning: Directory Image, entry 0x0144: Strip 0 is outside of the data area; ignored.
Error: Directory Image, entry 0x87b1 has invalid size 4294967295*1; skipping entry.
"""
    ]
    retval = [0]
