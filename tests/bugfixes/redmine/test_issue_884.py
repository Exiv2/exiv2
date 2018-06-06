# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class NewTamronAndPentaxLenses(metaclass=CaseMeta):

    url = "http://dev.exiv2.org/issues/884"

    filenames = [
        path("$data_path/exiv2-bug884{:s}.jpg".format(char))
        for char in ["a", "b", "c"]
    ]

    commands = [
        "$exiv2 -pt --grep LensType " + fname for fname in filenames
    ]

    stdout = [
        """Exif.Pentax.LensType                         Byte        2  Tamron SP AF 17-50mm F2.8 XR Di II
""",
        """Exif.Pentax.LensType                         Byte        2  smc PENTAX-F 35-70mm F3.5-4.5
""",
        """Exif.Pentax.LensType                         Byte        2  PENTAX-F 28-80mm F3.5-4.5
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)
