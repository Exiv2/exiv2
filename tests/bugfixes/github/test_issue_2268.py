# -*- coding: utf-8 -*-

from system_tests import CaseMeta, check_no_ASAN_UBSAN_errors


class issue_2268_jp2_assert(metaclass=CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/2268"
    filename = "$data_path/issue_2268_poc.jp2"
    commands = ["$exiv2 -pS $filename"]
    retval = [1]
    stderr = [
        """$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""
    ]
    compare_stdout = check_no_ASAN_UBSAN_errors
