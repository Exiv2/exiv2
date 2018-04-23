# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/57"

    filename = "$data_path/POC"
    commands = ["$exiv2 " + filename]
    stdout = [""]
    stderr = ["""$kerInvalidTypeValue:  0
$exiv2_exception_message """ + filename + """:
$kerInvalidTypeValue
"""]
    retval = [1]
