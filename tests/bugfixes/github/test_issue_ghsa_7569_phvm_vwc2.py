# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class Jp2ImageDoWriteMetadataOutOfBoundsRead(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-7569-phvm-vwc2
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-7569-phvm-vwc2"

    filename1 = path("$data_path/issue_ghsa_7569_phvm_vwc2_poc.jp2")
    filename2 = path("$data_path/issue_ghsa_7569_phvm_vwc2_poc.exv")
    commands = ["$exiv2 in $filename1"]
    stdout = [""]
    stderr = [
"""Warning: Directory Thumbnail, entry 0x1000 has unknown Exif (TIFF) type 28928; setting type size 1.
Error: Directory Thumbnail: IFD entry 1 lies outside of the data buffer.
Warning: Directory Thumbnail, entry 0x1000 has unknown Exif (TIFF) type 28928; setting type size 1.
Error: Offset of directory Thumbnail, entry 0x1000 is out of bounds: Offset = 0x2020506a; truncating the entry
$filename1: Could not write metadata to file: $kerCorruptedMetadata
"""]
    retval = [1]
