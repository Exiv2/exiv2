# -*- coding: utf-8 -*-

import system_tests

@system_tests.CopyFiles("$data_path/exiv2-bug1202.jpg")
class CheckFocusContinuous(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1202"
    filename = "$data_path/exiv2-bug1202_copy.jpg"

    commands = [
        """$exiv2 -M"set Exif.CanonCs.FocusContinuous SShort 0" $filename""",
        """$exiv2 -K Exif.CanonCs.FocusContinuous $filename""",

        """$exiv2 -M"set Exif.CanonCs.FocusContinuous SShort 1" $filename""",
        """$exiv2 -K Exif.CanonCs.FocusContinuous $filename""",

        """$exiv2 -M"set Exif.CanonCs.FocusContinuous SShort 8" $filename""",
        """$exiv2 -K Exif.CanonCs.FocusContinuous $filename""",

        """$exiv2 -M"set Exif.CanonCs.FocusContinuous SShort 9" $filename""",
        """$exiv2 -K Exif.CanonCs.FocusContinuous $filename""",

        """$exiv2 -M"set Exif.CanonCs.FocusContinuous SShort -1" $filename""",
        """$exiv2 -K Exif.CanonCs.FocusContinuous $filename""",
    ]

    stdout = [
        "",
        "Exif.CanonCs.FocusContinuous                 Short       1  Single\n",

        "",
        "Exif.CanonCs.FocusContinuous                 Short       1  Continuous\n",

        "",
        "Exif.CanonCs.FocusContinuous                 Short       1  Manual\n",

        "",
        "Exif.CanonCs.FocusContinuous                 Short       1  (9)\n",

        "",
        "Exif.CanonCs.FocusContinuous                 Short       1  (65535)\n",
    ]
    stderr = [""] * len(stdout)
    retval = [0] * len(stdout)
