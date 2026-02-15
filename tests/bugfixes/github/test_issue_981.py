# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path

class CanonAfInfoTest(metaclass=CaseMeta):

    filenameA = path("$data_path/test_issue_981a.exv")
    filenameB = path("$data_path/test_issue_981b.exv")
    filenameC = path("$data_path/test_issue_981c.exv")
    filenameC = path("$data_path/test_issue_981c.exv")
    filenameD = path("$data_path/test_issue_981d.exv")
    commands  = ["$exiv2 -pa --grep Canon.AF $filenameA",
                 "$exiv2 -pa --grep Canon.AF $filenameB",
                 "$exiv2 -pv --grep Points   $filenameC",
                 "$exiv2 -pt --grep Points   $filenameC",
                 "$exiv2 -pv --grep Primary  $filenameD",
                 "$exiv2 -pt --grep Primary  $filenameD",
                ]

    stdout = ["""Exif.Canon.AFInfo                            Short      48  96 2 9 9 4752 3168 4272 2848 115 115 115 162 200 162 115 115 115 153 153 153 105 199 105 153 153 153 64409 64862 64862 0 0 0 674 674 1127 0 321 65215 603 0 64933 321 65215 0 16 256 0 65535
Exif.Canon.AFInfoSize                        SShort      1  96
Exif.Canon.AFAreaMode                        SShort      1  Single-point AF
Exif.Canon.AFNumPoints                       SShort      1  9
Exif.Canon.AFValidPoints                     SShort      1  9
Exif.Canon.AFCanonImageWidth                 SShort      1  4752
Exif.Canon.AFCanonImageHeight                SShort      1  3168
Exif.Canon.AFImageWidth                      SShort      1  4272
Exif.Canon.AFImageHeight                     SShort      1  2848
Exif.Canon.AFAreaWidths                      SShort      9  115 115 115 162 200 162 115 115 115
Exif.Canon.AFAreaHeights                     SShort      9  153 153 153 105 199 105 153 153 153
Exif.Canon.AFXPositions                      SShort      9  -1127 -674 -674 0 0 0 674 674 1127
Exif.Canon.AFYPositions                      SShort      9  0 321 -321 603 0 -603 321 -321 0
Exif.Canon.AFPointsInFocus                   Short       1  4
Exif.Canon.AFPointsSelected                  Short       1  8
Exif.Canon.AFPrimaryPoint                    Short       1  (none)
""" , """Exif.Canon.AFInfo                            Short     273  546 2 63 61 6720 4480 6720 4480 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 0 0 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 0 0 65200 64790 64435 64099 63764 336 0 65200 64099 63764 1772 1437 1101 746 336 0 1437 1101 746 336 0 65200 64790 64435 336 0 65200 64790 64435 64099 63764 1772 64790 64435 64099 63764 1772 1437 1101 746 63764 1772 1437 1101 746 336 0 65200 1101 746 336 0 65200 64790 64435 64099 336 0 65200 1772 1437 0 0 547 625 625 625 625 821 821 821 308 308 625 625 625 625 547 547 308 308 308 274 274 274 308 308 0 0 0 0 0 0 0 308 65228 65228 65228 65228 0 0 0 0 64911 65228 65228 65228 65228 65262 65262 65262 64911 64911 64989 64989 64989 64911 64911 64911 64715 64715 64715 64911 64911 0 0 0 512 0 0 0 512 0 0 0 0 0 0 65535
Exif.Canon.AFInfoSize                        SShort      1  546
Exif.Canon.AFAreaMode                        SShort      1  Single-point AF
Exif.Canon.AFNumPoints                       SShort      1  63
Exif.Canon.AFValidPoints                     SShort      1  61
Exif.Canon.AFCanonImageWidth                 SShort      1  6720
Exif.Canon.AFCanonImageHeight                SShort      1  4480
Exif.Canon.AFImageWidth                      SShort      1  6720
Exif.Canon.AFImageHeight                     SShort      1  4480
Exif.Canon.AFAreaWidths                      SShort     63  218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 0 0
Exif.Canon.AFAreaHeights                     SShort     63  218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 218 0 0
Exif.Canon.AFXPositions                      SShort     63  -336 -746 -1101 -1437 -1772 336 0 -336 -1437 -1772 1772 1437 1101 746 336 0 1437 1101 746 336 0 -336 -746 -1101 336 0 -336 -746 -1101 -1437 -1772 1772 -746 -1101 -1437 -1772 1772 1437 1101 746 -1772 1772 1437 1101 746 336 0 -336 1101 746 336 0 -336 -746 -1101 -1437 336 0 -336 1772 1437 0 0
Exif.Canon.AFYPositions                      SShort     63  547 625 625 625 625 821 821 821 308 308 625 625 625 625 547 547 308 308 308 274 274 274 308 308 0 0 0 0 0 0 0 308 -308 -308 -308 -308 0 0 0 0 -625 -308 -308 -308 -308 -274 -274 -274 -625 -625 -547 -547 -547 -625 -625 -625 -821 -821 -821 -625 -625 0 0
Exif.Canon.AFPointsInFocus                   Short       4  25
Exif.Canon.AFPointsSelected                  Short       4  25
Exif.Canon.AFPrimaryPoint                    Short       4  (none)
""","""0x2602 Canon        AFNumPoints                 SShort      1  63
0x2603 Canon        AFValidPoints               SShort      1  61
0x260c Canon        AFPointsInFocus             Short       4  0 560 57344 0
0x260d Canon        AFPointsSelected            Short       4  0 1848 57344 0
""","""Exif.Canon.AFNumPoints                       SShort      1  63
Exif.Canon.AFValidPoints                     SShort      1  61
Exif.Canon.AFPointsInFocus                   Short       4  20,21,25,45,46,47
Exif.Canon.AFPointsSelected                  Short       4  19,20,21,24,25,26,45,46,47
""","""0x260e Canon        AFPrimaryPoint              Short       4  3608 49152 792 6272
""","""Exif.Canon.AFPrimaryPoint                    Short       4  3,4,9,10,11,30,31,35,36,40,41,55,59,60
"""
]
    stderr = [""]*len(commands)
    retval = [ 0]*len(commands)
