# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class TestExifTagsInTaglist(metaclass=CaseMeta):

    url = "https://github.com/Exiv2/exiv2/pull/1905/files"

    def setUp(self):
        self.stdout = [open(self.expand_variables("$filename_ref"),'r').read()]

    filename_ref = path("$ref_path/test_pr_1905_poc1_ref.out")
    
    commands = ["$taglist Exif"]

    stderr = [""]
    retval = [0]
    
