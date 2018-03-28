# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(system_tests.Case):

    url = "https://github.com/Exiv2/exiv2/issues/138"

    filename = "{data_path}/007-heap-buffer-over"
    commands = ["{exiv2} " + filename]
    stdout = [""]
    stderr = ["""{exiv2_exception_message} """ + filename + """:
{kerInvalidMalloc}
"""]
    retval = [1]
