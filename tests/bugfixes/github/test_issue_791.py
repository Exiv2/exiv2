import system_tests


class IntegerOverflowInWebpImageReadMetadata(
        metaclass=system_tests.CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/791

    Due to an integer overflow bug (#791), this test triggers a 4GB
    memory allocation. So the test will fail with a std::bad_alloc
    exception if less than 4GB is available.  On Linux, you can use
    `ulimit -v 4000000` to reduce the available memory to slightly
    less than 4GB.
    """
    url = "https://github.com/Exiv2/exiv2/issues/791"

    filename = system_tests.path(
        "$data_path/issue_791_poc1.webp"
    )
    commands = ["$exiv2 $filename"]
    stdout = [""]
    stderr = ["""Exiv2 exception in print action for file $filename:
corrupted image metadata
"""
]
    retval = [1]
