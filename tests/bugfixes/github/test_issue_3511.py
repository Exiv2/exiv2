# -*- coding: utf-8 -*-

import system_tests


class test_issue_3511_sigma_LoaderNative_getData(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/3511"

    filename = "$data_path/issue_3511_poc.eps"
    commands = ["$exiv2 -pp $filename"]
    retval = [1]
    stderr = [
        """$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""
    ]
    stdout = [""]
