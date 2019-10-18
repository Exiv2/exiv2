# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/208"

    filename = "$data_path/2018-01-09-exiv2-crash-001.tiff"
    commands = ["$exiv2 " + filename]
    retval = [1]
    stdout = [""]
    stderr = [
        """$exiv2_exception_message """ + filename + """:
$kerTiffParsingError
"""]
