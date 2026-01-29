# -*- coding: utf-8 -*-

import system_tests


class test_issue_3455_sigma_print0x0008_empty(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/3455"

    filename = "$data_path/issue_3455_sigma_print0x0008_empty.jpg"
    commands = ["$exiv2 -q $filename"]
    stderr = [""]
    stdout = ["""File name       : $filename
File size       : 13904 Bytes
MIME type       : image/jpeg
Image size      : 150 x 91
Thumbnail       : image/jpeg, 11998 Bytes
Camera make     : SIGMA
Camera model    : SIGMA SD10
Image timestamp : 
File number     : 
Exposure time   : 
Aperture        : 
Exposure bias   : +1 EV
Flash           : No flash
Flash bias      : 
Focal length    : 14.0 mm
Subject distance: 
ISO speed       : 
Exposure mode   : 
Metering mode   : Multi-segment
Macro mode      : 
Image quality   : 12
White balance   : Incandescent
Copyright       : 
Exif comment    : 

"""]
    retval = [0]

class test_issue_3455_sigma_print0x0009_empty(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/3455"

    filename = "$data_path/issue_3455_sigma_print0x0009_empty.jpg"
    commands = ["$exiv2 -q $filename"]
    stderr = [""]
    stdout = ["""File name       : $filename
File size       : 13904 Bytes
MIME type       : image/jpeg
Image size      : 150 x 91
Thumbnail       : image/jpeg, 11998 Bytes
Camera make     : SIGMA
Camera model    : SIGMA SD10
Image timestamp : 
File number     : 
Exposure time   : 8 s
Aperture        : F5.6
Exposure bias   : 
Flash           : No flash
Flash bias      : 
Focal length    : 14.0 mm
Subject distance: 
ISO speed       : 100
Exposure mode   : Manual
Metering mode   : 
Macro mode      : 
Image quality   : 12
White balance   : Incandescent
Copyright       : 
Exif comment    : 

"""]
    retval = [0]
