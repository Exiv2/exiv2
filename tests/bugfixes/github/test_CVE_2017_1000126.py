# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/175"

    filename = "$data_path/cve_2017_1000126_stack-oob-read.webp"
    commands = ["$exiv2 " + filename]
    stdout = [""]
    stderr = ["""$exiv2_exception_message """ + filename + """:
$kerTiffParsingError
"""]
    retval = [1]
