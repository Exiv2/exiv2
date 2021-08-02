# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path, check_no_ASAN_UBSAN_errors

class ValueTypeFloatToLong(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1830
    """
    url = "https://github.com/Exiv2/exiv2/issues/1830"

    filename = path("$data_path/issue_1830_poc.tiff")
    commands = ["$exiv2 -q $filename"]
    stderr = [""]
    retval = [0]

    compare_stdout = check_no_ASAN_UBSAN_errors
