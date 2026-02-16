# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/74"

    filename = "$data_path/005-invalid-mem"
    commands = [f"$exiv2 {filename}"]
    stdout = [""]
    stderr = [
        f"""$exiv2_exception_message {filename}:
{(
    "$kerFailedToReadImageData"
    if system_tests.BT.Config.is_64bit
    else "$kerCorruptedMetadata"
)}
"""
    ]
    retval = [1]

    def compare_stderr(self, i, command, got_stderr, expected_stderr):
        """Only check that an exception is thrown"""
        self.assertIn(expected_stderr, got_stderr)
