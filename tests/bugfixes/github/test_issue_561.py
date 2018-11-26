# -*- coding: utf-8 -*-

import system_tests

class ShouldNotThrowsWithSpecificIsoSpeedValue(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/561"

    filename = system_tests.path("$data_path/pocIssue561")
    commands = ["$exiv2 " + filename]
    retval = [0]
    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors
    compare_stdout = system_tests.check_no_ASAN_UBSAN_errors
