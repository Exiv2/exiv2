# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/188"
    found_by = ["Wei You", "@youwei1988"]

    filename = "$data_path/poc_2017-12-12_issue188"
    commands = ["$exiv2 " + filename]
    stdout = [""]
    stderr = ["""$exiv2_exception_message """ + filename + """:
$kerCorruptedMetadata
"""]
    retval = [1]
