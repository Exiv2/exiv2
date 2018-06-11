# -*- coding: utf-8 -*-

import system_tests


class AdditionOverflowInLoaderTiffGetData(metaclass=system_tests.CaseMeta):
    """
    Regression test for bug #366:
    https://github.com/Exiv2/exiv2/issues/366
    """
    filename = system_tests.path("$data_path/2-out-of-read-Poc")
    commands = ["$exiv2 -ep $filename"]
    stdout = [""]
    stderr = [
        """Warning: Directory Image, entry 0x0111: Strip 0 is outside of the data area; ignored.
$uncaught_exception $addition_overflow_message
"""
    ]
    retval = [1]
