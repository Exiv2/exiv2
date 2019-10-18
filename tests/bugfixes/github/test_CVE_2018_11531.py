# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors


class TestCvePoC(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/283
    """
    url = "https://github.com/Exiv2/exiv2/issues/283"
    cve_url = "https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2018-11531"

    def check_no_ASAN_UBSAN_errors(self, i, command, got_stderr, expected_stderr):
        """ Override of system_tests.check_no_ASAN_UBSAN_errors for this particular test case.

        Here we want to also check that the two last lines of got_stderr have the expected_stderr
        """

        check_no_ASAN_UBSAN_errors(self, i, command, got_stderr, expected_stderr)
        self.assertListEqual(expected_stderr.splitlines(), got_stderr.splitlines()[-2:])

    filename = path("$data_path/pocIssue283.jpg")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
        """$exiv2_exception_message $filename:
$kerTiffParsingError
"""]
    compare_stderr = check_no_ASAN_UBSAN_errors
    retval = [1]
