# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class XMPUtilsLeapYearOverflow(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1954
    """

    url = "https://github.com/Exiv2/exiv2/issues/1954"

    filename = path("$data_path/issue_1954_poc.xmp")
    commands = ["$exiv2 -q $filename"]
    stderr = [
        """$filename: No Exif data found in the file
"""
    ]
    stdout = [
        """File name       : $filename
File size       : 172 Bytes
MIME type       : application/rdf+xml
Image size      : 0 x 0
"""
    ]
    retval = [253]
