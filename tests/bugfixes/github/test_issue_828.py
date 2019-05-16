# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path
import unittest

@unittest.skip("Skipping test using option -pR (only for Debug mode)")
class SegmentationFaultPngImage(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/828
    """
    url = "https://github.com/Exiv2/exiv2/issues/828"

    filename = path("$data_path/issue_828_poc.png")
    commands = ["$exiv2 -pR $filename"]
    stdout = ["""STRUCTURE OF PNG FILE: $filename
 address | chunk |  length | data                           | checksum
       8 | IHDR  |      13 | .............                  | 0x085f152d
      33 | gAMA  |       4 | ....                           | 0x0bfc6105
      49 | PLTE  |     717 | .........].....Y..q..}..q..y.. | 0x7b35a4a5
     778 | tRNS  |     238 | -U...............}...=..e..... | 0xebd29012
    1028 | bKGD  |       1 | .                              | 0xcfb7d237
    1041 | tEXt  |      14 | Title.IceAlpha                 | 0xc562f8cb
    1067 | tEXt  |      31 | Author.Pieter S. van der Meule | 0x6bfe26a8
    1110 | tEXt  |     411 | Description.The original image | 0xe12effe4
"""
]
    stderr = [
        """$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""]
    retval = [1]
