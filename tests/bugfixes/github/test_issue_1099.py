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

    stderr = [
        """$filename1, line 1: Empty value for key `Exiff.LfkInfo.GPSDa'
exiv2: Error parsing -m option arguments
"""]
    retval = [1]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """ We don't care about the stdout, just don't crash """
        pass
