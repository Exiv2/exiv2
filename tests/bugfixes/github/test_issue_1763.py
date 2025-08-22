# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class CanonPrintCsLensTypeByMetadataNullIteratorDeref(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1763
    """

    url = "https://github.com/Exiv2/exiv2/issues/1763"

    filename = path("$data_path/issue_1763_poc.exv")
    commands = ["$exiv2 -Pt $filename"]

    stderr = [""]
    retval = [0]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        # Check that it printed "Bad value" for the date.
        self.assertIn("Unknown Lens (254)", got_stdout)
