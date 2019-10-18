# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class WebPImageGetHeaderOffset(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/pull/960
    """
    url = "https://github.com/Exiv2/exiv2/pull/960"

    filename1 = path("$data_path/issue_960.poc.webp")
    commands = ["$exiv2 $filename1"]
    stdout = [""]
    stderr = [
"""Warning: Failed to decode Exif metadata.
Exiv2 exception in print action for file $filename1:
$kerTiffParsingError
"""
]
    retval = [1]
