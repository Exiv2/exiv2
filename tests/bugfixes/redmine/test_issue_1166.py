# -*- coding: utf-8 -*-

import system_tests

class CheckTokina11_20mm(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1166"

    filename = system_tests.path("$data_path/exiv2-bug1166.exv")

    commands = [ "$exiv2 -pa --grep lens/i $filename" ]

    stdout = [ """Exif.CanonCs.LensType                        Short       1  Tokina AT-X 11-20 F2.8 PRO DX Aspherical 11-20mm f/2.8
Exif.CanonCs.Lens                            Short       3  11.0 - 20.0 mm
Exif.Canon.LensModel                         Ascii      74  11-20mm
Exif.Photo.LensSpecification                 Rational    4  11/1 20/1 0/1 0/1
Exif.Photo.LensModel                         Ascii       8  11-20mm
Exif.Photo.LensSerialNumber                  Ascii      11  0000000000
""",
    ]
    stderr = [""]
    retval = [0]
