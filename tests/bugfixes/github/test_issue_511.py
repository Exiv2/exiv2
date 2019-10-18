# -*- coding: utf-8 -*-

import system_tests
import unittest

@unittest.skip("Skipping test using option -pR (only for Debug mode)")
class ThrowsWhenIFDsAreMalformed(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/511"

    filename = system_tests.path("$data_path/pocIssue511")
    commands = ["$exiv2 -pR " + filename]
    retval = [1]
    stdout = ["""STRUCTURE OF TIFF FILE (II): """ + filename + """
"""]
    stderr = ["""$exiv2_exception_message """ + filename + """:
$kerTiffParsingError
"""]
