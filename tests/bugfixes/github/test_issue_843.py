# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class IntegerOverflowInCiffDirectoryReadDirectory(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/843

    An integer overflow causes an out-of-bounds read.
    """
    url = "https://github.com/Exiv2/exiv2/issues/843"

    filename = path("$data_path/issue_843_poc.crw")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
        """$exiv2_exception_message $filename:
$kerCorruptedMetadata
"""]
    retval = [1]
