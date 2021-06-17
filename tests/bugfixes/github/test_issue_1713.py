# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class InvalidDateXMP(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1713
    """
    url = "https://github.com/Exiv2/exiv2/issues/1713"

    filename = path("$data_path/issue_1713_poc.xmp")
    commands = ["$exiv2 -Ph $filename"]

    stderr = [
"""Warning: Failed to convert Xmp.xmp.CreateDate to Exif.Photo.DateTimeDigitized (Day is out of range)
Exiv2 exception in print action for file $filename:
Xmpdatum::copy: Not supported
"""
]
    retval = [1]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """ We don't care about the stdout, just don't crash """
        pass
