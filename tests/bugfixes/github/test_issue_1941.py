# -*- coding: utf-8 -*-

import system_tests
from system_tests import CaseMeta


class TestNikonFl7GroupWithFlash(metaclass=CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/1941"

    filename = system_tests.path("$data_path/exiv2-issue1941-1.exv")
    commands = ["""$exiv2 --grep NikonFl7 $filename"""]

    stderr = [""]
    retval = [0]

    stdout = [
        """Exif.NikonFl7.Version                        Undefined   4  1.08
Exif.NikonFl7.FlashSource                    Byte        1  External
Exif.NikonFl7.ExternalFlashFirmware          Short       1  5.01 (SB-900)
Exif.NikonFl7.ExternalFlashData1             Byte        1  External flash on, No external flash zoom override
Exif.NikonFl7.ExternalFlashData2             Byte        1  Off
Exif.NikonFl7.FlashCompensationMaster        Byte        1  n/a
Exif.NikonFl7.FlashFocalLength               Byte        1  n/a
Exif.NikonFl7.RepeatingFlashRate             Byte        1  n/a
Exif.NikonFl7.RepeatingFlashCount            Byte        1  n/a
Exif.NikonFl7.FlashGNDistance                Byte        1  n/a
Exif.NikonFl7.FlashColorFilter               Byte        1  None
Exif.NikonFl7.FlashGroupAControlData         Byte        1  Manual
Exif.NikonFl7.FlashGroupBCControlData        Byte        1  Off, Off
Exif.NikonFl7.FlashCompensationGroupA        Byte        1  1/16 (-2/3 EV)
Exif.NikonFl7.FlashCompensationGroupB        Byte        1  n/a
Exif.NikonFl7.FlashCompensationGroupC        Byte        1  n/a
Exif.NikonFl7.ExternalFlashData3             Byte        1  Exposure Comp.: Entire frame
Exif.NikonFl7.CameraFlashOutput              Byte        1  0.0 EV
Exif.NikonFl7.CameraExposureCompensation     Byte        1  0.0 EV
Exif.NikonFl7.CameraFlashCompensation        Byte        1  0.0 EV
Exif.NikonFl7.ExternalFlashData4             Byte        1  Illumination Pat.: Standard
Exif.NikonFl7.FlashZoomHeadPosition          Byte        1  n/a
Exif.NikonFl7.FlashMasterOutput              Byte        1  n/a
Exif.NikonFl7.FlashGroupAOutput              Byte        1  1/16 (-2/3 EV)
Exif.NikonFl7.FlashGroupBOutput              Byte        1  n/a
Exif.NikonFl7.FlashGroupCOutput              Byte        1  n/a
Exif.NikonFl7.WirelessFlashData              Byte        1  Optical AWL
"""
    ]


class TestNikonFl7GroupWithoutFlash(metaclass=CaseMeta):
    url = "https://github.com/Exiv2/exiv2/issues/1941"

    filename = system_tests.path("$data_path/exiv2-bug1014_2.exv")
    commands = ["""$exiv2 --grep NikonFl7 $filename"""]

    stderr = [""]
    retval = [0]

    stdout = [
        """Exif.NikonFl7.Version                        Undefined   4  1.07
Exif.NikonFl7.FlashSource                    Byte        1  None
Exif.NikonFl7.ExternalFlashFirmware          Short       1  n/a
Exif.NikonFl7.ExternalFlashData1             Byte        1  External flash off
Exif.NikonFl7.ExternalFlashData2             Byte        1  Off
Exif.NikonFl7.FlashCompensationMaster        Byte        1  n/a
Exif.NikonFl7.FlashFocalLength               Byte        1  n/a
Exif.NikonFl7.RepeatingFlashRate             Byte        1  n/a
Exif.NikonFl7.RepeatingFlashCount            Byte        1  n/a
Exif.NikonFl7.FlashGNDistance                Byte        1  n/a
Exif.NikonFl7.FlashColorFilter               Byte        1  None
Exif.NikonFl7.FlashGroupAControlData         Byte        1  Off
Exif.NikonFl7.FlashGroupBCControlData        Byte        1  Off, Off
Exif.NikonFl7.FlashCompensationGroupA        Byte        1  n/a
Exif.NikonFl7.FlashCompensationGroupB        Byte        1  n/a
Exif.NikonFl7.FlashCompensationGroupC        Byte        1  n/a
Exif.NikonFl7.ExternalFlashData3             Byte        1  Exposure Comp.: Entire frame
Exif.NikonFl7.CameraFlashOutput              Byte        1  0.0 EV
Exif.NikonFl7.CameraExposureCompensation     Byte        1  0.0 EV
Exif.NikonFl7.CameraFlashCompensation        Byte        1  0.0 EV
Exif.NikonFl7.ExternalFlashData4             Byte        1  Illumination Pat.: Standard
Exif.NikonFl7.FlashZoomHeadPosition          Byte        1  n/a
Exif.NikonFl7.FlashMasterOutput              Byte        1  n/a
Exif.NikonFl7.FlashGroupAOutput              Byte        1  n/a
Exif.NikonFl7.FlashGroupBOutput              Byte        1  n/a
Exif.NikonFl7.FlashGroupCOutput              Byte        1  n/a
"""
    ]
