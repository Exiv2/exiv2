# -*- coding: utf-8 -*-

import system_tests


class CheckBarometer(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1225"
    filename = "$data_path/exiv2-bug1225.exv"

    commands = [
        "$exiv2 -pa --grep Bar $filename"
    ]

    stdout = [
        "Exif.Nikon3.BarometerInfo                    Undefined  12  1583\n"
    ]

    stderr = [""]
    retval = [0]
