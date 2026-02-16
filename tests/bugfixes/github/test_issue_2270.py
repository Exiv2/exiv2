# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path


@CopyTmpFiles("$data_path/issue_2270_poc.webp")
class WebPImage_inject_VP8X_integer_overflow(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/2270
    """

    url = "https://github.com/Exiv2/exiv2/issues/2270"

    filename = path("$tmp_path/issue_2270_poc.webp")
    commands = ["$exiv2 rm $filename"]
    stdout = [""]
    stderr = [
        f"""$exception_in_erase {filename}:
$kerCorruptedMetadata
"""
    ]
    retval = [1]
