# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class Jp2ImageDoWriteMetadataOutOfBoundsRead(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-7569-phvm-vwc2
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-7569-phvm-vwc2"

    filename1 = path("$data_path/issue_ghsa_7569_phvm_vwc2_poc.jp2")
    filename2 = path("$data_path/issue_ghsa_7569_phvm_vwc2_poc.exv")
    commands = ["$exiv2 -q in $filename1"]
    stdout = [""]
    stderr = [
"""Exiv2 exception in insert action for file $filename1:
$kerCorruptedMetadata
"""]
    retval = [1]
