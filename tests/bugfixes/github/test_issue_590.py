# -*- coding: utf-8 -*-

import system_tests


URL = "https://github.com/Exiv2/exiv2/issues/590"


@system_tests.CopyFiles("$data_path/tiffimage_int-out-of-bound-read-poc-2")
class TiffImageIntOutOfBoundsRead(metaclass=system_tests.CaseMeta):
    """
    Regression test for the second issue reported in #590.
    """

    filename = system_tests.path(
        "$data_path/tiffimage_int-out-of-bound-read-poc-2_copy"
    )

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors

    commands = ["$exiv2 -Y 2011 -O 02 -D 29 adjust $filename"]
    stdout = [""]
    stderr = [""]
    retval = [0]
