# -*- coding: utf-8 -*-

import system_tests


class TestPoC(system_tests.Case):

    url = "https://github.com/Exiv2/exiv2/issues/176"

    filename = "{data_path}/heap-oob-write.tiff"
    commands = ["{exiv2} " + filename]
    stdout = [""]
    stderr = ["""{exiv2_exception_msg} """ + filename + """:
{kerInvalidMalloc}
"""]
    retval = [1]
