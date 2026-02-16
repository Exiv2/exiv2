# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class TestVerboseExtractStdoutMultiFile(metaclass=CaseMeta):
    filename = path("$data_path/exiv2-empty.jpg")
    commands = ["$exiv2 --verbose --extract X- $filename $filename"]

    stdout = [""""""]

    stderr = [
        """exiv2: Only one file is allowed when extracting to stdout
"""
    ]
    retval = [1]
