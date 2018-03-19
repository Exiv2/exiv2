# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(system_tests.Case):

    url = "https://github.com/Exiv2/exiv2/issues/188"
    found_by = ["Wei You", "@youwei1988"]

    filename = "{data_path}/poc_2017-12-12_issue188"
    commands = ["{exiv2} " + filename]
    stdout = [""]
    stderr = ["""std::overflow_error exception in print action for file """ + filename + """:
Overflow in addition
"""]
    retval = [1]
