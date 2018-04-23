# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/56"

    filename = "$data_path/POC9"
    commands = ["$exiv2 " + filename]
    stdout = [""""""]
    stderr = ["""$exiv2_exception_message """ + filename + """:
$kerInvalidMalloc
"""]
    retval = [1]
