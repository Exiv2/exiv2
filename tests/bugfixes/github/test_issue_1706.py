# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class InvalidDateXMP(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1706
    """
    url = "https://github.com/Exiv2/exiv2/issues/1706"

    filename = path("$data_path/issue_1706_poc.exv")
    commands = ["$exiv2 -PE $filename"]

    stderr = [
"""Error: Directory Photo with 65280 entries considered invalid; not read.
"""
]
    retval = [0]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        # Check that it printed "Bad value" for the date.
        self.assertRegex(got_stdout, "Exif.PentaxDng.Date\\s+Long\\s+1\\s+Bad value")
