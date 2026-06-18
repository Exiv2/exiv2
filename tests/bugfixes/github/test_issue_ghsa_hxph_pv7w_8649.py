# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors

class CrwMap_decodeBasic_OOB_read(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-hxph-pv7w-8649
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-hxph-pv7w-8649"

    filename = path("$data_path/issue_ghsa_hxph_pv7w_8649_poc.crw")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
        """$exiv2_exception_message """ + filename + """:
$kerCorruptedMetadata
"""]
    retval = [1]
