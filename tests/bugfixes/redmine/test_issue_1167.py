# -*- coding: utf-8 -*-

import system_tests

class CheckSigma17_70Lens(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1167"

    filename = system_tests.path("$data_path/exiv2-bug1167.exv")

    commands = [ "$exiv2 -pa --grep lens/i $filename" ]

    stdout = [ """Exif.CanonCs.LensType                        Short       1  Sigma 17-70mm f/2.8-4 DC Macro OS HSM
Exif.CanonCs.Lens                            Short       3  17.0 - 70.0 mm
Exif.Canon.LensModel                         Ascii      74  17-70mm
Exif.Photo.LensSpecification                 Rational    4  17/1 70/1 0/1 0/1
Exif.Photo.LensModel                         Ascii       8  17-70mm
Exif.Photo.LensSerialNumber                  Ascii      11  0000000000
""",
    ]
    stderr = [""]
    retval = [0]
