# -*- coding: utf-8 -*-

import os
import shutil

import system_tests


class PercentABrokenInRename(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/683"

    original_file = "$data_path/exiv2-nikon-d70.jpg"
    filename = "$data_path/exiv2-nikon-d70_copy.jpg"
    new_file = "$data_path/2004-03-30-Tue-090.jpg"

    commands = ["$exiv2 -u -f -r %Y-%m-%d-%a-%j $filename"]
    stdout = [""]
    stderr = [""]
    retval = [0]

    def setUp(self):
        shutil.copyfile(self.original_file, self.filename)

    def tearDown(self):
        self.assertTrue(os.path.exists(self.new_file))
        os.remove(self.new_file)
