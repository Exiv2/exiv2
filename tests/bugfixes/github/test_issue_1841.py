# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class MemoryLeakWebPImageDecodeChunks(metaclass=CaseMeta):
    """
    Test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1841
    """
    url = "https://github.com/Exiv2/exiv2/issues/1841"

    filename = path("$data_path/issue_1841_poc.webp")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = ["""$exiv2_exception_message $filename:
This does not look like a TIFF image
"""]
    retval = [1]
