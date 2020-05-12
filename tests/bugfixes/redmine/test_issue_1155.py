# -*- coding: utf-8 -*-

import system_tests

class CheckPentaxK3(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1155"

    filenames = ["$data_path/exiv2-bug1155a.exv",
                 "$data_path/exiv2-bug1155b.exv",
                 "$data_path/exiv2-bug1155c.exv",
                 "$data_path/exiv2-bug1155d.exv",
                 "$data_path/exiv2-bug1155e.exv",
                 "$data_path/exiv2-bug1155f.exv",
                 "$data_path/exiv2-bug1155g.exv",
                 "$data_path/exiv2-bug1155h.exv",
                 "$data_path/exiv2-bug1155i.exv",
    ]

    commands = [ "$exiv2 -pa -b -g Lens " + filenames[0],
                 "$exiv2 -pa -b -g Lens " + filenames[1],
                 "$exiv2 -pa -b -g Lens " + filenames[2],
                 "$exiv2 -pa -b -g Lens " + filenames[3],
                 "$exiv2 -pa -b -g Lens " + filenames[4],
                 "$exiv2 -pa -b -g Lens " + filenames[5],
                 "$exiv2 -pa -b -g Lens " + filenames[6],
                 "$exiv2 -pa -b -g Lens " + filenames[7],
                 "$exiv2 -pa -b -g Lens " + filenames[8],
    ]

    stdout = [ """Exif.PentaxDng.LensType                      Byte        4  Sigma 70-300mm F4-5.6 Macro
Exif.PentaxDng.LensInfo                      Undefined 128  0 131 128 0 0 255 5 240 0 0 0 0 0 0 0 0 40 148 110 65 69 6 238 65 78 153 80 40 1 73 107 251 255 255 255 0 0 69 6 238 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
              """Exif.PentaxDng.LensType                      Byte        4  Sigma 18-50mm F2.8-4.5 HSM OS
Exif.PentaxDng.LensInfo                      Undefined 128  0 168 144 114 0 255 1 144 0 0 0 0 0 0 0 0 40 214 30 20 144 3 19 109 81 155 113 32 1 69 108 251 255 255 255 255 41 144 3 19 0 119 7 97 0 0 0 0 239 255 255 224 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
              """Exif.PentaxDng.LensType                      Byte        4  Sigma Zoom 70-210mm F4-5.6 UC-II
Exif.PentaxDng.LensInfo                      Undefined 128  0 131 128 2 0 25 6 213 0 0 0 0 0 0 0 0 40 148 103 108 169 6 248 65 86 170 64 44 1 80 107 251 255 255 255 0 0 169 6 248 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
              """Exif.Pentax.LensType                         Byte        2  Sigma 70-300mm F4-5.6 Macro
Exif.Pentax.LensInfo                         Undefined  44  3 255 0 0 40 148 110 102 84 38 240 65 121 136 88 74 1 64 107 251 255 255 255 0 0 84 38 240 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
              """Exif.Pentax.LensType                         Byte        2  Sigma Zoom 70-210mm F4-5.6 UC-II
Exif.Pentax.LensInfo                         Undefined  44  3 25 0 0 40 148 100 108 169 6 248 65 86 170 64 44 1 80 107 251 255 255 255 0 0 169 6 248 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
              """Exif.Pentax.LensType                         Byte        2  Sigma AF 10-20mm F4-5.6 EX DC
Exif.Pentax.LensInfo                         Undefined  44  3 44 0 0 40 148 52 75 84 134 236 65 72 137 80 56 1 68 108 3 255 255 255 0 0 84 134 236 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
              """Exif.Pentax.LensType                         Byte        2  Sigma AF 10-20mm F4-5.6 EX DC
Exif.Pentax.LensInfo                         Undefined  36  3 44 0 0 40 148 52 83 83 134 236 65 68 137 80 56 1 66 108 3 255 255 255 0 0 83 134 236 0 0 0 0 0 0 0 0
""",
              """Exif.Pentax.LensType                         Byte        2  Sigma Zoom 70-210mm F4-5.6 UC-II
Exif.Pentax.LensInfo                         Undefined  36  3 25 0 0 40 148 102 85 243 6 248 65 177 153 96 62 1 69 107 251 255 255 255 0 0 243 6 248 0 0 0 0 0 0 0 0
""",
              """Exif.Pentax.LensType                         Byte        2  Sigma 70-300mm F4-5.6 Macro
Exif.Pentax.LensInfo                         Undefined  36  3 255 0 0 40 148 110 114 84 38 240 65 113 136 88 77 1 64 107 251 255 255 255 0 0 84 38 240 0 0 0 0 0 0 0 0
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)
