# -*- coding: utf-8 -*-
from system_tests import CaseMeta


class Canon_EF_100_400mm_F45_56_2x(metaclass=CaseMeta):
    filename = "$data_path/canon_ef_100_400mm_f4.5_5.6_2x_.exv"
    commands = ["$exiv2 -pa -g LensType $filename"]

    stdout = [
        "Exif.CanonCs.LensType                        Short       1  Tamron 100-400mm f/4.5-6.3 Di VC USD + 2x *OR* Canon EF 100-400mm f/4.5-5.6L IS II USM + 2x\n"
    ]
    stderr = [""]
    retval = [0]
