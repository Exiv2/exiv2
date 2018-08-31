# -*- coding: utf-8 -*-

import system_tests

class CheckPentaxK10(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1144"

    filenames = ["$data_path/exiv2-bug1144a.exv",
                 "$data_path/exiv2-bug1144b.exv",
                 "$data_path/exiv2-bug1144c.exv",
                 "$data_path/exiv2-bug1144d.exv",
                 "$data_path/exiv2-bug1144e.exv",
                 "$data_path/exiv2-bug1144f.exv",
                 "$data_path/exiv2-bug1144g.exv",
    ]

    commands = [ "$exiv2 -pa -g Lens " + filenames[0],
                 "$exiv2 -pa -g Lens " + filenames[1],
                 "$exiv2 -pa -g Lens " + filenames[2],
                 "$exiv2 -pa -g Lens " + filenames[3],
                 "$exiv2 -pa -g Lens " + filenames[4],
                 "$exiv2 -pa -g Lens " + filenames[5],
                 "$exiv2 -pa -g Lens " + filenames[6],
    ]

    stdout = [ """Exif.PentaxDng.LensType                      Byte        3  Sigma AF 10-20mm F4-5.6 EX DC
Exif.PentaxDng.LensInfo                      Undefined  69  131 0 0 44 0 40 148 55 134 102 102 234 65 108 170 80 56 1 78 108 3 255 255 255 0 0 102 102 234 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
              """Exif.PentaxDng.LensType                      Byte        4  Sigma AF 10-20mm F4-5.6 EX DC
Exif.PentaxDng.LensInfo                      Undefined 128  0 131 128 0 0 44 0 80 0 0 0 0 0 0 0 0 40 148 55 99 83 134 237 65 64 136 80 56 1 64 108 3 255 255 255 0 0 83 134 237 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
              """Exif.PentaxDng.LensType                      Byte        4  Sigma AF 10-20mm F4-5.6 EX DC
Exif.PentaxDng.LensInfo                      Undefined 128  0 131 128 0 0 44 0 140 0 0 0 0 0 0 0 0 40 148 55 122 102 102 234 65 112 170 80 56 1 78 108 3 255 255 255 0 0 102 102 234 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
              """Exif.PentaxDng.LensType                      Byte        4  Sigma AF 10-20mm F4-5.6 EX DC
Exif.PentaxDng.LensInfo                      Undefined 128  0 131 128 0 0 44 0 160 0 0 0 0 0 0 0 0 40 148 55 98 104 70 234 65 128 170 80 56 1 80 108 3 255 255 255 0 0 104 70 234 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
              """Exif.PentaxDng.LensType                      Byte        3  Sigma AF 10-20mm F4-5.6 EX DC
Exif.PentaxDng.LensInfo                      Undefined  69  131 0 0 44 0 40 148 54 99 83 134 237 65 64 136 80 56 1 64 108 3 255 255 255 0 0 83 134 237 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
              """Exif.PentaxDng.LensType                      Byte        3  Sigma AF 10-20mm F4-5.6 EX DC
Exif.PentaxDng.LensInfo                      Undefined  69  131 0 0 44 0 40 148 55 134 102 102 234 65 108 170 80 56 1 78 108 3 255 255 255 0 0 102 102 234 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
""",
              """Exif.PentaxDng.LensType                      Byte        3  Sigma AF 10-20mm F4-5.6 EX DC
Exif.PentaxDng.LensInfo                      Undefined  69  131 0 0 44 0 40 148 55 98 104 70 234 65 128 170 80 56 1 80 108 3 255 255 255 0 0 104 70 234 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)
