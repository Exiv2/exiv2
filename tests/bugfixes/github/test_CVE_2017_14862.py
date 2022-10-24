# -*- coding: utf-8 -*-

import system_tests
from system_tests import check_no_ASAN_UBSAN_errors

class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/75"

    filename = "$data_path/008-invalid-mem"
    commands = ["$exiv2 -q " + filename]

    stderr = [""]
    retval = [0]

    compare_stdout = check_no_ASAN_UBSAN_errors

