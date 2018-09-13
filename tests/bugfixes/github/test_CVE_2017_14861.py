# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = [
        "https://github.com/Exiv2/exiv2/issues/139",
        "https://bugzilla.redhat.com/show_bug.cgi?id=1494787"
    ]

    filename = "$data_path/009-stack-over"
    commands = ["$exiv2 $filename"]
    stdout = ["""File name       : $filename
File size       : 340 Bytes
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
        """Error: Directory Image: Next pointer is out of bounds; ignored.
Error: Offset of directory Image, entry 0x00fe is out of bounds: Offset = 0x00000000; truncating the entry
Error: Directory Image, entry 0x0100 has invalid size 1935897193*2; skipping entry.
Warning: Directory Image, entry 0x303e has unknown Exif (TIFF) type 12320; setting type size 1.
Error: Offset of directory Image, entry 0x0116 is out of bounds: Offset = 0x0011302a; truncating the entry
Warning: Directory Image, entry 0x0111: Strip 0 is outside of the data area; ignored.
"""
    ]
    retval = [0]
