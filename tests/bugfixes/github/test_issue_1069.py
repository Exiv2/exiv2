# -*- coding: utf-8 -*-

import system_tests

class NikonTamronLens_F045_35_F14(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/1069"

    filename = "$data_path/Tamron_SP_35mm_f1.4_Di_USD_F045.exv"
    commands = ["$exiv2 -pa --grep lensid/i $filename"]
    stderr = [""]
    stdout = [""
        """Exif.NikonLd3.LensIDNumber                   Byte        1  Tamron SP 35mm f/1.4 Di USD
"""
]
    retval = [0]
