# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path
@CopyTmpFiles("$data_path/issue_1815_poc.jpg")

class JpgImageDoWriteMetadataOutOfBoundsRead(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1815
    """
    url = "https://github.com/Exiv2/exiv2/issues/1815"

    filename = path("$tmp_path/issue_1815_poc.jpg")
    commands = ["$exiv2 rm $filename"]
    stdout = [""]
    stderr = [""]
    retval = [0]
