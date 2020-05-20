# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path
import unittest

@unittest.skip("Skipping test using option -pR (only for Debug mode)")

class test_issue_547(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/pull/547
    """
    url = "https://github.com/Exiv2/exiv2/issues/547"

    filename = path("$data_path/issue_547.poc")
    commands = ["$exiv2 -pR $filename"]
    stdout = ["""STRUCTURE OF TIFF FILE (II): $filename
 address |    tag                              |      type |    count |    offset | value
      12 | 0x0001 Version                      |    SSHORT |        0 |           | 
"""]
    stderr = [
"""Exiv2 exception in print action for file $filename:
$kerCorruptedMetadata
"""
]
    retval = [1]
