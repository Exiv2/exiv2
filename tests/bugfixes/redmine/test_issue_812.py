# -*- coding: utf-8 -*-

import os
import shutil

import system_tests


class DoNotDestroyHardLinks(metaclass=system_tests.CaseMeta):

    def setUp(self):
        shutil.copy(self.orig_file, self.filename)
        os.link(self.filename, self.link_1)
        os.link(self.filename, self.link_2)

    def tearDown(self):
        for f in [self.filename, self.link_1, self.link_2]:
            os.remove(f)

    url = "http://dev.exiv2.org/issues/812"
    num = 812

    # original file, copy & hardlinks
    orig_file = "$data_path/exiv2-bug884c.jpg"
    filename = "$data_path/exiv2-bug$num.jpg"
    link_1 = "$data_path/exiv2-bug$num-B.jpg"
    link_2 = "$data_path/exiv2-bug$num-C.jpg"

    # list of files passed to exiv2 since we can't use * in the command
    file_list = "{!s} {!s} {!s}".format(filename, link_1,  link_2)

    commands = [
        """$exiv2 -u -v -M"set Exif.Photo.UserComment Test Bug $num" $filename""",
        "$exiv2 -PE -g UserComment $file_list",
        """$exiv2 -u -v -M"set Exif.Photo.UserComment Test Bug $num modified" $filename""",
        "$exiv2 -PE -g UserComment $file_list"
    ]

    first_change = "Exif.Photo.UserComment                       Undefined  20  Test Bug 812"
    second_change = "Exif.Photo.UserComment                       Undefined  29  Test Bug 812 modified"

    stdout = [
        """File 1/1: $filename
Set Exif.Photo.UserComment "Test Bug 812" (Comment)
""",
        """$filename  $first_change
$link_1  $first_change
$link_2  $first_change
""",
        """File 1/1: $filename
Set Exif.Photo.UserComment "Test Bug 812 modified" (Comment)
""",
        """$filename  $second_change
$link_1  $second_change
$link_2  $second_change
"""
    ]
    stderr = [""] * 4
    retval = [0] * 4
