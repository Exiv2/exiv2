# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class QuickTimeVideoNikonTagsDecoderOutOfBoundsRead(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-crmj-qh74-2r36
    """

    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-crmj-qh74-2r36"

    filename = path("$data_path/issue_ghsa_crmj_qh74_2r36_poc.mov")
    commands = ["$exiv2 $filename"]
    retval = [1]
    stderr = [
        """$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""
    ]
    stdout = [""]
