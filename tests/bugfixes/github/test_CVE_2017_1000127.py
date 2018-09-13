# -*- coding: utf-8 -*-

import system_tests


class TestPoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/176"

    filename = "$data_path/heap-oob-write.tiff"
    commands = ["$exiv2 $filename"]
    stdout = [
        """File name       : $filename
File size       : 310 Bytes
MIME type       : image/tiff
Image size      : 200 x 130
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
Exif Resolution : 200 x 130
White balance   : 
Thumbnail       : None
Copyright       : 
Exif comment    : 

"""
    ]
    stderr = ["ignored"]
    retval = [0]

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors
