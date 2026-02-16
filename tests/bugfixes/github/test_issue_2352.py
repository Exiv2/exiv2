# -*- coding: utf-8 -*-

from system_tests import CaseMeta


class issue_2352_floatToRationalCast_integer_overflow(metaclass=CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/2352"
    filename = "$data_path/issue_2352_poc.jpg"
    commands = ["$exiv2 -q $filename"]
    retval = [0]
    stderr = [""]
    stdout = [
        """File name       : $filename
File size       : 20 Bytes
MIME type       : image/tiff
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
Subject distance: -214748.50 m
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
