# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = [
        "https://github.com/Exiv2/exiv2/issues/139",
        "https://bugzilla.redhat.com/show_bug.cgi?id=1494787"
    ]

    filename = "$data_path/009-stack-over"
    commands = ["$exiv2 " + filename]
    stdout = [""]
    stderr = ["""$exiv2_exception_message """ + filename + """:
$kerInvalidMalloc
"""]
    retval = [1]
