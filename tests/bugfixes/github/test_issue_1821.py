# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path
import unittest

# Issue #1821 is a memory leak caused by buggy third-party
# code in the xmpsdk sub-directory, so it isn't easy for us to fix.
# This test will fail when run with an ASAN build.
@unittest.skip("Skipping test until #1821 is fixed")

class MemoryLeakAddBinding(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1821
    """
    url = "https://github.com/Exiv2/exiv2/issues/1821"

    filename = path("$data_path/issue_1821_poc.xmp")
    commands = ["$exiv2 -q $filename"]
    stdout = ["""File name       : $filename
File size       : 163 Bytes
MIME type       : application/rdf+xml
Image size      : 0 x 0
"""]
    stderr = ["""$filename: No Exif data found in the file
"""]
    retval = [253]
