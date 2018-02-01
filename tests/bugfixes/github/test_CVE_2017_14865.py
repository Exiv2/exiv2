# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(system_tests.Case):

    url = "https://github.com/Exiv2/exiv2/issues/134"

    filename = "{data_path}/004-heap-buffer-over"
    commands = ["{exiv2} " + filename]
    stdout = [""]
    stderr = ["""{kerInvalidTypeValue}:  250
{exiv2_exception_message} """ + filename + """:
{kerInvalidTypeValue}
"""]
    retval = [1]
