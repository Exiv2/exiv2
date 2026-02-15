# -*- coding: utf-8 -*-

import system_tests

class CanonSigmaLens_18_300_F35_63_DC_Macro_HSM(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1053"
    filename = "$data_path/Sigma_18-300mm_F3.5-6.3_DC_MACRO_HSM.exv"
    commands = ["$exiv2 -pa --grep lenstype/i $filename"]
    stderr = [""]
    stdout = [""
    """Exif.CanonCs.LensType                        Short       1  Sigma 18-300mm f/3.5-6.3 DC Macro HSM
"""
]
    retval = [0]
