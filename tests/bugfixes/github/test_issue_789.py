import system_tests


class SegvInPngImageReadMetadata(
        metaclass=system_tests.CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/789
    """
    url = "https://github.com/Exiv2/exiv2/issues/789"

    filename = system_tests.path(
        "$data_path/issue_789_poc1.png"
    )
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = [""]
    retval = [1]

    compare_stderr = system_tests.check_no_ASAN_UBSAN_errors
