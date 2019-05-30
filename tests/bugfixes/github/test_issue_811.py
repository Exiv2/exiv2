# -*- coding: utf-8 -*-

import system_tests

class NikonSigmaLens_14_24_F28_DG_HSM_Art(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/811"

    filename = "$data_path/Sigma_14-24_F2.8_DG_HSM_Art.exv"
    commands = ["$exiv2 -pa --grep lensid/i $filename"]
    stderr = [""]
    stdout = [""
        """Exif.NikonLd3.LensIDNumber                   Byte        1  Sigma 14-24mm F2.8 DG HSM Art
"""
]
    retval = [0]
