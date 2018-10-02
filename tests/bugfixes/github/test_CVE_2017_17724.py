# -*- coding: utf-8 -*-

import system_tests


class TestFuzzedPoC(metaclass=system_tests.CaseMeta):

    url = [
        "https://github.com/Exiv2/exiv2/issues/210",
        "https://github.com/Exiv2/exiv2/issues/209"
    ]

    filename = system_tests.path("$data_path/2018-01-09-exiv2-crash-002.tiff")
    commands = [
        "$exiv2 -pR $filename",
        "$exiv2 -pS $filename",
        "$exiv2 $filename"
    ]
    retval = [1, 1, 0]

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """ We don't care about the stdout, just don't crash """
        pass
