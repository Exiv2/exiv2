# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/138"

    filename = "$data_path/007-heap-buffer-over"
    commands = ["$exiv2 $filename"]
    stdout = [
        """File name       : $filename
File size       : 331696 Bytes
MIME type       : image/tiff
Image size      : 0 x 12336
Thumbnail       : None
Camera make     : 0000
Camera model    : 0000000000000
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
        """Error: Offset of directory Image, entry 0x0100 is out of bounds: Offset = 0x30303030; truncating the entry
Warning: Directory Image, entry 0x0111: Strip 17 is outside of the data area; ignored.
Error: Directory Photo with 8224 entries considered invalid; not read.
Warning: Removing 913 characters from the beginning of the XMP packet
Error: XMP Toolkit error 201: XML parsing failure
Warning: Failed to decode XMP metadata.
"""
    ]
    retval = [0]
