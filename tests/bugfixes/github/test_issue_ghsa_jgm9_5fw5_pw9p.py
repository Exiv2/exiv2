# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class WebPImageDoWriteMetadataOutOfBoundsRead(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-jgm9-5fw5-pw9p
    """
    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-jgm9-5fw5-pw9p"

    filename1 = path("$data_path/issue_ghsa_jgm9_5fw5_pw9p_poc.jp2")
    filename2 = path("$data_path/issue_ghsa_jgm9_5fw5_pw9p_poc.exv")
    commands = ["$exiv2 in $filename1"]
    stdout = [""]
    stderr = [
"""Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x010f has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x010f has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x0000 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x8769 has unknown Exif (TIFF) type 0; setting type size 1.
Warning: Directory Image, entry 0x8769 doesn't look like a sub-IFD.
Warning: Directory Image, entry 0x8825 doesn't look like a sub-IFD.
$filename1: Could not write metadata to file: $kerCorruptedMetadata
"""]
    retval = [1]
