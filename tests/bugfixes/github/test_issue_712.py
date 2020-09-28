import system_tests


class BigTiffImageRecursionStackExhaustion(
        metaclass=system_tests.CaseMeta):
    """
    src/bigtiffimage.cpp is longer in the code base
    however, let's retain this test as support for BigTiff will
    be developed and the malicious test file may stress the bigtiff parser.
    
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
    stdout = [""]
    stderr = [
        """$exiv2_exception_message """ + filename + """:
$filename: $kerFileContainsUnknownImageType
"""]
    retval = [1]
