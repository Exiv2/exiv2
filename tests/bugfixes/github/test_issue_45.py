# -*- coding: utf-8 -*-

import system_tests


class Sigma24_105mmRecognization(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/45"

    filename = "$data_path/exiv2-g45.exv"
    commands = ["$exiv2 -pa --grep lens/i " + filename]
    stdout = ["""Exif.CanonCs.LensType                        Short       1  Sigma 24-105mm F4 DG OS HSM | A
Exif.CanonCs.Lens                            Short       3  24.0 - 105.0 mm
Exif.CanonCf.LensAFStopButton                Short       1  0
Exif.Canon.LensModel                         Ascii      74  24-105mm F4 DG OS HSM | Art 013
"""]
    stderr = [""]
    retval = [0]
