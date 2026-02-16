# -*- coding: utf-8 -*-

import system_tests


class NikonTokinaLens_ATX_i_11_20mm_F2_8_CF(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/2072"

    filename = "$data_path/Tokina_ATX-i_11-20mm_F2.8_CF.exv"
    commands = ["$exiv2 -pa --grep lensid/i $filename"]
    stderr = [""]
    stdout = [
        "Exif.NikonLd3.LensIDNumber                   Byte        1  Tokina ATX-i 11-20mm F2.8 CF\n"
    ]
    retval = [0]
