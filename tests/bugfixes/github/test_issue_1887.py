# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors


class OutOfBoundsReadInCrwMapDecode0x080a(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1887
    """
    url = "https://github.com/Exiv2/exiv2/issues/1887"

    filename = path("$data_path/issue_1887_poc.crw")
    commands = ["$exiv2 $filename"]
    stderr = [""]
    retval = [0]

    compare_stdout = check_no_ASAN_UBSAN_errors
