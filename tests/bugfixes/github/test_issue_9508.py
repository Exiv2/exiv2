# -*- coding: utf-8 -*-

import os
import shutil

from system_tests import CaseMeta, path

# Regression tests for https://github.com/Exiv2/exiv2/issues/9508:
# std::string::front() was called on an empty string, which aborts with
# _GLIBCXX_ASSERTIONS.


class EmptyGPSRefInExifToXmpConversion(metaclass=CaseMeta):
    """An empty GPSLatitudeRef is handled like a missing one."""

    url = "https://github.com/Exiv2/exiv2/issues/9508"

    original_file = path("$data_path/exiv2-empty.jpg")
    filename = path("$tmp_path/issue_9508_gps.jpg")
    xmp_file = path("$tmp_path/issue_9508_gps.xmp")

    commands = ["$exiv2 -m- $filename", "$exiv2 -eX $filename"]
    stdin = [
        """set Exif.GPSInfo.GPSLatitude 10/1 20/1 30/1
set Exif.GPSInfo.GPSLatitudeRef Ascii ""
""",
        None,
    ]
    stdout = ["", ""]
    stderr = ["", "Warning: Failed to convert Exif.GPSInfo.GPSLatitude to Xmp.exif.GPSLatitude\n"]
    retval = [0, 0]

    def setUp(self):
        shutil.copyfile(self.original_file, self.filename)
        if os.path.exists(self.xmp_file):
            os.remove(self.xmp_file)

    def tearDown(self):
        os.remove(self.filename)
        os.remove(self.xmp_file)


class OverwritePromptAtEndOfInput(metaclass=CaseMeta):
    """No answer to the overwrite prompt means "no"."""

    url = "https://github.com/Exiv2/exiv2/issues/9508"

    original_file = path("$data_path/exiv2-empty.jpg")
    filename = path("$tmp_path/issue_9508_overwrite.jpg")
    xmp_file = path("$tmp_path/issue_9508_overwrite.xmp")

    commands = ["$exiv2 -eX $filename", "$exiv2 -eX $filename"]
    stdin = [None, ""]
    stdout = ["", "exiv2: Overwrite `$xmp_file'? "]
    stderr = ["", ""]
    retval = [0, 0]

    def setUp(self):
        shutil.copyfile(self.original_file, self.filename)
        if os.path.exists(self.xmp_file):
            os.remove(self.xmp_file)

    def tearDown(self):
        os.remove(self.filename)
        os.remove(self.xmp_file)


class RenamePromptAtEndOfInput(metaclass=CaseMeta):
    """No answer to the rename prompt means "skip"."""

    url = "https://github.com/Exiv2/exiv2/issues/9508"

    original_file = path("$data_path/exiv2-empty.jpg")
    filename = path("$tmp_path/issue_9508_rename.jpg")
    existing_file = path("$tmp_path/20210102_030405.jpg")

    commands = ["$exiv2 -m- $filename", "$exiv2 mv $filename"]
    stdin = ["set Exif.Photo.DateTimeOriginal 2021:01:02 03:04:05\n", ""]
    stdout = ["", "exiv2: File `$existing_file' exists. [O]verwrite, [r]ename or [s]kip? "]
    stderr = ["", ""]
    retval = [0, 0]

    def setUp(self):
        shutil.copyfile(self.original_file, self.filename)
        shutil.copyfile(self.original_file, self.existing_file)

    def tearDown(self):
        # The file was skipped, not renamed.
        self.assertTrue(os.path.exists(self.filename))
        os.remove(self.filename)
        os.remove(self.existing_file)
