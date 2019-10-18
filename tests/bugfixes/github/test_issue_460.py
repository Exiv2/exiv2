# -*- coding: utf-8 -*-

import system_tests


class ThrowsWhenCRWImageIsMalformed(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/460"

    filename = system_tests.path("$data_path/issue_460")
    commands = ["$exiv2 " + filename]
    retval = [1]
    stderr = ["""$exiv2_exception_message """ + filename + """:
$kerTiffParsingError
"""]
