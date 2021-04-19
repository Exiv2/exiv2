# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class BmffImageReadMetadataOutOfBoundsRead(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1570
    """
    url = "https://github.com/Exiv2/exiv2/issues/1570"

    filename = path("$data_path/issue_1570_poc.bmff")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
"""$filename1: Could not write metadata to file: $kerCorruptedMetadata
"""]
    retval = [1]
