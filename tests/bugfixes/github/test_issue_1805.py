# -*- coding: utf-8 -*-

import unittest
import system_tests
from system_tests import CaseMeta, check_no_ASAN_UBSAN_errors

# Check that `exiv2 -pr` works for different file types.
# ExifTool has a list of markers that appear in the headers:
# https://exiftool.org/makernote_types.html


@unittest.skip("Skipping test using option -pR (only for Debug mode)")
class exiv2pRHeaderTest(metaclass=CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/1805"

    filename = system_tests.path(
        "$data_path/exiv2-SonyDSC-HX60V.exv"
    )  # Uses marker: "SONY DSC "
    commands = [f"$exiv2 -pR {filename}"]

    stderr = [""]
    retval = [0]

    compare_stdout = check_no_ASAN_UBSAN_errors
