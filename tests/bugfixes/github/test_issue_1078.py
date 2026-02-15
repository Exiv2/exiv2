# -*- coding: utf-8 -*-

import system_tests

class NikonTokinaLens_atx_i_11_16_F28(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/1078"

    filename = "$data_path/Tokina_atx-i_11-16mm_F2.8_CF.exv"
    commands = ["$exiv2 -pa --grep lensid/i $filename"]
    stderr = [""]
    stdout = [""
        """Exif.NikonLd3.LensIDNumber                   Byte        1  Tokina atx-i 11-16mm F2.8 CF
"""
]
    retval = [0]
