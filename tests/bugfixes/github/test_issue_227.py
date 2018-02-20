# -*- coding: utf-8 -*-

import system_tests


class SigmaLenses(system_tests.Case):

    files = [
        "Sigma_120-300_DG_OS_HSM_Sport_lens.exv",
        "Sigma_20mm_F1.4_DG_HSM_A.exv",
        "Sigma_50mm_F1.4_DG_HSM_A.exv"
    ]

    commands = list(
        map(lambda fname: "{exiv2} -pa --grep lens/i {data_path}/" + fname, files)
    )

    retval = 3 * [0]
    stderr = 3 * [""]
    stdout = [
        """Exif.Nikon3.LensType                         Byte        1  D G VR
Exif.Nikon3.Lens                             Rational    4  120-300mm F2.8
Exif.Nikon3.LensFStops                       Undefined   4  6
Exif.NikonLd3.LensIDNumber                   Byte        1  Sigma 120-300mm F2.8 DG OS HSM | S
Exif.NikonLd3.LensFStops                     Byte        1  F6.0
""",
        """Exif.Nikon3.LensType                         Byte        1  D G 
Exif.Nikon3.Lens                             Rational    4  20mm F1.4
Exif.Nikon3.LensFStops                       Undefined   4  7
Exif.NikonLd3.LensIDNumber                   Byte        1  Sigma 20mm F1.4 DG HSM | A
Exif.NikonLd3.LensFStops                     Byte        1  F7.0
""",
        """Exif.Nikon3.LensType                         Byte        1  D G 
Exif.Nikon3.Lens                             Rational    4  50mm F1.4
Exif.Nikon3.LensFStops                       Undefined   4  7
Exif.NikonLd3.LensIDNumber                   Byte        1  Sigma 50mm F1.4 DG HSM | A
Exif.NikonLd3.LensFStops                     Byte        1  F7.0
"""
    ]
