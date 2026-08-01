# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors

class QuickTimeVideo_userDataDecoder_infinite_loop(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-fgw8-p7pr-37cp
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-fgw8-p7pr-37cp"

    filename = path("$data_path/issue_ghsa_fgw8_p7pr_37cp_poc.mov")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
        """$exiv2_exception_message """ + filename + """:
$kerCorruptedMetadata
"""]
    retval = [1]
