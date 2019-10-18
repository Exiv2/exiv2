# -*- coding: utf-8 -*-

import system_tests


class DetectsWrongLengthOfImageResourceInPSDFile(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/426"

    filename = system_tests.path("$data_path/h02.psd")
    commands = ["$exiv2 " + filename]
    retval = [1]
    stderr = ["""$exiv2_exception_message """ + filename + """:
$kerTiffParsingError
"""]
