# -*- coding: utf-8 -*-

import system_tests

class Canon_Sigma_18_35_F18_DC_HSM(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/1368"
    
    filename1 = "$data_path/Canon_Sigma_18_35_F18_DC_HSM_firmware_1xx.exv"
    filename2 = "$data_path/Canon_Sigma_18_35_F18_DC_HSM_firmware_2xx.exv"
    commands  = ["$exiv2 -pa -K Exif.CanonCs.LensType $filename1",
                 "$exiv2 -pa -K Exif.CanonCs.LensType $filename2"]
    stderr = ["", ""]
    stdout = [
        """Exif.CanonCs.LensType                        Short       1  Sigma 18-35mm f/1.8 DC HSM | A
""",
        """Exif.CanonCs.LensType                        Short       1  Sigma 18-35mm f/1.8 DC HSM | A
"""
]
    retval = [0, 0]
