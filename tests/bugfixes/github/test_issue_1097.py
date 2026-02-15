# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors


class InfiniteRecursionInCiffDirectoryReadDirectory(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1097

    A malicious file can cause an infinite recursion.
    """
    url = "https://github.com/Exiv2/exiv2/issues/1097"

    filename = path("$data_path/issue_1097_poc.crw")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
        """$exiv2_exception_message $filename:
$kerOffsetOutOfRange
"""]
    retval = [1]
