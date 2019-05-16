# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class OutOfMemoryInPsdImageReadMetadata(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/855

    There is no bounds check on the value of resourceSize,
    leading to an out-of-memory error.
    """
    url = "https://github.com/Exiv2/exiv2/issues/855"

    filename = path("$data_path/issue_855_poc.psd")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = ["""Warning: Failed to decode IPTC metadata.
Exiv2 exception in print action for file $filename:
corrupted image metadata
"""
]
    retval = [1]
