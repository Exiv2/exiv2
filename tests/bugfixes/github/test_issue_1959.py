# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors

class XmpIptcStandardsTest(metaclass=CaseMeta):
    """
    Regression test for the issue described in:
    https://github.com/Exiv2/exiv2/issues/1959
    """
    url = "https://github.com/Exiv2/exiv2/issues/1959"

    filename      = path("$data_path/issue_1959_poc.xmp")
    filename_save = path("$tmp_path/issue_1959_poc.xmp_save.out")
    filename_out  = path("$data_path/issue_1959_poc.xmp.out")
    
    commands = ["$exiv2 -Pkvt $filename > $filename_save", "diff --strip-trailing-cr $filename_save $filename_out"]

    stderr = [""]*2
    retval = [0]*2
    compare_stdout = check_no_ASAN_UBSAN_errors
