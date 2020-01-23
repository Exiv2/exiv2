import system_tests


class BigTiffImageRecursionStackExhaustion(
        metaclass=system_tests.CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/issues/712

    A malicious input file can cause BigTiffImage::printIFD() to
    recurse arbitrarily deep, causing a crash due to stack exhaustion.

    The bug only existed in the -pR mode, which is now only enabled
    in debug builds.
    """
    url = "https://github.com/Exiv2/exiv2/issues/790"

    filename = system_tests.path(
        "$data_path/issue_712_poc.tif"
    )
    commands = ["$exiv2 -b -u -k pr $filename"]
    stdout = ["File name       : " + filename + """
File size       : 3720 Bytes
MIME type       : 
Image size      : 0 x 0
"""
]
    stderr = [filename + """: No Exif data found in the file
"""]
    retval = [253]
