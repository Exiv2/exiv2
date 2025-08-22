# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


# Full support for SonyMisc3c tags using this model
class SonyMisc3cSupportedTest(metaclass=CaseMeta):
    # Check that JPEG and TIFF based formats work
    filenames = [
        path("$data_path/exiv2-SonyDSC-HX60V.exv"),  # JPEG
        path("$data_path/exiv2-bug1153Aa.exv"),
    ]  # TIFF
    commands = [f"$exiv2 -pa --grep SonyMisc3c {fname!s}" for fname in filenames]

    stdout = [
        """Exif.SonyMisc3c.ReleaseMode2                 Byte        1  Normal
Exif.SonyMisc3c.ShotNumberSincePowerUp       Long        1  4
Exif.SonyMisc3c.SequenceImageNumber          Long        1  1
Exif.SonyMisc3c.SequenceLength1              Byte        1  1 shot
Exif.SonyMisc3c.SequenceFileNumber           Long        1  1
Exif.SonyMisc3c.SequenceLength2              Byte        1  1 file
Exif.SonyMisc3c.CameraOrientation            Byte        1  Horizontal (normal)
Exif.SonyMisc3c.Quality2                     Byte        1  JPEG
Exif.SonyMisc3c.SonyImageHeight              Short       1  3888
Exif.SonyMisc3c.ModelReleaseYear             Byte        1  2014
""",
        """Exif.SonyMisc3c.ReleaseMode2                 Byte        1  Normal
Exif.SonyMisc3c.ShotNumberSincePowerUp       Long        1  2
Exif.SonyMisc3c.SequenceImageNumber          Long        1  1
Exif.SonyMisc3c.SequenceLength1              Byte        1  1 shot
Exif.SonyMisc3c.SequenceFileNumber           Long        1  1
Exif.SonyMisc3c.SequenceLength2              Byte        1  1 file
Exif.SonyMisc3c.CameraOrientation            Byte        1  Horizontal (normal)
Exif.SonyMisc3c.Quality2                     Byte        1  Raw + JPEG
Exif.SonyMisc3c.SonyImageHeight              Short       1  4000
Exif.SonyMisc3c.ModelReleaseYear             Byte        1  2014
""",
    ]
    stderr = [""] * 2
    retval = [0] * 2


# Partial support for SonyMisc3c tags using this model
class SonyMisc3cPartiallySupportedTest(metaclass=CaseMeta):
    filename = path("$data_path/exiv2-SonyILCE-7SM3.exv")
    commands = ["$exiv2 -pa --grep SonyMisc3c $filename"]

    stdout = [
        """Exif.SonyMisc3c.ReleaseMode2                 Byte        1  Normal
Exif.SonyMisc3c.ShotNumberSincePowerUp       Long        1  n/a
Exif.SonyMisc3c.SequenceImageNumber          Long        1  1
Exif.SonyMisc3c.SequenceLength1              Byte        1  1 shot
Exif.SonyMisc3c.SequenceFileNumber           Long        1  1
Exif.SonyMisc3c.SequenceLength2              Byte        1  1 file
Exif.SonyMisc3c.CameraOrientation            Byte        1  Horizontal (normal)
Exif.SonyMisc3c.Quality2                     Byte        1  Raw + JPEG
Exif.SonyMisc3c.SonyImageHeight              Short       1  n/a
Exif.SonyMisc3c.ModelReleaseYear             Byte        1  n/a
"""
    ]
    stderr = [""]
    retval = [0]


# No support for SonyMisc3c tags using this model
class SonyMisc3cUnsupportedTest(metaclass=CaseMeta):
    filename = path("$data_path/test_issue_1464.exv")
    commands = ["$exiv2 -pa --grep SonyMisc3c $filename"]

    stdout = [""""""]
    stderr = [""]
    retval = [1]
