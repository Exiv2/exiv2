# -*- coding: utf-8 -*-

import system_tests


class CheckShutterInPentax(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1223"
    filenames = ["$data_path/RAW_PENTAX_K30.exv",
                 "$data_path/RAW_PENTAX_K100.exv"]

    commands = [
        "$exiv2 -pa --grep Shutter " + filenames[0],
        "$exiv2 -pa --grep Shutter " + filenames[1],
    ]

    stdout = [
        "Exif.PentaxDng.ShutterCount                  Undefined   4  115\n",
        "Exif.Pentax.ShutterCount                     Undefined   4  24757\n",
    ]

    stderr = [""] * len(stdout)
    retval = [0] * len(stdout)
