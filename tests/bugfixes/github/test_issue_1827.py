# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path, check_no_ASAN_UBSAN_errors

class ExposureTimeCastDoubleToLong(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1827
    """
    url = "https://github.com/Exiv2/exiv2/issues/1827"

    filename = path("$data_path/issue_1827_poc.crw")
    commands = ["$exiv2 $filename"]
    stderr = [""]
    retval = [0]

    compare_stdout = check_no_ASAN_UBSAN_errors
