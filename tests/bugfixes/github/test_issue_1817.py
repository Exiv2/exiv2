# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class MemoryLeakInPngImagePrintStructure(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1817

    Note: the test only fails in an ASAN build.
    """
    url = "https://github.com/Exiv2/exiv2/issues/1817"

    filename = path("$data_path/issue_1817_poc.png")
    commands = ["$exiv2 -pS $filename"]
    stdout = ["""STRUCTURE OF PNG FILE: $filename
 address | chunk |  length | data                           | checksum
       8 | eXIf  |       0 |                                | 0x00000000
"""]
    stderr = ["""$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""]
    retval = [1]
