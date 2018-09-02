# -*- coding: utf-8 -*-

import system_tests


class PngReadRawProfile(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/428"

    filenames = [
        system_tests.path("$data_path/issue_428_poc1.png"),
        system_tests.path("$data_path/issue_428_poc2.png")
    ]

    commands = ["$exiv2 " + fname for fname in filenames]
    stdout = [""] * 2
    stderr = [
        """$exiv2_exception_message """ + fname + """:
$kerFailedToReadImageData
"""
        for fname in filenames
    ]
    retval = [1] * 2
