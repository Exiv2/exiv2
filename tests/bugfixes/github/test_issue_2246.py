# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class OlympusArtFilterPartialColor(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/2246
    """

    url = "https://github.com/Exiv2/exiv2/issues/2246"

    filename = path("$data_path/issue_2246_poc1.exv")
    commands = ["$exiv2 --Print kyvt --key Exif.OlympusCs.ArtFilter $filename"]
    stderr = [""]
    stdout = [
        """Exif.OlympusCs.ArtFilter                     Short      39 4352 0 5  Partial Color (position 6)
"""
    ]
    retval = [0]
