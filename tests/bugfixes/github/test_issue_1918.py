# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors

class PrintXmpDateOutOfBoundsIndex19(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1918
    """
    url = "https://github.com/Exiv2/exiv2/issues/1918"

    filename1 = path("$data_path/issue_1918_poc1.jpg")
    filename2 = path("$data_path/issue_1918_poc2.jpg")
    filename3 = path("$data_path/issue_1918_poc3.jpg")
    commands = ["$exiv2 -px $filename1", "$exiv2 -px $filename2", "$exiv2 -px $filename3"]
    stderr = ["", "", ""]
    retval = [0, 0, 0]

    compare_stdout = check_no_ASAN_UBSAN_errors
