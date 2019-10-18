# -*- coding: utf-8 -*-

import system_tests


class DivByZeroInPrintIFD(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/262"

    filename = system_tests.path(
        "$data_path/7-printIFD-divbyzero-1"
    )
    commands = ["$exiv2 -pX $filename"]
    stdout = [
        """STRUCTURE OF BIGTIFF FILE $filename
 address |    tag                           |      type |    count |    offset | value
      10 | 0x0008 FlashSetting              |   unknown |        0 |           | 
"""
    ]
    stderr = [
        """$exiv2_exception_message $filename:
$kerTiffParsingError
"""
    ]
    retval = [1]
