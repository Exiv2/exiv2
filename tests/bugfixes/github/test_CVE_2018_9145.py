# -*- coding: utf-8 -*-

import system_tests
import unittest

@unittest.skip("Skipping test using option -pR (only for Debug mode)")
class SubBoxLengthDataBufAbort(metaclass=system_tests.CaseMeta):

    url = "https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2018-9145"

    filename = system_tests.path(
        "$data_path/4-DataBuf-abort-1"
    )
    commands = ["$exiv2 -pR $filename"]
    stdout = [
        """STRUCTURE OF JPEG2000 FILE: $filename
 address |   length | box       | data
       0 |       12 | jP        | 
      12 |       20 | jp2h      | 
"""
    ]
    stderr = [
        """$exiv2_exception_message $filename:
$kerTiffParsingError
"""
    ]
    retval = [1]
