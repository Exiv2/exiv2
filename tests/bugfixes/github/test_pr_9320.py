# -*- coding: utf-8 -*-

import system_tests


class TestCanonException0x000c(metaclass=system_tests.CaseMeta):
    """Regression test for this PR: pr_9320.jpg"""
    url = "https://github.com/Exiv2/exiv2/pull/9320"

    filename = "$data_path/pr_9320.jpg"
    commands = [
        "$exiv2 -pa -K Exif.Canon.SerialNumber $filename",
    ]
    stdout = [
        "Exif.Canon.SerialNumber                      Ascii       6  bogus\n",
    ]
    stderr = [""]
    retval = [0]
