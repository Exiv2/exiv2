# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors

class XMPUtilsSetTimeZoneIntegerOverflow(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1901
    """
    url = "https://github.com/Exiv2/exiv2/issues/1901"

    filename1 = path("$data_path/issue_1901_poc1.xmp")
    filename2 = path("$data_path/issue_1901_poc2.xmp")
    filename3 = path("$data_path/issue_1901_poc3.xmp")
    filename4 = path("$data_path/issue_1901_poc4.xmp")
    commands = ["$exiv2 -q $filename1",
                "$exiv2 -q $filename2",
                "$exiv2 -q $filename3",
                "$exiv2 -q $filename4"]
    stderr = ["""$filename1: No Exif data found in the file
""",
              """$filename2: No Exif data found in the file
""",
              """$filename3: No Exif data found in the file
""",
              ""]
    retval = [253, 253, 253, 0]

    compare_stdout = check_no_ASAN_UBSAN_errors
