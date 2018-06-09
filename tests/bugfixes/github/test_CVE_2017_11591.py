# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/55"

    filename = "$data_path/POC8"
    commands = ["$exiv2 $filename"]
    stdout = [
        """File name       : $filename
File size       : 2044 Bytes
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
    stderr = ["ignored"]
    retval = [0]

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors
