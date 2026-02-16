# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class BrotliUncompressOutOfBoundsWrite(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-hrw9-ggg3-3r4r
    """

    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-hrw9-ggg3-3r4r"

    filename = path("$data_path/issue_ghsa_hrw9_ggg3_3r4r_poc.jpg")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
        """Exiv2 exception in print action for file $filename:
$kerFailedToReadImageData
"""
    ]
    retval = [1]
