# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path
@CopyTmpFiles("$data_path/issue_1845_poc.jp2")

class TiffDirectoryWriteDirEntryAssert(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1845
    """
    url = "https://github.com/Exiv2/exiv2/issues/1845"

    filename = path("$tmp_path/issue_1845_poc.jp2")
    commands = ["$exiv2 -q -D +1 ad $filename"]
    stderr = [""]
    stdout = [""]
    retval = [0]
