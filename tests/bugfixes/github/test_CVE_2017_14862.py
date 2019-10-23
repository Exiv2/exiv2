# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/75"

    filename = "$data_path/008-invalid-mem"
    commands = ["$exiv2 " + filename]
    stdout = [""]
    stderr = ["""$exiv2_exception_message """ + filename + """:
$kerCorruptedMetadata
"""]
    retval = [1]

    def compare_stderr(self, i, command, got_stderr, expected_stderr):
        """
        Only check that an exception is thrown for this file
        ignore all the warnings on stderr on purpose.
        """
        self.assertIn(expected_stderr, got_stderr)
