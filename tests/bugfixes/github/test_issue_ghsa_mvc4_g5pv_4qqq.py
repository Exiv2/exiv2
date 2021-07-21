# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path
@CopyTmpFiles("$data_path/issue_ghsa_mvc4_g5pv_4qqq_poc.jpg")

class JpegBasePrintStructureInfiniteLoop(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-mvc4-g5pv-4qqq
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-mvc4-g5pv-4qqq"

    filename = path("$tmp_path/issue_ghsa_mvc4_g5pv_4qqq_poc.jpg")
    commands = ["$exiv2 -d I rm $filename"]
    stdout = [""]
    stderr = [
"""Exiv2 exception in erase action for file $filename:
$kerFailedToReadImageData
"""]
    retval = [1]
