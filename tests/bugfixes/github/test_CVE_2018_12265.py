# -*- coding: utf-8 -*-

import system_tests
from system_tests import check_no_ASAN_UBSAN_errors

class AdditionOverflowInLoaderExifJpeg(metaclass=system_tests.CaseMeta):
    """
    Regression test for bug #365:
    https://github.com/Exiv2/exiv2/issues/365
    aka CVE 2018-12265:
    https://cve.mitre.org/cgi-bin/cvename.cgi?name=2018-12265
    """
    filename = system_tests.path("$data_path/1-out-of-read-Poc")
    commands = ["$exiv2 -pS $filename"]
    retval = [1]

    compare_stdout = check_no_ASAN_UBSAN_errors

    def compare_stderr(self, i, command, got_stderr, expected_stderr):
        """ Only check that an exception is thrown """
        self.assertIn(expected_stderr, got_stderr)
