# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors


class PentaxMakerNotePrintTimeSignedLeftShift(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1920
    """

    url = "https://github.com/Exiv2/exiv2/issues/1920"

    filename = path("$data_path/issue_1920_poc.tiff")
    commands = ["$exiv2 -q -Pt $filename"]
    stderr = [""]
    retval = [0]

    compare_stdout = check_no_ASAN_UBSAN_errors
