# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors

class ImagePrintIFDStructure(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-m479-7frc-gqqg
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-m479-7frc-gqqg"

    filename = path("$data_path/issue_ghsa_m479_7frc_gqqg_poc.crw")
    commands = ["$exiv2 -p C $filename"]
    stdout = [""]
    stderr = ["""Exiv2 exception in print action for file $filename:
$kerCorruptedMetadata
"""]
    retval = [1]
