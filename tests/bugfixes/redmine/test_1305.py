# -*- coding: utf-8 -*-

import system_tests


class Issue1305Test(metaclass=system_tests.CaseMeta):
    err_msg_dir_img = """Warning: Directory Image, entry 0x3030 has unknown Exif (TIFF) type 12336; setting type size 1.
Error: Directory Image, entry 0x3030 has invalid size 808464432*1; skipping entry.
"""

    err_msg_dir_ph = """Warning: Directory Photo, entry 0x3030 has unknown Exif (TIFF) type 12336; setting type size 1.
Error: Directory Photo, entry 0x3030 has invalid size 808464432*1; skipping entry.
"""

    err_msg_dir_pentax = """Warning: Directory Pentax, entry 0x3030 has unknown Exif (TIFF) type 12336; setting type size 1.
Error: Directory Pentax, entry 0x3030 has invalid size 808464432*1; skipping entry.
"""

    name = "regression test for issue 1305"
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

"""]

    stderr = [
        """Error: Directory Image: Next pointer is out of bounds; ignored.
"""
        + 8 * err_msg_dir_img
        + """Warning: Directory Photo has an unexpected next pointer; ignored.
"""
        + 13 * err_msg_dir_ph
        + """Warning: Directory Photo, entry 0x927c has unknown Exif (TIFF) type 12336; setting type size 1.
Warning: Directory Pentax has an unexpected next pointer; ignored.
"""
        + 6 * err_msg_dir_pentax
        + """Warning: Directory Pentax, entry 0x0006 has unknown Exif (TIFF) type 12336; setting type size 1.
Warning: Directory Pentax, entry 0x0007 has unknown Exif (TIFF) type 12336; setting type size 1.
"""
        + 39 * err_msg_dir_pentax
        + 23 * err_msg_dir_ph
        + """Warning: Directory Photo, entry 0x3030 has unknown Exif (TIFF) type 48; setting type size 1.
Error: Directory Photo, entry 0x3030 has invalid size 808464432*1; skipping entry.
"""
        + 5 * err_msg_dir_ph
        + """Warning: Directory Photo, entry 0x3030 has unknown Exif (TIFF) type 12336; setting type size 1.
Error: Directory Photo, entry 0x3030 has invalid size 808452102*1; skipping entry.
Warning: Directory Photo, entry 0x3030 has unknown Exif (TIFF) type 12336; setting type size 1.
Error: Directory Photo, entry 0x3030 has invalid size 808452103*1; skipping entry.
"""
        + 3 * err_msg_dir_ph
        + err_msg_dir_img
        + """Warning: Directory Image, entry 0x3030 has unknown Exif (TIFF) type 12336; setting type size 1.
Error: Directory Image, entry 0x3030 has invalid size 1414415696*1; skipping entry.
"""
        + 36 * err_msg_dir_img
        + """Warning: JPEG format error, rc = 5
"""]

    retval = [0]
