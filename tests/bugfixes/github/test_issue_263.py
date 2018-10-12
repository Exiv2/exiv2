# -*- coding: utf-8 -*-

import system_tests


class OutOfBoundsReadInPrintStructure(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/263"

    filenames = [
        system_tests.path("$data_path/9-printStructure-outbound-read-1"),
        system_tests.path("$data_path/10-printStructure-outbound-read-2")
    ]

    commands = ["$exiv2 -pR " + fname for fname in filenames]

    retval = [1] * 2

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors
    compare_stdout = system_tests.check_no_ASAN_UBSAN_errors
