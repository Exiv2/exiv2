# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors

class FloatToRationalCastOutOfRange(metaclass=CaseMeta):
    """
    Test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1838
    """
    url = "https://github.com/Exiv2/exiv2/issues/1838"

    filename = path("$data_path/issue_1838_poc.crw")
    commands = ["$exiv2 $filename"]
    stderr = [""]
    retval = [0]

    compare_stdout = check_no_ASAN_UBSAN_errors
