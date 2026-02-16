# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path, check_no_ASAN_UBSAN_errors
@CopyTmpFiles("$data_path/issue_ghsa_496f_x7cq_cq39_poc.jpg")

class EpsImageDeleteSegV(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-496f-x7cq-cq39
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-496f-x7cq-cq39"

    filename = path("$tmp_path/issue_ghsa_496f_x7cq_cq39_poc.jpg")
    commands = ["$exiv2 rm $filename"]
    stdout = [""]
    stderr = [
        """$exception_in_erase """ + filename + """:
$kerFailedToReadImageData
"""]
    retval = [1]
