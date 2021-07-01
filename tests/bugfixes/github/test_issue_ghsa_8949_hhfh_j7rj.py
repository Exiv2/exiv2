# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path
@CopyTmpFiles("$data_path/issue_ghsa_8949_hhfh_j7rj_poc.jp2","$data_path/issue_ghsa_8949_hhfh_j7rj_poc.exv")

class Jp2ImageEncodeJp2HeaderOutOfBoundsRead(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-8949-hhfh-j7rj
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-8949-hhfh-j7rj"

    filename1 = path("$tmp_path/issue_ghsa_8949_hhfh_j7rj_poc.jp2")
    filename2 = path("$tmp_path/issue_ghsa_8949_hhfh_j7rj_poc.exv")
    commands = ["$exiv2 in $filename1"]
    stdout = [""]
    stderr = [
"""Error: XMP Toolkit error 201: XML parsing failure
Warning: Failed to decode XMP metadata.
"""]
    retval = [0]
