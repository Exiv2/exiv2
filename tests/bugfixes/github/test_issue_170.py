# -*- coding: utf-8 -*-

import system_tests


class DecodeIHDRChunkOutOfBoundsRead(system_tests.Case):

    url = "https://github.com/Exiv2/exiv2/issues/170"

    filename = "{data_path}/issue_170_poc"

    commands = ["{exiv2} " + filename]
    stdout = [""]
    stderr = ["""{exiv2_exception_message} """ + filename + """:
{kerFailedToReadImageData}
"""]
    retval = [1]
