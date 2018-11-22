# -*- coding: utf-8 -*-

import system_tests
import unittest

@unittest.skip("Skipping test using option -pR (only for Debug mode)")
class UncontrolledRecursion(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/216"

    filename = system_tests.path(
        "$data_path/"
        "exiv2_0-26_exiv2_uncontrolled-recursion_printIFDStructure.tif"
    )

    commands = ["$exiv2 -pR $filename"]
    retval = [1]
    stdout = [
        """STRUCTURE OF TIFF FILE (MM): $filename
 address |    tag                              |      type |    count |    offset | value
      10 | 0x0100 ImageWidth                   |     SHORT |        1 |           | 1
      22 | 0x0103 Compression                  |     SHORT |        0 |           | 
      34 | 000000 GPSVersionID                 |      BYTE |        0 |           | """
    ]

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors
