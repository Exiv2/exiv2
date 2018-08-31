# -*- coding: utf-8 -*-

import system_tests

class CheckSigma35mm(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1170"

    filename = system_tests.path("$data_path/exiv2-bug1170.exv")

    commands = [ "$exiv2 -pa --grep lens/i $filename" ]

    stdout = [ """Exif.CanonCs.LensType                        Short       1  Sigma 35mm f/1.4 DG HSM
Exif.CanonCs.Lens                            Short       3  35.0 mm
Exif.Canon.LensModel                         Ascii      74  35mm
Exif.Photo.LensSpecification                 Rational    4  35/1 35/1 0/1 0/1
Exif.Photo.LensModel                         Ascii       5  35mm
Exif.Photo.LensSerialNumber                  Ascii      11  0000000000
""",
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)
