# -*- coding: utf-8 -*-

import system_tests


class TestPoC(metaclass=system_tests.CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/168"

    stderr_common = """Error: Directory Image: IFD exceeds data buffer, cannot read next pointer.
Error: Offset of directory Image, entry 0x0117 is out of bounds: Offset = 0x30303030; truncating the entry
""" + 12 * """Error: Offset of directory Image, entry 0x3030 is out of bounds: Offset = 0x30303030; truncating the entry
"""

    filename = "$data_path/cve_2017_18005_reproducer.tiff"

    commands = [
        "$exiv2 -vPEIXxgklnycsvth -b " + filename,
        "$exiv2json " + filename
    ]

    stdout = ["""File 1/1: """ + filename + """
0x0117 Image        Exif.Image.StripByteCounts                   StripByteCounts             Strip Byte Count               SByte       0   0  

""",
    """{
	"Exif": {
		"Image": {
			"StripByteCounts": 0,
			"0x3030": 0,
			"0x3030": "",
			"0x3030": 0,
			"0x3030": 0,
			"0x3030": 0,
			"0x3030": 0,
			"0x3030": 0,
			"0x3030": 0,
			"0x3030": 0,
			"0x3030": 0,
			"0x3030": 0,
			"0x3030": 0,
			"0x3030": 0
		}
	}
}
"""
    ]
    stderr = [
        stderr_common + filename + """: No IPTC data found in the file
""" + filename + """: No XMP data found in the file
""",
        stderr_common
    ]
    retval = [0, 0]
