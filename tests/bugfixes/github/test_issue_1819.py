# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path, check_no_ASAN_UBSAN_errors

class EmptyStringXmpTextValueRead(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1819
    """
    url = "https://github.com/Exiv2/exiv2/issues/1819"

    filename = path("$data_path/issue_1819_poc.exv")
    commands = ["$exiv2 -q $filename"]
    stdout = ["""File name       : $filename
File size       : 1088 Bytes
MIME type       : application/rdf+xml
Image size      : 0 x 0
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

"""]
    stderr = [""]
    retval = [0]
