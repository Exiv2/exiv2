# -*- coding: utf-8 -*-

import system_tests


class CVE_2017_14857(system_tests.Case):

    filename = "{data_path}/010_bad_free"
    commands = ["{exiv2} " + filename]
    retval = [1]
    stdout = [""]
    stderr = [
        """{exiv2_exception_msg} """ + filename + """:
{error_57_message}
"""]
