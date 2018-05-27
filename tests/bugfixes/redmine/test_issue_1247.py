# -*- coding: utf-8 -*-

import system_tests


class NotValidIccProfile(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1247"
    filename = "$data_path/exiv2-bug1247.jpg"

    commands = [
        """$exiv2 -pa $filename"""
    ]

    stdout = [""]

    stderr = [ """Exiv2 exception in print action for file $filename:
Not a valid ICC Profile
"""
    ]

    retval = [1]
