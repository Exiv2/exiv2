# -*- coding: utf-8 -*-

import system_tests
from system_tests import CaseMeta, path, CopyTmpFiles, check_no_ASAN_UBSAN_errors

@CopyTmpFiles("$data_path/issue_1934_poc1.exv")
class TestExiv2ExtractThumbnailToStdout(metaclass=CaseMeta):
    """
    Regression test for 'extracting a thumbnail to stdout' bug described in:
    https://github.com/Exiv2/exiv2/issues/1934
    """
    url = "https://github.com/Exiv2/exiv2/issues/1934"

    filename1 = path("$tmp_path/issue_1934_poc1.exv")
    filename2 = path("$tmp_path/issue_1934_poc1-thumb.jpg")
    filename3 = path("$data_path/issue_1934_poc1-thumb.jpg")
    commands = ["$exiv2 --force --extract t- $filename1 > $filename2",
                "cmp $filename2 $filename3"
               ]
    stderr = [""]*2
    retval = [0]*2

    compare_stdout = check_no_ASAN_UBSAN_errors

