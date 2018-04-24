# -*- coding: utf-8 -*-

import system_tests


class DetectionOfSigma55_200mmLens(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/816"

    filenames = [
        "$data_path/exiv2-bug816{!s}.exv".format(char)
        for char in ["a", "b", "c", "d", "e"]
    ]

    commands = [
        "$exiv2 -pa --grep Lens " + filename for filename in filenames
    ]
    stdout = [
        """Exif.Pentax.LensType                         Byte        2  Sigma 55-200mm F4-5.6 DC
Exif.Pentax.LensInfo                         Undefined  36  3 255 0 0 40 148 71 152 80 6 241 65 237 153 88 36 1 76 107 251 255 255 255 0 0 80 6 241 0 0 0 0 0 0 0 0
""",
        """Exif.PentaxDng.LensType                      Byte        3  Sigma 55-200mm F4-5.6 DC
Exif.PentaxDng.LensInfo                      Undefined  69  131 0 0 255 0 40 148 68 244 112 6 243 65 197 153 88 35 1 73 107 251 255 255 255 0 0 112 6 243 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
        """Exif.Pentax.LensType                         Byte        3  Sigma 55-200mm F4-5.6 DC
Exif.Pentax.LensInfo                         Undefined  69  131 0 0 255 0 40 148 68 244 112 6 243 65 197 153 88 35 1 73 107 251 255 255 255 0 0 112 6 243 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
        """Exif.Pentax.LensType                         Byte        3  Sigma 55-200mm F4-5.6 DC
Exif.Pentax.LensInfo                         Undefined  69  131 0 0 255 0 40 148 68 244 112 6 243 65 197 153 88 35 1 73 107 251 255 255 255 0 0 112 6 243 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
        """Exif.PentaxDng.LensType                      Byte        4  Sigma 55-200mm F4-5.6 DC
Exif.PentaxDng.LensInfo                      Undefined 128  0 131 128 0 0 255 1 184 0 0 0 0 0 0 0 0 40 148 71 78 128 70 244 65 89 136 88 61 1 64 107 251 255 255 255 0 0 128 70 244 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
"""
    ]
    stderr = [""] * 5
    retval = [0] * 5
