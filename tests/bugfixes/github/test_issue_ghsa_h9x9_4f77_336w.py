# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path, check_no_ASAN_UBSAN_errors

class Jp2ImageEncodeJp2HeaderOutOfBoundsRead2(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-h9x9-4f77-336w
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-h9x9-4f77-336w"

    filename = path("$data_path/issue_ghsa_h9x9_4f77_336w_poc.exv")
    commands = ["$exiv2 -P t $filename"]
    retval = [0]

    compare_stdout = check_no_ASAN_UBSAN_errors
    compare_stderr = check_no_ASAN_UBSAN_errors
