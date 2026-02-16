# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class RafNoTiffRegression(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1791
    """
    url = "https://github.com/Exiv2/exiv2/issues/1791"

    filename1 = path("$data_path/issue_1791_old.raf")
    filename2 = path("$data_path/issue_1791_new.raf")
    commands = ["$exiv2 -pa $filename1", "$exiv2 -pa $filename2"]
    stdout = ["""Exif.Image2.JPEGInterchangeFormat            Long        1  104
Exif.Image2.JPEGInterchangeFormatLength      Long        1  12
""",
              """Exif.Image2.JPEGInterchangeFormat            Long        1  104
Exif.Image2.JPEGInterchangeFormatLength      Long        1  12
Exif.Image.NewSubfileType                    Long        1  Primary image
"""]
    stderr = ["", ""]
    retval = [0, 0]
