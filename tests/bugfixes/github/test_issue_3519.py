# -*- coding: utf-8 -*-

import system_tests

class test_issue_3519_cast_to_int64(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/3519"

    filename = "$data_path/issue_3519_poc.xmp"
    commands = ["$exiv2 $filename"]
    retval = [1]
    stderr = [
        """$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""
    ]
    stdout = [""]
