# -*- coding: utf-8 -*-

import system_tests


class CanonLenses(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1252"
    filenames = [
        "$data_path/exiv2-bug1252a.exv",
        "$data_path/exiv2-bug1252b.exv",
    ]

    commands = [
        f"$exiv2 -pa --grep lens/i {filenames[0]}",
        f"$exiv2 -pa --grep lens/i {filenames[1]}",
    ]

    stdout = ["""Exif.CanonCs.LensType                        Short       1  Sigma APO 120-300mm f/2.8 EX DG OS HSM *OR* Sigma 120-300mm f/2.8 DG OS HSM S013
Exif.CanonCs.Lens                            Short       3  120.0 - 300.0 mm
Exif.Canon.LensModel                         Ascii      74  120-300mm
Exif.CanonLe.LensSerialNumber                Byte        5  0000000000
Exif.CanonAfC.USMLensElectronicMF            SLong       1  Enable After AF
Exif.CanonAfC.LensDriveWhenAFImpossible      SLong       1  Continue Focus Search
Exif.Photo.LensSpecification                 Rational    4  120-300mm
Exif.Photo.LensModel                         Ascii      10  120-300mm
Exif.Photo.LensSerialNumber                  Ascii      11  0000000000
""",
              """Exif.CanonCs.LensType                        Short       1  Sigma 150-500mm f/5-6.3 APO DG OS HSM
Exif.CanonCs.Lens                            Short       3  150.0 - 500.0 mm
Exif.Canon.LensModel                         Ascii      74  150-500mm
Exif.CanonLe.LensSerialNumber                Byte        5  0000000000
Exif.CanonAfC.USMLensElectronicMF            SLong       1  Enable After AF
Exif.CanonAfC.LensDriveWhenAFImpossible      SLong       1  Continue Focus Search
Exif.Photo.LensSpecification                 Rational    4  150-500mm
Exif.Photo.LensModel                         Ascii      10  150-500mm
Exif.Photo.LensSerialNumber                  Ascii      11  0000000000
"""
    ]

    stderr = [""] * len(commands)
    retval = [0]  * len(commands)
