# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/144"

    filename = "$data_path/POC1"
    commands = ["$exiv2 $filename"]
    stdout = [
        """File name       : $filename
File size       : 26 Bytes
MIME type       : image/tiff
Image size      : 0 x 0
"""
    ]
    stderr = [""]
    retval = ["$no_exif_data_found_retval"]

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors
