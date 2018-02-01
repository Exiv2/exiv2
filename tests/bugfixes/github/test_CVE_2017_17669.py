# -*- coding: utf-8 -*-

import system_tests


class RunPocFile(system_tests.Case):

    filename = "{data_path}/issue_187"
    commands = ["{exiv2} " + filename]
    retval = [1]
    stdout = [""]
    stderr = [
	"""{exiv2_exception_message} """ + filename + """:
{kerFailedToReadImageData}
"""
    ]
