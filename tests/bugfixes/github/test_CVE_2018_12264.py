# -*- coding: utf-8 -*-

import system_tests


class AdditionOverflowInLoaderTiffGetData(metaclass=system_tests.CaseMeta):
    """
    Regression test for bug #366:
    https://github.com/Exiv2/exiv2/issues/366
    aka CVE-2018-12264
    https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2018-12264
    """
    filename = "$data_path/2-out-of-read-Poc"
    commands = ["$exiv2 -ep $filename"]
    stdout = [""]
    stderr = [
        """Warning: Directory Image, entry 0x0111: Strip 0 is outside of the data area; ignored.
$uncaught_exception Overflow in addition
"""
    ]
    retval = [1]
