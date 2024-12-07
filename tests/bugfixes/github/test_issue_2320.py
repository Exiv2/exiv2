# -*- coding: utf-8 -*-

from system_tests import CaseMeta, check_no_ASAN_UBSAN_errors


class issue_2320_printDegrees_integer_overflow(metaclass=CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/2320"
    filename = "$data_path/issue_2320_poc.jpg"
    commands = ["$exiv2 -q -pa $filename"]
    retval = [0]
    stderr = [""]
    compare_stdout = check_no_ASAN_UBSAN_errors
