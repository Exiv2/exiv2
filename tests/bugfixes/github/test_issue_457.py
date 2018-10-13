# -*- coding: utf-8 -*-

import system_tests


class ThrowsWhenColorProfileInTiffMetadataIsCorrupted(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/457"

    filename = "$data_path/issue_457_poc"

    commands = ["$exiv2 " + filename]
    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors
    retval = [1]
