# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


<<<<<<< HEAD
class CrwEncode0x1810IntegerOverflow(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1530
=======
class MrmImageLargeAllocation(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/pull/943
>>>>>>> c92ac88cb... Regression test for https://github.com/Exiv2/exiv2/issues/1530
    """
    url = "https://github.com/Exiv2/exiv2/issues/1530"

    filename1 = path("$data_path/issue_1530_poc.crw")
    filename2 = path("$data_path/issue_1530_poc.exv")
    commands = ["$exiv2 in $filename1 $filename2"]
    stdout = [""]
    stderr = [
<<<<<<< HEAD
"""$filename1: Could not write metadata to file: $kerCorruptedMetadata
=======
"""$filename1: Could not write metadata to file: corrupted image metadata
>>>>>>> c92ac88cb... Regression test for https://github.com/Exiv2/exiv2/issues/1530
"""]
    retval = [1]
