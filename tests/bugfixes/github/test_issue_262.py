# -*- coding: utf-8 -*-

import system_tests


class DivByZeroInPrintIFD(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/262"

    filename = system_tests.path(
        "$data_path/7-printIFD-divbyzero-1"
    )
    commands = ["$exiv2 -pX $filename"]
    stdout = [""]
    stderr = [
        """$exiv2_exception_message $filename:
$filename: $kerFileContainsUnknownImageType
"""
    ]
    retval = [1]
