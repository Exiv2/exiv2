# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors


class TagsIntPrint0x0007IntegerOverflow(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1912
    """

    url = "https://github.com/Exiv2/exiv2/issues/1912"

    filename = path("$data_path/issue_1912_poc.jpg")
    commands = ["$exiv2 -q -Pt $filename"]
    stderr = [""]
    retval = [0]

    compare_stdout = check_no_ASAN_UBSAN_errors
