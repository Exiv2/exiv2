# -*- coding: utf-8 -*-

import os
import shutil

import system_tests

URL = "https://github.com/Exiv2/exiv2/issues/950"

@system_tests.CopyFiles("$data_path/issue_950")
class RegressionTestForInfiteLoopInJp2Image(metaclass=system_tests.CaseMeta):
    """
    Regression test for the issue reported in #950.
    """

    def setUp(self):
        shutil.copy(self.filename, self.filenameExv)

    def tearDown(self):
        os.remove(self.filenameExv)

    filename = system_tests.path("$data_path/issue_950_copy")
    filenameExv = system_tests.path("$data_path/issue_950_copy.exv")

    commands = ["$exiv2 insert $filenameExv"]
    stdout = [""]
    stderr = ["""$filenameExv: Could not write metadata to file: $kerCorruptedMetadata
"""]
    retval = [1]
