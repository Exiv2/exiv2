# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path, check_no_ASAN_UBSAN_errors

class coverage_xmpsidecar_isXmpType(metaclass=CaseMeta):
    """
    Test added to improve code coverage in xmpsidecar.cpp after
    Codecov complained about a lack of code coverage in this PR:
    https://github.com/Exiv2/exiv2/pull/1786
    """

    filename = path("$data_path/coverage_xmpsidecar_isXmpType.xmp")
    commands = ["$exiv2 $filename"]
    stderr = ["""Error: XMP Toolkit error 201: XML parsing failure
Warning: Failed to decode XMP metadata.
$filename: No Exif data found in the file
"""]
    retval = [253]

    compare_stdout = check_no_ASAN_UBSAN_errors
