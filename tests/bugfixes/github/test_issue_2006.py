# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class PanasonicMakerPrintAccelerometerIntOverflow(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/2006
    """
    url = "https://github.com/Exiv2/exiv2/issues/2006"

    filename = path("$data_path/issue_2006_poc.tiff")
    commands = ["$exiv2 -q -PE $filename"]
    stderr = [""]
    stdout = ["""Exif.Image.Make                              Ascii      32  Panasonic   
Exif.Image.DNGPrivateData                    0x2020     32  80 97 110 97 115 111 110 105 99 32 32 32 0 32 32 255 32 32 32 32 32 255 255 255 32 255 255 198 52 32 32 0
Exif.MakerNote.Offset                        Long        1  48
Exif.MakerNote.ByteOrder                     Ascii       3  MM
Exif.Panasonic.AccelerometerY                SLong       4  2147483425
"""]
    retval = [0]
