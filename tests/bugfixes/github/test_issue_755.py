# -*- coding: utf-8 -*-

import system_tests

class RAFPixelWidthAndHeight(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/755"

    commands = ["$exiv2 $data_path/issue_755.RAF"]
    stdout = [
        """File name       : $data_path/issue_755.RAF
File size       : 6850400 Bytes
MIME type       : image/x-fuji-raf
Image size      : 1472 x 2160
Camera make     : FUJIFILM
Camera model    : FinePix S5000 
Image timestamp : 2008:06:22 03:48:05
Image number    : 
Exposure time   : 1/280 s
Aperture        : F2.8
Exposure bias   : 0 EV
Flash           : No, compulsory
Flash bias      : 
Focal length    : 5.7 mm
Subject distance: 
ISO speed       : 200
Exposure mode   : Auto
Metering mode   : Multi-segment
Macro mode      : Off
Image quality   : NORMAL 
Exif Resolution : 1280 x 960
White balance   : Auto
Thumbnail       : image/jpeg, 8582 Bytes
Copyright       :     
Exif comment    : \n\n"""
    ]
    stderr = [""]
    retval = [0]
