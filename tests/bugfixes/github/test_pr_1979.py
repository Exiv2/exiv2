# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class TestExiv2ExtractThumbnailToStdout(metaclass=CaseMeta):
    """
    Regression test for 'extracting a thumbnail to stdout' bug described in:
    https://github.com/Exiv2/exiv2/issues/1934
    """

    url = "https://github.com/Exiv2/exiv2/issues/1934"

    encodings = [bytes]

    def setUp(self):
        self.stdout = [bytes(open(self.expand_variables("$filename_ref"), "rb").read())]

    filename = path("$data_path/issue_1934_poc1.exv")
    filename_ref = path("$data_path/issue_1934_poc1-thumb.jpg")

    commands = ["$exiv2 --force --extract t- $filename"]

    stderr = [bytes([])]
    retval = [0]
