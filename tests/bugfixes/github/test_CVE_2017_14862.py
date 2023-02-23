# -*- coding: utf-8 -*-

import ctypes
import system_tests
from system_tests import check_no_ASAN_UBSAN_errors

class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/75"

    filename = "$data_path/008-invalid-mem"
    commands = ["$exiv2 -q " + filename]

    if ctypes.sizeof(ctypes.c_voidp) > 4:
        stderr = [""]
        retval = [0]
    else:
        stderr = ["""$exiv2_exception_message """ + filename + """:
$kerCorruptedMetadata
"""]
        retval = [1]

    compare_stdout = check_no_ASAN_UBSAN_errors

