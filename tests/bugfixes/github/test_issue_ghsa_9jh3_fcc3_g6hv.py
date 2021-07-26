# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path
@CopyTmpFiles("$data_path/issue_ghsa_9jh3_fcc3_g6hv_poc.jpg")

class JpegBasePrintStructureInfiniteLoop(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-9jh3-fcc3-g6hv
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-9jh3-fcc3-g6hv"

    filename = path("$tmp_path/issue_ghsa_9jh3_fcc3_g6hv_poc.jpg")
    commands = ["$exiv2 -d I rm $filename"]
    stdout = [""]
    stderr = [""]
    retval = [0]
