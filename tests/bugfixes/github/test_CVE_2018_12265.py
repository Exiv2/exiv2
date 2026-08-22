# -*- coding: utf-8 -*-

import system_tests


class AdditionOverflowInLoaderExifJpeg(metaclass=system_tests.CaseMeta):
    """
    Regression test for bug #365:
    https://github.com/Exiv2/exiv2/issues/365
    aka CVE 2018-12265:
    https://cve.mitre.org/cgi-bin/cvename.cgi?name=2018-12265
    """

    filename = system_tests.path("$data_path/1-out-of-read-Poc")
    commands = ["$exiv2 -q -ep $filename"]
    retval = [1]
    stderr = [
        """$exception_in_extract $filename:
$kerCorruptedMetadata
"""
    ]
    stdout = [""]
