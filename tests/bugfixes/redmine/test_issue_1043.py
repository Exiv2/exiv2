# -*- coding: utf-8 -*-

import os
import shutil
import string

import system_tests


class FailureOnCifsShares(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1043"
    num = 1043

    original_file = system_tests.path("$data_path/exiv2-bug884c.jpg")

    files = [
        system_tests.path("$data_path/bug$num-" + char + ".jpg")
        for char in string.ascii_uppercase
    ]

    def setUp(self):
        for fname in self.files:
            shutil.copyfile(self.original_file, fname)

    def tearDown(self):
        for fname in self.files:
            os.remove(fname)

    commands = [
        """$exiv2 -u -v -M"set Exif.Photo.UserComment Test Bug $num my filename is {fname_short}" {fname}"""\
        .format(fname=fname, fname_short=os.path.split(fname)[1])
        for fname in files
    ] + [
        # workaround for * wildcard in bash:
        """$exiv2 -PE -g UserComment {!s}""".format(" ".join(files))
    ]

    retval = [0] * (len(files) + 1)
    stdout = [
        """File 1/1: {fname}
Set Exif.Photo.UserComment "Test Bug $num my filename is {fname_short}" (Comment)
"""
        .format(fname=fname, fname_short=os.path.split(fname)[1])
        for fname in files
    ] + [
        """""".join(
            """{fname}  Exif.Photo.UserComment                       Undefined  50  Test Bug $num my filename is {fname_short}
"""
            .format(fname=fname, fname_short=os.path.split(fname)[1])
            for fname in files
        )
    ]

    stderr= [""] * (len(files) + 1)
