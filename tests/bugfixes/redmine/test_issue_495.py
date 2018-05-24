# -*- coding: utf-8 -*-

import system_tests


class CorruptedIopDirectory(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/495"

    filename = "$data_path/exiv2-bug495.jpg"

    commands = ["$exiv2 -u -pi $filename"]

    stdout = ["""Iptc.Application2.Caption                    String     56  Die Insel Stromboli mit dem Vulkan Stromboli, 1000m hoch
Iptc.Application2.Program                    String      7  digiKam
Iptc.Application2.ProgramVersion             String     11  0.9.0-beta3
"""]
    stderr = [""]
    retval = [0]

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors
