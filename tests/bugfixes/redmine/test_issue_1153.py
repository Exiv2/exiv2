# -*- coding: utf-8 -*-

import system_tests
import itertools

class CheckSony6000WithoutLensModels(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1153"

    filenames = [
        "$data_path/exiv2-bug1153{E}{i}.exv".format(E=E, i=i)
        for E, i in itertools.product(
                ['A', 'J'],
                "a b c d e f g h i j k".split()
        )
    ]

    commands = [
        "$exiv2 -pa -g Lens {!s}".format(fname) for fname in filenames
    ]

    stdout = [
        """Exif.Sony2.LensID                            Long        1  Sony E 50mm F1.8 OSS
Exif.Photo.LensSpecification                 Rational    4  500/10 500/10 18/10 18/10
Exif.Photo.LensModel                         Ascii      16  E 50mm F1.8 OSS
""",
        """Exif.Sony2.LensID                            Long        1  Sony E 50mm F1.8 OSS
Exif.Photo.LensSpecification                 Rational    4  500/10 500/10 18/10 18/10
Exif.Photo.LensModel                         Ascii      16  E 50mm F1.8 OSS
""",
        """Exif.Sony2.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
Exif.Photo.LensSpecification                 Rational    4  160/10 500/10 35/10 56/10
Exif.Photo.LensModel                         Ascii      26  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony2.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
Exif.Photo.LensSpecification                 Rational    4  160/10 500/10 35/10 56/10
Exif.Photo.LensModel                         Ascii      26  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony2.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
Exif.Photo.LensSpecification                 Rational    4  160/10 500/10 35/10 56/10
Exif.Photo.LensModel                         Ascii      26  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony2.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
Exif.Photo.LensSpecification                 Rational    4  160/10 500/10 35/10 56/10
Exif.Photo.LensModel                         Ascii      26  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony2.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
Exif.Photo.LensSpecification                 Rational    4  160/10 500/10 35/10 56/10
Exif.Photo.LensModel                         Ascii      26  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony2.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
Exif.Photo.LensSpecification                 Rational    4  160/10 500/10 35/10 56/10
Exif.Photo.LensModel                         Ascii      26  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony2.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
Exif.Photo.LensSpecification                 Rational    4  160/10 500/10 35/10 56/10
Exif.Photo.LensModel                         Ascii      26  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony2.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
Exif.Photo.LensSpecification                 Rational    4  160/10 500/10 35/10 56/10
Exif.Photo.LensModel                         Ascii      26  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony2.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
Exif.Photo.LensSpecification                 Rational    4  160/10 500/10 35/10 56/10
Exif.Photo.LensModel                         Ascii      26  E PZ 16-50mm F3.5-5.6 OSS
""",

        """Exif.Sony1.LensID                            Long        1  Sony E 50mm F1.8 OSS
""",
        """Exif.Sony1.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony1.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony1.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony1.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony1.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony1.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony1.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony1.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony1.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
""",
        """Exif.Sony1.LensID                            Long        1  E PZ 16-50mm F3.5-5.6 OSS
""",
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)
