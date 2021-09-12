# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class XMPUtilsSetTimeZoneIntegerOverflow(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1901
    """
    url = "https://github.com/Exiv2/exiv2/issues/1901"

    filename1 = path("$data_path/issue_1901_poc1.xmp")
    filename2 = path("$data_path/issue_1901_poc2.xmp")
    filename3 = path("$data_path/issue_1901_poc3.xmp")
    filename4 = path("$data_path/issue_1901_poc4.xmp")
    commands = ["$exiv2 -q $filename1",
                "$exiv2 -q $filename2",
                "$exiv2 -q $filename3",
                "$exiv2 -q $filename4"]
    stderr = ["", "", "", ""]
    stdout = ["""File name       : $filename1
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

""",
              """File name       : $filename2
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

""",
              """File name       : $filename3
File size       : 160 Bytes
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

""",
              """File name       : $filename4
File size       : 154 Bytes
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
    retval = [0, 0, 0, 0]
