# -*- coding: utf-8 -*-

import shutil
import system_tests
import os

@system_tests.CopyTmpFiles("$data_path/Stonehenge.heic")
class Exiv2FilePathsWithSpecialCharacters(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/1996"

    # Rename temporary file so that the path contains special Greek characters
    original_file = system_tests.path("$tmp_path/Stonehenge.heic")
    dst_file = system_tests.path("$tmp_path/Εκκρεμότητες.heic")

    def setUp(self):
        shutil.copyfile(self.original_file, self.dst_file)

    def tearDown(self):
        os.remove(self.original_file)
        os.remove(self.dst_file)

    commands = ["$exiv2 -K Xmp.cm2e.Father $dst_file"]
    stdout = ["Xmp.cm2e.Father                              XmpText    11  Robin Mills\n"]
    stderr = [""]
    retval = [0]
