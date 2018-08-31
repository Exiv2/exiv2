# -*- coding: utf-8 -*-

import system_tests

class CheckNikonTimezoneWithoutCruft(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1062"

    filename = system_tests.path("$data_path/exiv2-bug1062.jpg")
    commands = [ "$exiv2 -pa -g zone $filename" ]

    stdout = [ """Exif.NikonWt.Timezone                        SShort      1  UTC -06:00
"""
    ]
    stderr = [""]
    retval = [0]
