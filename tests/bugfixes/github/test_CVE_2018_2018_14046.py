# -*- coding: utf-8 -*-

import system_tests


class WebpDecodeChunksOutOfBoundsRead(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/378"

    filename = "$data_path/issue_378_1-poc-heapoverflow"

    commands = ["$exiv2 -pp $filename"]
    stdout = [""]
    stderr = [
        """$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""
    ]
    retval = [1]
