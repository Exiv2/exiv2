# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(system_tests.Case):

    url = "https://github.com/Exiv2/exiv2/issues/76"

    filename = "{data_path}/010_bad_free"
    commands = ["{exiv2} " + filename]
    retval = [1]
    stdout = [""]
    stderr = [
        """{exiv2_exception_message} """ + filename + """:
{kerInvalidMalloc}
"""]
