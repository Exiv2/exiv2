# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class MrmImageLargeAllocation(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/pull/943
    """
    url = "https://github.com/Exiv2/exiv2/pull/943"

    filename1 = path("$data_path/issue_943_poc1.mrm")
    filename2 = path("$data_path/issue_943_poc2.mrm")
    commands = ["$exiv2 $filename1", "$exiv2 $filename2"]
    stdout = ["",""]
    stderr = [
"""Exiv2 exception in print action for file $filename1:
Failed to read image data
""",
"""Exiv2 exception in print action for file $filename2:
Failed to read image data
"""
]
    retval = [1,1]
