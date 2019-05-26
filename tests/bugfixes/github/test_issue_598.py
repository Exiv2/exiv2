# -*- coding: utf-8 -*-

import system_tests

class NikonTamronLens_A030_70_300_F4_56(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/598"

    filename = "$data_path/Tamron_SP70-300_F4-5.6_Di_VC_USD_A030.exv"
    commands = ["$exiv2 -pa --grep lensid/i $filename"]
    stderr = [""]
    stdout = [""
        """Exif.NikonLd3.LensIDNumber                   Byte        1  Tamron SP 70-300mm F/4-5.6 Di VC USD
"""
]
    retval = [0]
