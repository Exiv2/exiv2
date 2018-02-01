# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(system_tests.Case):

    url = "https://github.com/Exiv2/exiv2/issues/73"

    filename = "{data_path}/003-heap-buffer-over"
    commands = ["{exiv2} " + filename]
    stdout = [""]
    stderr = ["""{exiv2_exception_message} """ + filename + """:
{kerCorruptedMetadata}
"""]
    retval = [1]
