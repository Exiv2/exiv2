# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors

class TiffMnEntryDoCountInvalidTiffType(metaclass=CaseMeta):
    """
    Test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1847
    """
    url = "https://github.com/Exiv2/exiv2/issues/1847"

    filename = path("$data_path/issue_1847_poc.jpg")
    commands = ["$exiv2 -pS $filename"]
    stderr = [""]
    retval = [0]

    compare_stdout = check_no_ASAN_UBSAN_errors
