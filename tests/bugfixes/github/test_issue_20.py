# -*- coding: utf-8 -*-

import system_tests


class TamronSupport(metaclass=system_tests.CaseMeta):

    description = "Added support for 'Tamron SP 15-30mm f/2.8 Di VC USD' and 'Tamron SP 90mm f/2.8 Di VC USD MACRO1:1'"

    files = [
        "exiv2-g20.exv",
        "CanonEF100mmF2.8LMacroISUSM.exv",
        "TamronSP15-30mmF2.8DiVCUSDA012.exv",
        "TamronSP90mmF2.8DiVCUSDMacroF004.exv",
        "TamronSP90mmF2.8DiVCUSDMacroF017.exv"
    ]
    commands = ["$exiv2 -pa --grep lens/i ../../../test/data/" + files[0]] \
        + list(map(
            lambda fname: "$exiv2 -pa --grep lenstype/i ../../../test/data/" + fname,
            files[1:]
        ))
    retval = [0] * len(files)

    stdout = [
        # exiv2-g20.exv
        """Exif.CanonCs.LensType                        Short       1  Tamron SP 90mm f/2.8 Di VC USD Macro 1:1
Exif.CanonCs.Lens                            Short       3  90.0 mm
Exif.Canon.LensModel                         Ascii      70  TAMRON SP 90mm F/2.8 Di VC USD MACRO1:1 F017
Exif.Photo.LensSpecification                 Rational    4  90/1 90/1 0/0 0/0
Exif.Photo.LensModel                         Ascii      70  TAMRON SP 90mm F/2.8 Di VC USD MACRO1:1 F017
Exif.Photo.LensSerialNumber                  Ascii      12  0000000000
""",

        # CanonEF100mmF2.8LMacroISUSM.exv
        """Exif.CanonCs.LensType                        Short       1  Canon EF 100mm f/2.8L Macro IS USM
""",

        # TamronSP15-30mmF2.8DiVCUSDA012.exv"
        """Exif.CanonCs.LensType                        Short       1  Tamron SP 15-30mm f/2.8 Di VC USD
""",

        # TamronSP90mmF2.8DiVCUSDMacroF004.exv
        """Exif.CanonCs.LensType                        Short       1  Tamron SP 90mm f/2.8 Di VC USD Macro 1:1
""",

        # TamronSP90mmF2.8DiVCUSDMacroF017.exv
        """Exif.CanonCs.LensType                        Short       1  Tamron SP 90mm f/2.8 Di VC USD Macro 1:1
"""
        ""]
    stderr = [""] * len(files)
