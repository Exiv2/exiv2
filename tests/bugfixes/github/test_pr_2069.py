# -*- coding: utf-8 -*-

import system_tests


class NikonSigmaLens_APO_MACRO_180_F35_EX_DG_HSM(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/2069"

    filename = "$data_path/Sigma_12-24mm_F4_DG_HSM_Art.exv"
    commands = ["$exiv2 -pa --grep lensid/i $filename"]
    stderr = [""]
    stdout = [
        ""
        """Exif.NikonLd3.LensIDNumber                   Byte        1  Sigma 12-24mm F4 DG HSM | A
"""
    ]
    retval = [0]
