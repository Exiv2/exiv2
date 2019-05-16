# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class DenialOfServiceInAdjustTimeOverflow(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/853

    The date parsing code in XMPUtils::ConvertToDate does not
    check that the month and day are in bounds. This can cause a
    denial of service in AdjustTimeOverflow because it adjusts
    out-of-bounds days in a loop that subtracts one month per
    iteration.
    """
    url = "https://github.com/Exiv2/exiv2/issues/853"

    filename = path("$data_path/issue_853_poc.jpg")
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [
        """Exiv2 exception in print action for file $filename:
Not a valid ICC Profile
"""]
    retval = [1]
