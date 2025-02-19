# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class TiffSubIfd_use_after_free(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-38h4-fx85-qcx7
    """

    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-38h4-fx85-qcx7"

    filename = path("$data_path/issue_ghsa_38h4_fx85_qcx7_poc.tiff")
    commands = ["$exiv2 -q fi $filename"]
    stdout = [""]
    stderr = [
        """Exiv2 exception in fixiso action for file $filename:
$kerImageWriteFailed
"""
    ]
    retval = [1]
