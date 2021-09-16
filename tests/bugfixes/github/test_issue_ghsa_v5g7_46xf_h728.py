# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path, check_no_ASAN_UBSAN_errors

class Jp2ImageEncodeJp2HeaderOutOfBoundsRead2(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-v5g7-46xf-h728
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-v5g7-46xf-h728"

    filename = path("$data_path/issue_ghsa_v5g7_46xf_h728_poc.exv")
    commands = ["$exiv2 $filename"]
    stdout = ["""File name       : $filename
File size       : 276 Bytes
MIME type       : application/rdf+xml
Image size      : 0 x 0
"""]
    stderr = ["""Error: XMP Toolkit error 201: Error in XMLValidator
Warning: Failed to decode XMP metadata.
$filename: No Exif data found in the file
"""]
    retval = [253]
