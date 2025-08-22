# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/73"

    filename = "$data_path/02-Invalid-mem-def"
    commands = [f"$exiv2 -q {filename}"]
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
