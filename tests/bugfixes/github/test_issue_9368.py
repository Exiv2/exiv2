# -*- coding: utf-8 -*-

import system_tests

# This test was added to increase code coverage of XmpTextValue::read()
class XmpTextValue_read_coverage(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/9368"

    filename = "$data_path/issue_9368_coverage.exv"
    commands = ["$exiv2 $filename"]
    retval = [1]
    stderr = [
        """$exiv2_exception_message $filename:
Invalid XmpText type `Oops'
"""
    ]
    stdout = [""]
