# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class SonyMisc2bTestSupported(metaclass=CaseMeta):
    # Check that JPEG and TIFF based formats work
    filenames = [
        path("$data_path/test_issue_1464.exv"),  # JPEG
        path("$data_path/exiv2-bug1153Aa.exv"),
    ]  # TIFF
    commands = [f"$exiv2 -pa --grep SonyMisc2b {fname!s}" for fname in filenames]

    stdout = [
        """Exif.SonyMisc2b.ExposureProgram              Byte        1  Shutter speed priority AE
Exif.SonyMisc2b.IntelligentAuto              Byte        1  Off
Exif.SonyMisc2b.LensZoomPosition             Short       1  100%
Exif.SonyMisc2b.FocusPosition2               Byte        1  0
""",
        """Exif.SonyMisc2b.ExposureProgram              Byte        1  Aperture-priority AE
Exif.SonyMisc2b.IntelligentAuto              Byte        1  Off
Exif.SonyMisc2b.LensZoomPosition             Short       1  0%
Exif.SonyMisc2b.FocusPosition2               Byte        1  0
""",
    ]
    stderr = [""] * 2
    retval = [0] * 2


# An example of a Sony camera model that does NOT support SonyMisc2b
class SonyMisc2bTestUnsupported(metaclass=CaseMeta):
    filename = path("$data_path/exiv2-pr906.exv")
    commands = ["$exiv2 -pa --grep SonyMisc2b $filename"]

    stdout = [""]
    stderr = [""]
    retval = [1]
