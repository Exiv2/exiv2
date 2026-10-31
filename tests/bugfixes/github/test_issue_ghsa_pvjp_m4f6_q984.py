# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class MinoltaDivZero(metaclass=CaseMeta):
    """
    Regression test for the bug described in:
    https://github.com/Exiv2/exiv2/security/advisories/GHSA-pvjp-m4f6-q984
    """

    url = "https://github.com/Exiv2/exiv2/security/advisories/GHSA-pvjp-m4f6-q984"

    filename = path("$data_path/issue_ghsa_pvjp_m4f6_q984_poc.exv")
    commands = ["$exiv2 -p t $filename"]
    stderr = [
        """Error: Directory Image, entry 0x011a has invalid size 1769473*8; skipping entry.
Error: Offset of directory Photo, entry 0x829a is out of bounds: Offset = 0x6d00035e; truncating the entry
Error: Directory Photo, entry 0x8822 has invalid size 28417*2; skipping entry.
Error: Directory Photo, entry 0x8827 has invalid size 589825*2; skipping entry.
Warning: Directory Photo, entry 0x8832 has unknown Exif (TIFF) type 49; setting type size 1.
Error: Directory Sony2, entry 0x2006 has invalid size 11665409*4; skipping entry.
Warning: Directory Sony2, entry 0x20c1 has unknown Exif (TIFF) type 181; setting type size 1.
Error: Offset of directory Sony2, entry 0x3000 is out of bounds: Offset = 0x0057097c; truncating the entry
Error: Directory Sony2, entry 0x0115 has invalid size 14942209*4; skipping entry.
Error: Directory Sony2, entry 0x2013 has invalid size 9371649*4; skipping entry.
Warning: Directory Photo, entry 0xa003 has unknown Exif (TIFF) type 242; setting type size 1.
Warning: Directory Iop has an unexpected next pointer; ignored.
Warning: Directory Photo, entry 0xa402 has unknown Exif (TIFF) type 89; setting type size 1.
Error: Offset of directory Thumbnail, entry 0x0132 is out of bounds: Offset = 0xff00968b; truncating the entry
"""
    ]
    retval = [0]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """We don't care about the stdout, just don't crash"""
