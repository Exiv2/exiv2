# -*- coding: utf-8 -*-

import system_tests


class PentaxImageWithIssues(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1305"

    filename = "$data_path/IMGP0006-min.jpg"

    commands = ["$exiv2 $filename"]

    stdout = ["""File name       : $filename
File size       : 12341 Bytes
MIME type       : image/jpeg
Image size      : 0 x 0
Thumbnail       : None
Camera make     : PENTAX000000000000000000000000000000000000000000
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

    stderr = [""] * len(commands)
    retval = [0]  * len(commands)

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors
