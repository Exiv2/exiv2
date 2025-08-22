# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/73"

    filename = "$data_path/003-heap-buffer-over"
    commands = [f"$exiv2 {filename}"]
    stdout = [""]
    stderr = [
        f"""$exiv2_exception_message {filename}:
$kerCorruptedMetadata
"""
    ]
    retval = [1]
