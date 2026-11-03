# -*- coding: utf-8 -*-

from system_tests import CaseMeta


class VerboseVersionIsReproducible(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/3279

    The output of `exiv2 --verbose --version` must not depend on the
    build date/time (__DATE__ and __TIME__ make builds non-reproducible).
    """

    url = "https://github.com/Exiv2/exiv2/issues/3279"

    commands = ["$exiv2 --verbose --version"]

    stderr = [""]
    retval = [0]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        keys = [line.split("=", 1)[0] for line in got_stdout.splitlines()]
        self.assertIn("version", keys)
        self.assertNotIn("date", keys)
        self.assertNotIn("time", keys)
