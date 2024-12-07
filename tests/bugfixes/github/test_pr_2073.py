# -*- coding: utf-8 -*-

import system_tests


class NikonTokinaLens_AT_X_14_20_F2_PRO_DX(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/2073"

    filename = "$data_path/Tokina_AT-X_14-20_F2_PRO_DX.exv"
    commands = ["$exiv2 -pa --grep lensid/i $filename"]
    stderr = [""]
    stdout = [
        "Exif.NikonLd3.LensIDNumber                   Byte        1  Tokina AT-X 14-20 F2 PRO DX (AF 14-20mm f/2)\n"
    ]
    retval = [0]
