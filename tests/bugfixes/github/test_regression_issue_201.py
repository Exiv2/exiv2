# -*- coding: utf-8 -*-

import system_tests


class ShadowingError(system_tests.Case):

    commands = ["{exiv2} -PE {data_path}/IMGP0020.exv"]
    stdout = [""]
    stderr = [""]
    retval = [0]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """
        We only really care about the LensInfo line and that exiv2 does not
        crash, which the return value check also ensures.
        """
        self.assertIn(
            "Exif.PentaxDng.LensInfo                      Undefined  69  131 0 0 255 0 40 148 111 65 69 6 238 65 78 153 80 40 1 73 107 251 255 255 255 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
            got_stdout
        )
