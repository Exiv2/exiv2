# -*- coding: utf-8 -*-

import system_tests
import os.path

class TestFirstPoC(metaclass=system_tests.CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/283
    """
    url = "https://github.com/Exiv2/exiv2/issues/283"

    def check_no_ASAN_UBSAN_errors(self, i, command, got_stderr, expected_stderr):
        """ Override of system_tests.check_no_ASAN_UBSAN_errors for this particular test case.

        Here we want to also check that the two last lines of got_stderr have the expected_stderr
        """

        system_tests.check_no_ASAN_UBSAN_errors(self, i, command, got_stderr, expected_stderr)
        self.assertListEqual(expected_stderr.splitlines(), got_stderr.splitlines()[-2:])

    filename = os.path.join("$data_path", "pocIssue283.jpg")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
        """$exiv2_exception_message """ + filename + """:
$kerCorruptedMetadata
"""]
    compare_stderr = check_no_ASAN_UBSAN_errors
    retval = [1]

