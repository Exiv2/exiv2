# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors


class QuickTimeVideoNikonTagsDecoderOutOfBoundsRead(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-g9xm-7538-mq8w
    """

    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-g9xm-7538-mq8w"

    filename = path("$data_path/issue_ghsa_g9xm_7538_mq8w_poc.mov")
    commands = ["$exiv2 $filename"]
    retval = [1]

    compare_stdout = check_no_ASAN_UBSAN_errors
    compare_stderr = check_no_ASAN_UBSAN_errors
