# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class SonyMisc1Test(metaclass=CaseMeta):
    filename = path("$data_path/exiv2-pr906.exv")
    commands = ["$exiv2 -pa --grep SonyMisc1 $filename"]

    stdout = [
        """Exif.SonyMisc1.CameraTemperature             SByte       1  26 °C
"""
    ]
    stderr = [""]
    retval = [0]
