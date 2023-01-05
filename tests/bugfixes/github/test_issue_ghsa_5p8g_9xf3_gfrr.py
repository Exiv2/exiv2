# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class WebPImageDoWriteMetadataOutOfBoundsRead(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-5p8g-9xf3-gfrr
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-5p8g-9xf3-gfrr"

    filename1 = path("$data_path/issue_ghsa_5p8g_9xf3_gfrr_poc.webp")
    filename2 = path("$data_path/issue_ghsa_5p8g_9xf3_gfrr_poc.exv")
    commands = ["$exiv2 in $filename1"]
    stdout = [""]
    stderr = [
"""Warning: Directory Nikon3, entry 0x002b has unknown Exif (TIFF) type 64002; setting type size 1.
Warning: Directory Nikon3, entry 0x002b has unknown Exif (TIFF) type 64002; setting type size 1.
$filename1: Could not write metadata to file: $kerCorruptedMetadata
"""]
    retval = [1]
