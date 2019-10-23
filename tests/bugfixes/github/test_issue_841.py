# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors


class InvalidDataLocationInCiffComponentDoRead(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/841

    An invalid data location causes an assertion failure.
    """
    url = "https://github.com/Exiv2/exiv2/issues/841"

    filename = path("$data_path/issue_841_poc.crw")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
        """$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""]
    retval = [1]
