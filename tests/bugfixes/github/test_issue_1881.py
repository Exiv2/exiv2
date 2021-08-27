# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path
@CopyTmpFiles("$data_path/issue_1881_poc.jpg")

class SonyPreviewImageLargeAllocation(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1881
    """
    url = "https://github.com/Exiv2/exiv2/issues/1881"

    filename = path("$tmp_path/issue_1881_poc.jpg")
    commands = ["$exiv2 -q -d I rm $filename"]
    stdout = [""]
    stderr = [
"""Exiv2 exception in erase action for file $filename:
$kerCorruptedMetadata
"""]
    retval = [1]
