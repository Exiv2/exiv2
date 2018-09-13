# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/76"

    filename = "$data_path/010_bad_free"
    commands = ["$exiv2 $filename"]
    retval = [0]
    stdout = [
        """File name       : $filename
File size       : 20274 Bytes
MIME type       : image/tiff
Image size      : 12336 x 12336
Camera make     : 00000000
Camera model    : 000000000000
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
Exif Resolution : 12336 x 12336
White balance   : 
Thumbnail       : None
Copyright       : 00000
Exif comment    : 

"""
    ]
    stderr = [
        """Warning: Directory Image, entry 0x0111: Strip 0 is outside of the data area; ignored.
Warning: Directory Image, entry 0x0111: Strip 1 is outside of the data area; ignored.
Warning: Directory Image, entry 0x0111: Strip 2 is outside of the data area; ignored.
Warning: Directory Image, entry 0x0111: Strip 3 is outside of the data area; ignored.
Warning: Directory Image, entry 0x0111: Strip 4 is outside of the data area; ignored.
Warning: Directory Image, entry 0x0111: Strip 5 is outside of the data area; ignored.
Warning: Directory Image, entry 0x0111: Strip 6 is outside of the data area; ignored.
Warning: Directory Image, entry 0x0111: Strip 7 is outside of the data area; ignored.
Warning: Directory Image, entry 0x0111: Strip 8 is outside of the data area; ignored.
Warning: Directory Image, entry 0x0111: Strip 9 is outside of the data area; ignored.
Error: Offset of directory Image, entry 0x0132 is out of bounds: Offset = 0x30003030; truncating the entry
Error: Directory Image, entry 0x8649 has invalid size 4294967295*1; skipping entry.
Error: Directory Image, entry 0x8769 Sub-IFD pointer 0 is out of bounds; ignoring it.
Error: XMP Toolkit error 201: XML parsing failure
Warning: Failed to decode XMP metadata.
"""
    ]
