# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors

class XmpIptcStandardsTest(metaclass=CaseMeta):
    """
    Regression test for the issue described in:
    https://github.com/Exiv2/exiv2/issues/1959
    """
    url = "https://github.com/Exiv2/exiv2/issues/1959"

    def setUp(self):
        self.stdout = [open(self.expand_variables("$filename_ref"),'r').read()]
    
    filename      = path("$data_path/issue_1959_poc.xmp")
    filename_ref  = path("$data_path/issue_1959_poc.xmp.out")
    
    commands = ["$exiv2 -Pkvt $filename"]

    stderr = [""]
    retval = [0]
