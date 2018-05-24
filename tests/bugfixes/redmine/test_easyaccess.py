# -*- coding: utf-8 -*-

import system_tests


@system_tests.CopyFiles("$data_path/exiv2-empty.jpg")
class EasyaccessTest(metaclass=system_tests.CaseMeta):

    filename = "$data_path/exiv2-empty_copy.jpg"

    commands = [
        """$exiv2 -u -v -M"set Exif.Image.Make Samsung" $filename""",
        "$easyaccess_test $filename"
    ]
    stdout = [
        """File 1/1: $filename
Set Exif.Image.Make "Samsung" (Ascii)
""",
         """Orientation          (                                   ) : 
ISO speed            (                                   ) : 
Flash bias           (                                   ) : 
Exposure mode        (                                   ) : 
Scene mode           (                                   ) : 
Macro mode           (                                   ) : 
Image quality        (                                   ) : 
White balance        (                                   ) : 
Lens name            (                                   ) : 
Saturation           (                                   ) : 
Sharpness            (                                   ) : 
Contrast             (                                   ) : 
Scene capture type   (                                   ) : 
Metering mode        (                                   ) : 
Camera make          (Exif.Image.Make                    ) : Samsung
Camera model         (                                   ) : 
Exposure time        (                                   ) : 
FNumber              (                                   ) : 
Subject distance     (                                   ) : 
Camera serial number (                                   ) : 
Focal length         (                                   ) : 
AF point             (                                   ) : 
"""
    ]
    stderr = [""] * 2
    retval = [0] * 2
