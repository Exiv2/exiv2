# -*- coding: utf-8 -*-

import system_tests

class CanonSigmaLens_28mm_F14_DG_HSM_A(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1454"
    
    filename = "$data_path/Sigma_28mm_F1.4_DG_HSM_A_for_EOS.exv"
    commands = ["$exiv2 -pa --grep lensType/i $filename"]
    stderr = [""]
    stdout = [""
        """Exif.CanonCs.LensType                        Short       1  Sigma 28mm f/1.4 DG HSM | A
"""
]
    retval = [0]


class CanonSigmaLens_35mm_F14_DG_HSM_A(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1454"
    
    filename = "$data_path/Sigma_35mm_F1.4_DG_HSM_A_for_EOS.exv"
    commands = ["$exiv2 -pa --grep lensType/i $filename"]
    stderr = [""]
    stdout = [""
        """Exif.CanonCs.LensType                        Short       1  Sigma 35mm f/1.4 DG HSM | A
"""
]
    retval = [0]

class CanonSigmaLens_40mm_F14_DG_HSM_A(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1454"
    
    filename = "$data_path/Sigma_40mm_F1.4_DG_HSM_A_for_EOS.exv"
    commands = ["$exiv2 -pa --grep lensType/i $filename"]
    stderr = [""]
    stdout = [""
        """Exif.CanonCs.LensType                        Short       1  Sigma 40mm f/1.4 DG HSM | A
"""
]
    retval = [0]

class CanonSigmaLens_50mm_F14_DG_HSM_A(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1454"
    
    filename = "$data_path/Sigma_50mm_F1.4_DG_HSM_A_for_EOS.exv"
    commands = ["$exiv2 -pa --grep lensType/i $filename"]
    stderr = [""]
    stdout = [""
        """Exif.CanonCs.LensType                        Short       1  Sigma 50mm f/1.4 DG HSM | A
"""
]
    retval = [0]

class CanonSigmaLens_14_24mm_F28_DG_HSM_A(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1454"
    
    filename = "$data_path/Sigma_14-24mm_F2.8_DG_HSM_A_for_EOS.exv"
    commands = ["$exiv2 -pa --grep lensType/i $filename"]
    stderr = [""]
    stdout = [""
        """Exif.CanonCs.LensType                        Short       1  Sigma 14-24mm f/2.8 DG HSM | A
"""
]
    retval = [0]

class CanonSigmaLens_60_600mm_F45_63_DG_OS_HSM_S(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1454"
    
    filename = "$data_path/Sigma_60-600mm_F4.5-6.3_DG_OS_HSM_S_for_EOS.exv"
    commands = ["$exiv2 -pa --grep lensType/i $filename"]
    stderr = [""]
    stdout = [""
        """Exif.CanonCs.LensType                        Short       1  Sigma 60-600mm f/4.5-6.3 DG OS HSM | S
"""
]
    retval = [0]

