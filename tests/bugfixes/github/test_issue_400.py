# -*- coding: utf-8 -*-

import system_tests


class parseTXTChunkOutOfBoundsRead(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/400"

    filenames = [
        system_tests.path("$data_path/issue_400_poc" + str(i)) for i in (1, 2)
    ]

    commands = ["$exiv2 " + fname for fname in filenames]
    stdout = [""] * 2
    stderr = [
        """$exiv2_exception_message """ + fname + """:
$kerCorruptedMetadata
"""
        for fname in filenames
    ]
    retval = [1] * 2
