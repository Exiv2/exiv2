# -*- coding: utf-8 -*-

import system_tests


class TestFirstPoC(metaclass=system_tests.CaseMeta):
    """
    Regression test for the first bug described in:
    https://github.com/Exiv2/exiv2/issues/159
    """
    url = "https://github.com/Exiv2/exiv2/issues/159"

    filename = "$data_path/printStructure"
    commands = ["$exiv2 " + filename]
    stdout = [""]
    stderr = ["""$exiv2_exception_message """ + filename + """:
$kerCorruptedMetadata
"""]
    retval = [1]


# todo:
# class TestSecondPoC(system_tests.Case):
