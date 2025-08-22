# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors


class TiffMnEntryDoCountInvalidTiffType(metaclass=CaseMeta):
    """
    Test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1833
    """

    url = "https://github.com/Exiv2/exiv2/issues/1833"

    filename = path("$data_path/issue_1833_poc.jpg")
    commands = [f"$exiv2 -pS {filename}"]
    stderr = [
        f"""$exiv2_exception_message {filename}:
$kerFailedToReadImageData
"""
    ]
    retval = [1]

    compare_stdout = check_no_ASAN_UBSAN_errors
