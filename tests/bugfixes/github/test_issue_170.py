# -*- coding: utf-8 -*-

import system_tests


class decodeIHDRChunkOutOfBoundsRead(system_tests.Case):

    url = "https://github.com/Exiv2/exiv2/issues/170"

    filename = "{data_path}/issue_170_poc"

    commands = ["{exiv2} " + filename]
    stdout = [""]
    stderr = ["""{exiv2_exception_msg} """ + filename + """:
{error_14_message}
"""]
    retval = [1]
