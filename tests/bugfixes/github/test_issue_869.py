# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class OutOfBoundsReadInIptcParserDecode(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/869
    """
    url = "https://github.com/Exiv2/exiv2/issues/869"

    filename = path("$data_path/issue_869_poc.png")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
        """Exiv2 exception in print action for file $filename:
Failed to read image data
"""
]
    retval = [1]
