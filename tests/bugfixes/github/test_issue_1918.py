# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors

class PrintXmpDateOutOfBoundsIndex19(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1918
    """
    url = "https://github.com/Exiv2/exiv2/issues/1918"

    filename = path("$data_path/issue_1918_poc.jpg")
    commands = ["$exiv2 -px $filename"]
    stderr = [""]
    retval = [0]

    compare_stdout = check_no_ASAN_UBSAN_errors
