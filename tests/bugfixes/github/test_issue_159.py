# -*- coding: utf-8 -*-

import system_tests


class TestFirstPoC(metaclass=system_tests.CaseMeta):
    """
    Regression test for the first bug described in:
    https://github.com/Exiv2/exiv2/issues/159
    """
    url = "https://github.com/Exiv2/exiv2/issues/159"

    filename = "$data_path/printStructure"
    commands = ["$exiv2 " + filename]
    stdout = [
        """File name       : $filename
File size       : 12357 Bytes
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
    stderr = [""]
    retval = [0]

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors


# todo:
# class TestSecondPoC(system_tests.Case):
