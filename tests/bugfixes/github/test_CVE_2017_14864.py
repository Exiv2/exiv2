# -*- coding: utf-8 -*-

import system_tests

class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/73"

    filename = "$data_path/02-Invalid-mem-def"
    commands = ["$exiv2 -q " + filename]
    retval = [1]

    def compare_stderr(self, i, command, got_stderr, expected_stderr):
        """ Only check that an exception is thrown """
        self.assertIn(expected_stderr, got_stderr)
