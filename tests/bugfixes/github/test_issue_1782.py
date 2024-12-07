# -*- coding: utf-8 -*-

import system_tests


class test_issue_1782(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/1782"

    filename = "$data_path/issue_1782_canon_lens_sigma_30mm_f1.4_DC_DN_C.exv"
    commands = ["$exiv2 -pa --grep LensType $filename"]
    stderr = [""]
    stdout = [
        """Exif.CanonCs.LensType                        Short       1  Sigma 30mm f/1.4 DC DN | C
Exif.CanonFi.RFLensType                      SShort      1  n/a
"""
    ]
    retval = [0]
