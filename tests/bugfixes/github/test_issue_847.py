# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class LargeAllocationInPgfReadMetadata(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/847

    An unchecked allocation size causes a std::bad_alloc to
    be thrown.
    """
    url = "https://github.com/Exiv2/exiv2/issues/847"

    filename = path("$data_path/issue_847_poc.pgf")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
        """$exiv2_exception_message $filename:
Failed to read input data
"""]
    retval = [1]
