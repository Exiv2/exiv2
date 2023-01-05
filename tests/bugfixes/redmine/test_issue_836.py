# -*- coding: utf-8 -*-

import unittest
import os.path
import shutil

import system_tests


RESOURCE_FORK_EXISTS = os.path.exists(
    os.path.join(system_tests.data_path, "exiv2-bug836.eps/..namedfork/rsrc")
)


@unittest.skipUnless(RESOURCE_FORK_EXISTS,
                     "File system does not support resource forks")
@system_tests.CopyFiles("$data_path/exiv2-bug836.eps")
class WriteMetadataDestroysResourceForkOnMacOSXForBigFiles(
        metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/836"

    filename = "$data_path/exiv2-bug836_copy.eps"
    filename_orig = "$data_path/exiv2-bug836.eps"

    commands = [
        "$exiv2 -M'set Exif.Photo.UserComment Test' $filename",
    ]

    def setUp(self):
        """ Copy the rsrc file into the the resource fork """
        shutil.copyfile(
            self.filename_orig + ".rsrc", self.filename + "/..namedfork/rsrc"
        )

    def post_tests_hook(self):
        """ Check that the resource fork didn't change """
        with open(self.filename_orig + ".rsrc", "rb") as rsrc_orig:
            expected_resource_fork = rsrc_orig.read(-1)

        with open(self.filename + "/..namedfork/rsrc", "rb") as rsrc_f:
            got_resource_fork = rsrc_f.read(-1)

        self.assertEqual(expected_resource_fork, got_resource_fork)

    stdout = [""]
    stderr = [""]
    retval = [0]
