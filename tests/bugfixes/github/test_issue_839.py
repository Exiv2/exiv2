# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors


class OutOfMemoryInLoaderTiffGetData(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/839

    Due to a missing bounds check, this test triggers a 4GB memory
    allocation. So the test will fail with a std::bad_alloc exception
    if less than 4GB is available.  On Linux, you can use `ulimit -v
    4000000` to reduce the available memory to slightly less than 4GB.
    """
    url = "https://github.com/Exiv2/exiv2/issues/839"

    filename = path("$data_path/issue_839_poc.rw2")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
        """$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""]
    compare_stderr = check_no_ASAN_UBSAN_errors
    retval = [1]
