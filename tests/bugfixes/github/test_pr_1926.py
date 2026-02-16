# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class Sony2FpAmbientTemperatureUnsupportedTagTest(metaclass=CaseMeta):
    filename = path("$data_path/exiv2-bug1153Ja.exv")
    commands = ["$exiv2 -pa --grep AmbientTemperature $filename"]

    stdout = [
        """Exif.Sony2Fp.AmbientTemperature              SByte       1  n/a
"""
    ]
    stderr = [""]
    retval = [0]


class SonyMisc1CameraTemperatureUnsupportedTagTest(metaclass=CaseMeta):
    filename = path("$data_path/exiv2-bug1145a.exv")
    commands = ["$exiv2 -pa --grep SonyMisc1 $filename"]

    stdout = [
        """Exif.SonyMisc1.CameraTemperature             SByte       1  n/a
"""
    ]
    stderr = [""]
    retval = [0]
