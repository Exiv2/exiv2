# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class XMPUtilsSetTimeZoneIntegerOverflow(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1901
    """
    url = "https://github.com/Exiv2/exiv2/issues/1901"

    filename = path("$data_path/issue_1901_poc.xmp")
    commands = ["$exiv2 $filename"]
    stderr = ["""Warning: Unsupported date format
"""]
    stdout = ["""File name       : $filename
File size       : 170 Bytes
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
    retval = [0]
