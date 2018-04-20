# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/187"

    filename = "$data_path/issue_187"
    commands = ["$exiv2 " + filename]
    retval = [1]
    stdout = [""]
    stderr = [
	"""$exiv2_exception_message """ + filename + """:
$kerFailedToReadImageData
"""
    ]
