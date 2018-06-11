# -*- coding: utf-8 -*-

import system_tests


class AdditionOverflowInLoaderExifJpeg(metaclass=system_tests.CaseMeta):
    """
    Regression test for bug #365:
    https://github.com/Exiv2/exiv2/issues/365
    """
    filename = system_tests.path("$data_path/1-out-of-read-Poc")
    commands = ["$exiv2 -ep $filename"]
    stdout = [""]
    stderr = [
        """Error: Upper boundary of data for directory Image, entry 0x00fe is out of bounds: Offset = 0x0000002a, size = 64, exceeds buffer size by 22 Bytes; truncating the entry
Warning: Directory Image, entry 0x0201: Strip 0 is outside of the data area; ignored.
Warning: Directory Image, entry 0x0201: Strip 7 is outside of the data area; ignored.
Error: Offset of directory Thumbnail, entry 0x0201 is out of bounds: Offset = 0x00000000; truncating the entry
$uncaught_exception $addition_overflow_message
"""
    ]
    retval = [1]
