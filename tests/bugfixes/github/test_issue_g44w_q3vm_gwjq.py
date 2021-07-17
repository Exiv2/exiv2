# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path, check_no_ASAN_UBSAN_errors

class ImagePrintIFDStructureZeroCountAssert(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-g44w-q3vm-gwjq
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-g44w-q3vm-gwjq"

    filename = path("$data_path/issue_ghsa_g44w_q3vm_gwjq_poc.jpg")
    commands = ["$exiv2 -p R $filename"]
    stderr = ["""invalid type in tiff structure0
Exiv2 exception in print action for file $filename:
$kerInvalidTypeValue
"""]
    retval = [1]

    compare_stdout = check_no_ASAN_UBSAN_errors
