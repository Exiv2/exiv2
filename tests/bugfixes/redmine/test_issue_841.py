# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class ThrowOnInput(metaclass=CaseMeta):

    url = "http://dev.exiv2.org/issues/841"

    filename = path("$data_path/exiv2-bug841.png")

    commands = ["$exiv2 $filename"]

    stdout = [""]
    stderr = [
        """$exiv2_exception_message $filename:
$kerFailedToReadImageData
"""
    ]
    retval = [1]
