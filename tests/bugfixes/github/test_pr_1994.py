# -*- coding: utf-8 -*-

from system_tests import CaseMeta, path


class TestAddModelsForNikonFl3(metaclass=CaseMeta):
    """
    Enables NikonFl3 group to be used by more camera models
    """

    filename1 = path("$data_path/pr_1994_poc1.jpg")
    filename2 = path("$data_path/pr_1994_poc2.jpg")
    commands = [
        "$exiv2 --grep NikonFl3 $filename1",
        "$exiv2 --grep NikonFl3 $filename2",
    ]
    stdout = [
        """Exif.NikonFl3.Version                        Undefined   4  1.04
Exif.NikonFl3.FlashSource                    Byte        1  None
Exif.NikonFl3.ExternalFlashFirmware          Short       1  n/a
Exif.NikonFl3.ExternalFlashFlags             Byte        1  Fired
Exif.NikonFl3.FlashFocalLength               Byte        1  n/a
Exif.NikonFl3.RepeatingFlashRate             Byte        1  n/a
Exif.NikonFl3.RepeatingFlashCount            Byte        1  n/a
Exif.NikonFl3.FlashGNDistance                Byte        1  n/a
Exif.NikonFl3.FlashColorFilter               Byte        1  None
""",
        """Exif.NikonFl3.Version                        Undefined   4  1.05
Exif.NikonFl3.FlashSource                    Byte        1  None
Exif.NikonFl3.ExternalFlashFirmware          Short       1  n/a
Exif.NikonFl3.ExternalFlashFlags             Byte        1  Fired
Exif.NikonFl3.FlashFocalLength               Byte        1  n/a
Exif.NikonFl3.RepeatingFlashRate             Byte        1  n/a
Exif.NikonFl3.RepeatingFlashCount            Byte        1  n/a
Exif.NikonFl3.FlashGNDistance                Byte        1  n/a
Exif.NikonFl3.FlashColorFilter               Byte        1  None
""",
    ]
    stderr = [""] * 2
    retval = [0] * 2
