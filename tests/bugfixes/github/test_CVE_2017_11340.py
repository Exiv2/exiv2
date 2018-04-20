# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/53"

    filename = "$data_path/POC6"
    commands = ["$exiv2 " + filename]
    stdout = [""]
    stderr = ["""$exiv2_exception_message """ + filename + """:
$kerInvalidMalloc
"""]
    retval = [1]
