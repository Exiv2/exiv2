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

    commands = [ f"$exiv2 -pa -g Lens {filenames[0]}",
                 f"$exiv2 -pa -g Lens {filenames[1]}",
                 f"$exiv2 -pa -g Lens {filenames[2]}",
                 f"$exiv2 -pa -g Lens {filenames[3]}",
                 f"$exiv2 -pa -g Lens {filenames[4]}",
    ]

    stdout = [ """Exif.Sony1.LensID                            Long        1  Tamron SP AF 11-18mm F4.5-5.6 Di II LD Aspherical IF
Exif.Sony1.LensSpec                          Byte        8  Unknown
Exif.Photo.LensSpecification                 Rational    4  11-18mm F4.5-5.6
Exif.Photo.LensModel                         Ascii      20  DT 11-18mm F4.5-5.6
""",
              """Exif.Sony1.LensID                            Long        1  Tamron SP AF 17-50mm F2.8 XR Di II LD Aspherical
Exif.Sony1.LensSpec                          Byte        8  Unknown
Exif.Photo.LensSpecification                 Rational    4  n/a
Exif.Photo.LensModel                         Ascii       5  ----
""",
              """Exif.Sony1.LensID                            Long        1  Tamron AF 18-200mm F3.5-6.3 XR Di II LD Aspherical [IF] Macro
Exif.Sony1.LensSpec                          Byte        8  Unknown
Exif.Photo.LensSpecification                 Rational    4  n/a
Exif.Photo.LensModel                         Ascii       5  ----
""",
              """Exif.Sony1.LensID                            Long        1  Tamron SP 70-300mm F4-5.6 Di USD
Exif.Sony1.LensSpec                          Byte        8  70-300mm F4.0-5.6
Exif.Photo.LensSpecification                 Rational    4  70-300mm F4-5.6
Exif.Photo.LensModel                         Ascii      16  70-300mm F4-5.6
""",
              """Exif.Sony1.LensID                            Long        1  Tamron SP AF 90mm F2.8 Di Macro
Exif.Sony1.LensSpec                          Byte        8  Unknown
Exif.Photo.LensSpecification                 Rational    4  100mm F2.8
Exif.Photo.LensModel                         Ascii      17  100mm F2.8 Macro
""",
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)
