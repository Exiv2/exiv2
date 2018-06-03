# -*- coding: utf-8 -*-

import system_tests

class CheckSonyMinoltaLenses(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1145"

    filenames = ["$data_path/exiv2-bug1145a.exv",
                 "$data_path/exiv2-bug1145b.exv",
                 "$data_path/exiv2-bug1145c.exv",
                 "$data_path/exiv2-bug1145d.exv",
                 "$data_path/exiv2-bug1145e.exv",
    ]

    commands = [ "$exiv2 -pa -g Lens " + filenames[0],
                 "$exiv2 -pa -g Lens " + filenames[1],
                 "$exiv2 -pa -g Lens " + filenames[2],
                 "$exiv2 -pa -g Lens " + filenames[3],
                 "$exiv2 -pa -g Lens " + filenames[4],
    ]

    stdout = [ """Exif.Sony1.LensID                            Long        1  Tamron SP AF 11-18mm F4.5-5.6 Di II LD Aspherical IF
Exif.Photo.LensSpecification                 Rational    4  110/10 180/10 45/10 56/10
Exif.Photo.LensModel                         Ascii      20  DT 11-18mm F4.5-5.6
""",
              """Exif.Sony1.LensID                            Long        1  Tamron SP AF 17-50mm F2.8 XR Di II LD Aspherical
Exif.Photo.LensSpecification                 Rational    4  0/10 0/10 0/10 0/10
Exif.Photo.LensModel                         Ascii       5  ----
""",
              """Exif.Sony1.LensID                            Long        1  Tamron AF 18-200mm F3.5-6.3 XR Di II LD Aspherical [IF] Macro
Exif.Photo.LensSpecification                 Rational    4  0/10 0/10 0/10 0/10
Exif.Photo.LensModel                         Ascii       5  ----
""",
              """Exif.Sony1.LensID                            Long        1  Tamron SP 70-300mm F4-5.6 Di USD
Exif.Photo.LensSpecification                 Rational    4  700/10 3000/10 40/10 56/10
Exif.Photo.LensModel                         Ascii      16  70-300mm F4-5.6
""",
              """Exif.Sony1.LensID                            Long        1  Tamron SP AF 90mm F2.8 Di Macro
Exif.Photo.LensSpecification                 Rational    4  1000/10 1000/10 28/10 28/10
Exif.Photo.LensModel                         Ascii      17  100mm F2.8 Macro
""",
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)
