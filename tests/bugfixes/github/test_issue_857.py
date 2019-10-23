# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class OutOfMemoryInRafImageReadMetadata(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/857

    There is no bounds check on the value of jpg_img_len in
    RafImage::readMetadata(), leading to an out-of-memory error.
    """
    url = "https://github.com/Exiv2/exiv2/issues/857"

    filename1 = path("$data_path/issue_857_poc.raf")
    filename2 = path("$data_path/issue_857_coverage.raf")
    commands = ["$exiv2 $filename1", "$exiv2 $filename2"]
    stdout = ["", ""]
    stderr = [
"""Exiv2 exception in print action for file $filename1:
$kerCorruptedMetadata
""",
"""Exiv2 exception in print action for file $filename2:
$kerCorruptedMetadata
"""
]
    retval = [1,1]
