# -*- coding: utf-8 -*-

import system_tests


@system_tests.CopyFiles("$data_path/exiv2-empty.jpg")
class AssertionSvEqualsDFailedRegression(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/769"

    filename = "$data_path/exiv2-empty_copy.jpg"

    commands = [
        """$exiv2 -u -v -M"add Exif.Image.Make Canon" -M"add Exif.CanonCs.0x0001 Short 1" -M"add Exif.CanonCs.0x0000 Short 2" $filename""",
        "$exiv2 -u -v -PEkyct $filename"
    ]

    stdout = [
        """File 1/1: $filename
Add Exif.Image.Make "Canon" (Ascii)
Add Exif.CanonCs.0x0001 "1" (Short)
Add Exif.CanonCs.0x0000 "2" (Short)
""",
        """File 1/1: $filename
Exif.Image.Make                              Ascii       6  Canon
Exif.Image.ExifTag                           Long        1  44
Exif.Photo.MakerNote                         Undefined  18  1 0 1 0 3 0 2 0 0 0 4 0 1 0 0 0 0 0
Exif.MakerNote.Offset                        Long        1  62
Exif.MakerNote.ByteOrder                     Ascii       3  II
Exif.CanonCs.0x0000                          Short       1  4
Exif.CanonCs.Macro                           Short       1  On
"""
    ]

    stderr = [""] * 2
    retval = [0] * 2
