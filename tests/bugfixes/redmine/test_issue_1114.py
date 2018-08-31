# -*- coding: utf-8 -*-

import system_tests

class CheckNegativeValuesOfSByte(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1114"

    filename = system_tests.path("$data_path/exiv2-bug1114.jpg")
    commands = [ "$exiv2 -pv -g TuneAdj $filename" ]

    stdout = [ """0x0002 NikonAFT     AFFineTuneAdj               SByte       1  -2
"""
    ]
    stderr = [""]
    retval = [0]
