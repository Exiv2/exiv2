# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors

class Jp2ImagePrintStructureICC(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-583f-w9pm-99r2
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-583f-w9pm-99r2"

    filename = path("$data_path/issue_ghsa_583f_w9pm_99r2_poc.jp2")
    commands = ["$exiv2 -p C $filename"]
    stdout = [""]
    stderr = ["""Exiv2 exception in print action for file $filename:
$kerCorruptedMetadata
"""]
    retval = [1]
