# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/306"

    filename = "$data_path/pocIssue306"
    commands = ["$exiv2 -et " + filename]
    retval = [1]
    stdout = [""]
    stderr = [
        """$exception_in_extract """ + filename + """:
$kerFailedToReadImageData
"""]
