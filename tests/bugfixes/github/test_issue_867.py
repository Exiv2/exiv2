# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class OutOfBoundsReadInIptcParserDecode(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/867
    """
    url = "https://github.com/Exiv2/exiv2/issues/867"

    filename = path("$data_path/issue_867_poc.psd")
    commands = ["$exiv2 $filename"]
    stdout = ["""File name       : $filename
File size       : 9830 Bytes
MIME type       : image/x-photoshop
Image size      : 150 x 91
"""
]
    stderr = [
        """Warning: Failed to decode IPTC metadata.
$filename: No Exif data found in the file
"""
]
    retval = [253]
