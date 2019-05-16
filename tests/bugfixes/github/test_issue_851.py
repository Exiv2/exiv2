# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class DenialOfServiceInAdjustTimeOverflow(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/851

    The date parsing code in XMPUtils::ConvertToDate does not
    check that the month and day are in bounds. This can cause a
    denial of service in AdjustTimeOverflow because it adjusts
    out-of-bounds days in a loop that subtracts one month per
    iteration.
    """
    url = "https://github.com/Exiv2/exiv2/issues/851"

    filename = path("$data_path/issue_851_poc.xmp")
    commands = ["$exiv2 $filename"]
    stdout = ["""File name       : $filename
File size       : 317 Bytes
MIME type       : application/rdf+xml
Image size      : 0 x 0
"""
]
    stderr = [
        """Warning: Failed to convert Xmp.xmp.CreateDate to Exif.Photo.DateTimeDigitized (Day is out of range)
$filename: No Exif data found in the file
"""]
    retval = [253]
