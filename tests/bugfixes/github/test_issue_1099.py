# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors


class EmptyValueInCommandFile(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/1099

    An empty value in the command file causes a std::out_of_range exception.
    """
    url = "https://github.com/Exiv2/exiv2/issues/1099"

    filename1 = path("$data_path/issue_1099_poc.txt")
    filename2 = path("$data_path/issue_1099_poc.bin")
    commands = ["$exiv2 -m $filename1 mo $filename2"]
    stdout = ["""Usage: $exiv2exe [ options ] [ action ] file ...

Manipulate the Exif metadata of images.
"""]
    stderr = [
        """$filename1, line 1: Empty value for key `Exiff.LfkInfo.GPSDa'
$exiv2exe: Error parsing -m option arguments
"""]
    retval = [1]
