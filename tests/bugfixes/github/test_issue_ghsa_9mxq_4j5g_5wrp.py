# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path


class CrwMap_decode0x0805_OutOfBoundsRead(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-9mxq-4j5g-5wrp
    """

    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-9mxq-4j5g-5wrp"

    filename = path("$data_path/issue_ghsa_9mxq_4j5g_5wrp.crw")
    commands = ["$exiv2 $filename"]
    stdout = ["""File name       : $filename
File size       : 74 Bytes
MIME type       : image/x-canon-crw
Image size      : 0 x 0
"""
]
    stderr   = ["""$filename: No Exif data found in the file
"""]
    retval = [253]
