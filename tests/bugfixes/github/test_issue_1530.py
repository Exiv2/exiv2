# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class CrwEncode0x1810IntegerOverflow(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1530
    """
    url = "https://github.com/Exiv2/exiv2/issues/1530"

    filename1 = path("$data_path/issue_1530_poc.crw")
    filename2 = path("$data_path/issue_1530_poc.exv")
    commands = ["$exiv2 in $filename1 $filename2"]
    stdout = [""]
    stderr = [
"""$filename1: Could not write metadata to file: $kerCorruptedMetadata
"""]
    retval = [1]
