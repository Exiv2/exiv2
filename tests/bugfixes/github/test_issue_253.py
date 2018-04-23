# -*- coding: utf-8 -*-

import system_tests


class TestFirstPoC(metaclass=system_tests.CaseMeta):
    """
    Regression test for the first bug described in:
    https://github.com/Exiv2/exiv2/issues/253
    """
    url = "https://github.com/Exiv2/exiv2/issues/253"

    filename = "$data_path/3-stringformat-outofbound-read"
    commands = ["$exiv2 " + filename]
    stdout = [""]
    stderr = ["""$exiv2_exception_message """ + filename + """:
$kerNotAJpeg
"""]
    retval = [1]
