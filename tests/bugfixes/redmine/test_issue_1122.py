# -*- coding: utf-8 -*-

import system_tests

class CheckLensDetectionTeleconverter(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1122"

    filename = system_tests.path("$data_path/exiv2-bug1122.exv")
    commands = [ "$exiv2 -pv -g Lens $filename",
                 "$exiv2 -pa -g Lens $filename",
               ]

    stdout = [ """0x0016 CanonCs      LensType                    Short       1  173
0x0017 CanonCs      Lens                        Short       3  1000 300 1
0x0095 Canon        LensModel                   Ascii      74  300-1000mm
0xa432 Photo        LensSpecification           Rational    4  300/1 1000/1 0/1 0/1
0xa434 Photo        LensModel                   Ascii      11  300-1000mm
0xa435 Photo        LensSerialNumber            Ascii      11  0000000000
""",
    """Exif.CanonCs.LensType                        Short       1  Sigma 150-500mm f/5-6.3 APO DG OS HSM + 2x
Exif.CanonCs.Lens                            Short       3  300.0 - 1000.0 mm
Exif.Canon.LensModel                         Ascii      74  300-1000mm
Exif.Photo.LensSpecification                 Rational    4  300/1 1000/1 0/1 0/1
Exif.Photo.LensModel                         Ascii      11  300-1000mm
Exif.Photo.LensSerialNumber                  Ascii      11  0000000000
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)
