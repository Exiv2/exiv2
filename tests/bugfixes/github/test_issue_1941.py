# -*- coding: utf-8 -*-

import system_tests
from system_tests import CaseMeta, CopyTmpFiles, path

class TestNikonFl7GroupWithFlash(metaclass=CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/1941"

    filename = system_tests.path("$data_path/exiv2-issue1941-1.exv")
    commands = ["""$exiv2 --grep NikonFl7 $filename"""]
    
    stderr = [""]
    retval = [0]
    
    stdout = ["""Exif.NikonFl7.Version                        Undefined   4  1.08
Exif.NikonFl7.FlashSource                    Byte        1  External
Exif.NikonFl7.ExternalFlashFirmware          Short       1  5.01 (SB-900)
Exif.NikonFl7.ExternalFlashData1             Byte        1  No external flash zoom override, external flash attached
Exif.NikonFl7.ExternalFlashData2             Byte        1  n/a
Exif.NikonFl7.FlashCompensation              SByte       1  0
Exif.NikonFl7.FlashFocalLength               Byte        1  n/a
Exif.NikonFl7.RepeatingFlashRate             Byte        1  n/a
Exif.NikonFl7.RepeatingFlashCount            Byte        1  n/a
Exif.NikonFl7.FlashGNDistance                Byte        1  None
Exif.NikonFl7.FlashGroupAControlData         Byte        1  Manual
Exif.NikonFl7.FlashGroupBCControlData        Byte        1  Off, Off
Exif.NikonFl7.FlashGroupAData                Byte        1  4%
Exif.NikonFl7.FlashGroupBData                Byte        1  0
Exif.NikonFl7.FlashGroupCData                Byte        1  0
"""]

class TestNikonFl7GroupWithoutFlash(metaclass=CaseMeta):

    url = "https://github.com/Exiv2/exiv2/issues/1941"

    filename = system_tests.path("$data_path/exiv2-bug1014_2.exv")
    commands = ["""$exiv2 --grep NikonFl7 $filename"""]
    
    stderr = [""]
    retval = [0]
    
    stdout = ["""Exif.NikonFl7.Version                        Undefined   4  1.07
Exif.NikonFl7.FlashSource                    Byte        1  None
Exif.NikonFl7.ExternalFlashFirmware          Short       1  n/a
Exif.NikonFl7.ExternalFlashData1             Byte        1  No external flash zoom override, external flash not attached
Exif.NikonFl7.ExternalFlashData2             Byte        1  n/a
Exif.NikonFl7.FlashCompensation              SByte       1  0
Exif.NikonFl7.FlashFocalLength               Byte        1  n/a
Exif.NikonFl7.RepeatingFlashRate             Byte        1  n/a
Exif.NikonFl7.RepeatingFlashCount            Byte        1  n/a
Exif.NikonFl7.FlashGNDistance                Byte        1  None
Exif.NikonFl7.FlashGroupAControlData         Byte        1  Off
Exif.NikonFl7.FlashGroupBCControlData        Byte        1  Off, Off
Exif.NikonFl7.FlashGroupAData                Byte        1  0
Exif.NikonFl7.FlashGroupBData                Byte        1  0
Exif.NikonFl7.FlashGroupCData                Byte        1  0
"""]
