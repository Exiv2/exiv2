# -*- coding: utf-8 -*-

import system_tests


class TestFirstPoC(metaclass=system_tests.CaseMeta):
    """
    Regression test for the two bugs described in:
    https://github.com/Exiv2/exiv2/issues/159

    We do not actually check the output of these files, we only check that we
    don't get a crash (the metadata are bogus anyway, so no point in checking).
    """
    url = "https://github.com/Exiv2/exiv2/issues/159"

    filename = [
        system_tests.path("$data_path/printStructure"),
        system_tests.path("$data_path/printStructure2")
    ]
    commands = [
        "$exiv2 " + filename[0],
        "$exiv2 -pS " + filename[1],
    ]
    retval = [0, 1]

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors
    compare_stdout = system_tests.check_no_ASAN_UBSAN_errors
