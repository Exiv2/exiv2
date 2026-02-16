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
    stderr = ["""Error: Upper boundary of data for directory Image, entry 0x011a is out of bounds: Offset = 0x000000f2, size = 14155784, exceeds buffer size by 14110766 Bytes; truncating the entry
Error: Offset of directory Photo, entry 0x829a is out of bounds: Offset = 0x6d00035e; truncating the entry
Error: Upper boundary of data for directory Photo, entry 0x8822 is out of bounds: Offset = 0x00000003, size = 56834, exceeds buffer size by 11577 Bytes; truncating the entry
Error: Upper boundary of data for directory Photo, entry 0x8827 is out of bounds: Offset = 0x00000640, size = 1179650, exceeds buffer size by 1135990 Bytes; truncating the entry
Warning: Directory Photo, entry 0x8832 has unknown Exif (TIFF) type 49; setting type size 1.
Error: Offset of directory Sony2, entry 0x2006 is out of bounds: Offset = 0x00000000; truncating the entry
Warning: Directory Sony2, entry 0x20c1 has unknown Exif (TIFF) type 181; setting type size 1.
Error: Offset of directory Sony2, entry 0x2063 is out of bounds: Offset = 0x00000000; truncating the entry
Error: Offset of directory Sony2, entry 0x3000 is out of bounds: Offset = 0x0057097c; truncating the entry
Error: Offset of directory Sony2, entry 0x0115 is out of bounds: Offset = 0x00000000; truncating the entry
Error: Upper boundary of data for directory Sony2, entry 0x2013 is out of bounds: Offset = 0x00000002, size = 37486596, exceeds buffer size by 37441338 Bytes; truncating the entry
Warning: Directory Photo, entry 0xa003 has unknown Exif (TIFF) type 242; setting type size 1.
Warning: Directory Iop has an unexpected next pointer; ignored.
Warning: Directory Photo, entry 0xa402 has unknown Exif (TIFF) type 89; setting type size 1.
Error: Offset of directory Photo, entry 0xa402 is out of bounds: Offset = 0x00000000; truncating the entry
Error: Offset of directory Thumbnail, entry 0x0132 is out of bounds: Offset = 0xff00968b; truncating the entry
"""]
    retval = [0]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """ We don't care about the stdout, just don't crash """
        pass
