# -*- coding: utf-8 -*-

import system_tests


class test_issue_9324_packIfdId_overflow(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/9324"

    filename = "$data_path/issue_9324_poc.crw"
    command_filename = "$data_path/issue_9324_poc.txt"
    commands = [f"$exiv2 -m $command_filename $filename"]
    retval = [1]
    stderr = [
        """Exiv2 exception in modify action for file $filename:
$kerCorruptedMetadata
"""
    ]
    stdout = [""]
