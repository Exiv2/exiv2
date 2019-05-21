# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class Issue743NikonSigmaArtLens(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/743
    """
    url = "https://github.com/Exiv2/exiv2/issues/743"

    filename = path("$data_path/issue_743.exv")
    commands = ["$exiv2 -pa --grep lensid/i $filename"]
    stderr = [""]
    stdout = [""
        """Exif.NikonLd3.LensIDNumber                   Byte        1  Sigma 24-70mm F2,8 DG OS HSM Art
"""
]
    retval = [0]
