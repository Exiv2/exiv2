# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class test_issue_3332Test(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/3332

    The value of a nested XMP property has to be printed through the print
    function registered for that property, so the PLUS telephone types are
    translated instead of shown as the raw vocabulary URL.
    """

    url = "https://github.com/Exiv2/exiv2/issues/3332"

    filename = path("$data_path/issue_1959_poc.xmp")
    commands = ["$exiv2 -px -g LicensorTelephoneType $filename"]
    stdout = [
        """Xmp.plus.Licensor[1]/plus:LicensorTelephoneType1 XmpText    36  Work
Xmp.plus.Licensor[1]/plus:LicensorTelephoneType2 XmpText    36  Cell
"""
    ]
    stderr = [""]
    retval = [0]
