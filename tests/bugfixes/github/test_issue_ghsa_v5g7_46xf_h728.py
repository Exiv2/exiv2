# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path, check_no_ASAN_UBSAN_errors

class Jp2ImageEncodeJp2HeaderOutOfBoundsRead2(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-v5g7-46xf-h728
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-v5g7-46xf-h728"

    filename = path("$data_path/issue_ghsa_v5g7_46xf_h728_poc.exv")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = ["""Exiv2 exception in print action for file $filename:
Invalid XmpText type `'
"""]
    retval = [1]
