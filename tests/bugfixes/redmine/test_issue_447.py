# -*- coding: utf-8 -*-

import system_tests


class BufferOverflowInSscanf(metaclass=system_tests.CaseMeta):
    """
    This requires some sort of memchecker, either valgrind or ASAN.
    """

    url = "http://dev.exiv2.org/issues/447"

    filename = "$data_path/exiv2-bug447.jpg"
    commands = ["$exiv2 -u -pi $filename"]

    stdout = ["""Iptc.Application2.Caption                    String      0  
Iptc.Application2.DateCreated                Date        8  2005-08-09
Iptc.Application2.TimeCreated                Time       11  01:28:31-07:00
"""]
    stderr = [""]
    retval = [0]
