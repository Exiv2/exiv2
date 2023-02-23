# -*- coding: utf-8 -*-

import system_tests
import ctypes

class TestCvePoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/73"

    filename = "$data_path/02-Invalid-mem-def"
    commands = ["$exiv2 -q " + filename]
    stdout = [""]
    stderr = ["""$exiv2_exception_message """ + filename + ":\n" +
              ("$kerFailedToReadImageData" if ctypes.sizeof(ctypes.c_voidp) > 4 else "$kerCorruptedMetadata") +
              "\n"]
    retval = [1]
