# -*- coding: utf-8 -*-

import system_tests
from system_tests import check_no_ASAN_UBSAN_errors


class TestCvePoC(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/75"

    filename = "$data_path/008-invalid-mem"
    commands = [f"$exiv2 -q {filename}"]

    if system_tests.BT.Config.is_64bit:
        stderr = [""]
        retval = [0]
    else:
        stderr = [
            f"""$exiv2_exception_message {filename}:
$kerCorruptedMetadata
"""
        ]
        retval = [1]

    compare_stdout = check_no_ASAN_UBSAN_errors
