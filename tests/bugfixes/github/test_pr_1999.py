# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class TestXmpDateTimeSetting(metaclass=CaseMeta):
    """
    Test fix for issue 1998.
    """

    infile = path("$data_path/issue_1998.xmp")
    commands = [
        '$exiv2 -M"set Xmp.xmp.CreateDate XmpText 2021-02-03T12:00:00+01:00" $infile',
        "$exiv2 -K Xmp.xmp.CreateDate $infile",
        '$exiv2 -M"set Xmp.xmp.CreateDate XmpText 2021-02-03T12:34:56+02:00" $infile',
        "$exiv2 -K Xmp.xmp.CreateDate $infile",
    ]
    stdout = [
        "",
        """Xmp.xmp.CreateDate                           XmpText    25  2021-02-03T12:00:00+01:00
""",
        "",
        """Xmp.xmp.CreateDate                           XmpText    25  2021-02-03T12:34:56+02:00
""",
    ]
    stderr = [""] * 4
    retval = [0] * 4
