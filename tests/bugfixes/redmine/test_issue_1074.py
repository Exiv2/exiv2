# -*- coding: utf-8 -*-

import hashlib

import system_tests


class IccProfileInApp2Segment(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1074"
    num = 1074

    encodings = [bytes]

    filenames = [
        system_tests.path("$data_path/" + fname)
        for fname in (
            "exiv2-bug$num.png", "imagemagick.png", "Reagan.tiff", "Reagan.jpg"
        )
    ]

    commands = [
        "$exiv2 -pC " + fname for fname in filenames
    ]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        self.assertEqual(
            hashlib.md5(got_stdout).hexdigest(), expected_stdout
        )

    stderr = [bytes()] * len(filenames)
    stdout = [
        "5c02432934195866147d8cbfa49f3fcf",
        "cf0aeee7fdc11b20ad8a19d65628488e",
        "1d3fda2edb4a89ab60a23c5f7c7d81dd",
        "50b9125494306a6fc1b7c4f2a1a8d49d"
    ]
    retval = [0] * len(filenames)
