# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class Sony2FpTest(metaclass=CaseMeta):
    filename = path("$data_path/exiv2-pr906.exv")
    commands = ["$exiv2 -pa --grep Sony2Fp $filename"]

    stdout = [
        """Exif.Sony2Fp.AmbientTemperature              SByte       1  19 °C
Exif.Sony2Fp.FocusMode                       Byte        1  DMF
Exif.Sony2Fp.AFAreaMode                      Byte        1  Expanded Flexible Spot
Exif.Sony2Fp.FocusPosition2                  Byte        1  140
"""
    ]
    stderr = [""]
    retval = [0]
