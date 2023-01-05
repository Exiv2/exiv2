# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class InvalidDateXMP(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1713
    """
    url = "https://github.com/Exiv2/exiv2/issues/1713"

    filename = path("$data_path/issue_1713_poc.xmp")
    commands = ["$exiv2 -Ph $filename"]

    stderr = [
"""Error: XMP Toolkit error 201: Error in XMLValidator
Warning: Failed to decode XMP metadata.
"""
]
    retval = [0]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """ We don't care about the stdout, just don't crash """
        pass
